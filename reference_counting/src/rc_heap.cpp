#include "rc_heap.h"
#include <iostream>
#include <algorithm>
#include <sstream>

// ============================================
// КОНСТРУКТОР
// ============================================

RCHeap::RCHeap(EventLogger &logger_, RCLogger &rc_logger_, std::size_t heap_size_bytes_)
    : heap_size_bytes(heap_size_bytes_),
      rc(objects, logger_),
      logger(logger_),
      rc_logger(rc_logger_)
{
    rc_logger.log_init(heap_size_bytes);
}

// ============================================
// ALLOCATE - выделить новый объект
// ============================================

bool RCHeap::allocate(int obj_id, size_t size)
{
    // Проверить, не существует ли уже объект с таким ID
    if (objects.count(obj_id) > 0)
    {
        std::cerr << "Error: Object " << obj_id << " already exists\n";
        return false;
    }

    // Проверить валидность ID
    if (obj_id < 0)
    {
        std::cerr << "Error: Invalid object ID " << obj_id << "\n";
        return false;
    }

    // Выделить новый объект
    objects.emplace(obj_id, RCObject(obj_id));
    object_sizes[obj_id] = size;

    // Логировать с размером объекта
    rc_logger.log_allocate(obj_id, size);
    logger.log_allocate(obj_id, static_cast<int>(size));

    return true;
}

// ============================================
// ADD_ROOT - добавить объект в корни
// ============================================

bool RCHeap::add_root(int obj_id)
{
    // Проверить, существует ли объект
    if (!object_exists(obj_id))
    {
        std::cerr << "Error: Object " << obj_id << " does not exist\n";
        return false;
    }

    // Проверить, не является ли объект уже корнем
    if (roots.count(obj_id) > 0)
    {
        std::cerr << "Warning: Object " << obj_id << " is already a root\n";
        return false;
    }

    // Добавить в корни и увеличить ref_count
    roots.insert(obj_id);
    objects[obj_id].ref_count++;

    // Логировать
    rc_logger.log_make_root(obj_id);
    logger.log_add_ref(0, obj_id, objects[obj_id].ref_count);

    return true;
}

// ============================================
// ADD_REF - добавить ссылку от объекта к объекту
// ============================================

bool RCHeap::add_ref(int from, int to)
{
    // Валидация ID'ов
    if (from < 0 || to < 0)
    {
        std::cerr << "Error: Invalid object IDs\n";
        return false;
    }

    // Проверить, существуют ли оба объекта
    if (!object_exists(from))
    {
        std::cerr << "Error: Source object " << from << " does not exist\n";
        return false;
    }

    if (!object_exists(to))
    {
        std::cerr << "Error: Target object " << to << " does not exist\n";
        return false;
    }

    // Запретить саморефренцию
    if (from == to)
    {
        std::cerr << "Error: Self-reference not allowed\n";
        return false;
    }

    // Делегировать ReferenceCounter
    bool result = rc.add_ref(from, to);

    if (result)
    {
        rc_logger.log_add_ref(from, to);
    }

    return result;
}

// ============================================
// REMOVE_REF - удалить ссылку между объектами
// ============================================

bool RCHeap::remove_ref(int from, int to)
{
    // Валидация ID'ов
    if (from < 0 || to < 0)
    {
        std::cerr << "Error: Invalid object IDs\n";
        return false;
    }

    // Проверить, существует ли source объект
    if (!object_exists(from))
    {
        std::cerr << "Error: Source object " << from << " does not exist\n";
        return false;
    }

    // Целевой объект может быть удален во время операции, но проверим перед удалением
    if (!object_exists(to))
    {
        std::cerr << "Error: Target object " << to << " does not exist\n";
        return false;
    }

    // Логировать удаление ссылки
    rc_logger.log_remove_ref(from, to);

    // Получить текущий ref_count перед удалением для логирования
    int old_ref_count = objects[to].ref_count;

    // Делегировать ReferenceCounter - УДАЛИТЬ ССЫЛКУ БЕЗ КАСКАДА
    if (!rc.remove_ref_no_cascade(from, to))
    {
        return false;
    }

    // Получить новый ref_count после удаления
    int new_ref_count = objects[to].ref_count;

    // Проверить, стал ли ref_count целевого объекта 0
    if (new_ref_count == 0)
    {
        // Если объект стал недостижим (не корень), запустить каскад
        if (roots.count(to) == 0)
        {
            rc.cascade_delete(to);
        }
    }

    return true;
}

// ============================================
// REMOVE_ROOT - удалить объект из корней
// ============================================

bool RCHeap::remove_root(int obj_id)
{
    // Проверить, существует ли объект
    if (!object_exists(obj_id))
    {
        std::cerr << "Error: Object " << obj_id << " does not exist\n";
        return false;
    }

    // Проверить, является ли объект корнем
    if (roots.count(obj_id) == 0)
    {
        std::cerr << "Error: Object " << obj_id << " is not a root\n";
        return false;
    }

    // Получить ref_count перед удалением
    int old_ref_count = objects[obj_id].ref_count;

    // Удалить из корней
    roots.erase(obj_id);

    // Уменьшить ref_count (корень считался как +1 к ref_count)
    objects[obj_id].ref_count--;

    // Получить новый ref_count
    int new_ref_count = objects[obj_id].ref_count;

    if (new_ref_count < 0)
    {
        std::cerr << "Error: ref_count became negative for root object " << obj_id << "\n";
        objects[obj_id].ref_count = 0;
        return false;
    }

    // Логировать удаление корня
    rc_logger.log_remove_root(obj_id);
    logger.log_remove_ref(0, obj_id, new_ref_count);

    // Если ref_count == 0, начать каскадное удаление
    if (new_ref_count == 0)
    {
        rc.cascade_delete(obj_id);
    }

    return true;
}

// ============================================
// DUMP_STATE - вывести состояние heap
// ============================================

void RCHeap::dump_state() const
{
    std::cout << "=== HEAP STATE ===\n";
    // Вывести корни
    std::cout << "ROOTS: ";
    if (roots.empty())
    {
        std::cout << "[none]";
    }
    else
    {
        for (int root : roots)
        {
            std::cout << root << " ";
        }
    }
    std::cout << "\n\n";

    if (objects.empty())
    {
        std::cout << "[empty]\n";
    }
    else
    {
        // Вывести объекты отсортированные по ID для консистентности
        std::vector<int> ids;
        for (const auto &[id, _] : objects)
        {
            ids.push_back(id);
        }
        std::sort(ids.begin(), ids.end());

        for (int id : ids)
        {
            const RCObject &obj = objects.at(id);
            std::cout << "Object " << id
                      << " | ref_count=" << obj.ref_count
                      << " | refs: ";
            // Вывести исходящие ссылки
            for (int ref : obj.references)
            {
                std::cout << ref << " ";
            }
            std::cout << "\n";
        }
    }

    std::cout << "=================\n\n";
}

// ============================================
// RUN_SCENARIO - выполнить сценарий операций
// ============================================

void RCHeap::run_scenario(const ScenarioOp ops[], int size)
{
    for (int i = 0; i < size; ++i)
    {
        const ScenarioOp &op = ops[i];
        if (op.op == "allocate")
        {
            allocate(op.id, op.size);
        }
        else if (op.op == "add_root" || op.op == "addroot" || op.op == "make_root")
        {
            add_root(op.id);
        }
        else if (op.op == "remove_root" || op.op == "removeroot")
        {
            remove_root(op.id);
        }
        else if (op.op == "add_ref" || op.op == "addref")
        {
            add_ref(op.from, op.to);
        }
        else if (op.op == "remove_ref" || op.op == "removeref")
        {
            remove_ref(op.from, op.to);
        }
        else
        {
            std::cerr << "Unknown operation: " << op.op << "\n";
        }
    }

    dump_state();
}

// ============================================
// GET_REF_COUNT - получить счетчик ссылок объекта
// ============================================

int RCHeap::get_ref_count(int obj_id) const
{
    auto it = objects.find(obj_id);
    if (it != objects.end())
    {
        return it->second.ref_count;
    }
    return -1; // Объект не существует
}

// ============================================
// DETECT_AND_LOG_LEAKS - обнаружить утечки памяти
// ============================================

void RCHeap::detect_and_log_leaks()
{
    for (const auto &[id, obj] : objects)
    {
        // Логировать объекты с ref_count > 0 (утечка памяти!)
        if (obj.ref_count > 0)
        {
            rc_logger.log_leak(id);
            logger.log_leak(id);
        }
    }
}

// ============================================
// GET_OBJECT - получить объект по ID (неконстантная версия)
// ============================================

RCObject *RCHeap::get_object(int obj_id)
{
    auto it = objects.find(obj_id);
    if (it != objects.end())
    {
        return &it->second;
    }
    return nullptr;
}

// ============================================
// GET_OBJECT - получить объект по ID (константная версия)
// ============================================

const RCObject *RCHeap::get_object(int obj_id) const
{
    auto it = objects.find(obj_id);
    if (it != objects.end())
    {
        return &it->second;
    }
    return nullptr;
}

// ============================================
// ДОПОЛНИТЕЛЬНЫЕ МЕТОДЫ (для совместимости с main.cpp)
// ============================================

bool RCHeap::addroot(int obj_id)
{
    return add_root(obj_id);
}

bool RCHeap::removeroot(int obj_id)
{
    return remove_root(obj_id);
}

bool RCHeap::addref(int from, int to)
{
    return add_ref(from, to);
}

bool RCHeap::removeref(int from, int to)
{
    return remove_ref(from, to);
}

int RCHeap::getrefcount(int obj_id) const
{
    return get_ref_count(obj_id);
}

bool RCHeap::objectexists(int obj_id) const
{
    return object_exists(obj_id);
}

std::size_t RCHeap::getheapsize() const
{
    return get_heap_size();
}

std::size_t RCHeap::getrootscount() const
{
    return get_roots_count();
}

std::size_t RCHeap::get_heap_size_bytes() const
{
    return heap_size_bytes;
}