#include "reference_counter.h"
#include <iostream>

ReferenceCounter::ReferenceCounter(std::unordered_map<int, RCObject> &heap_, EventLogger &logger_)
    : heap(heap_), logger(logger_) {}

bool ReferenceCounter::add_ref(int from, int to)
{
    if (!heap.count(from) || !heap.count(to))
    {
        std::cerr << "add_ref error: invalid object\n";
        return false;
    }

    RCObject &src = heap[from];
    RCObject &dst = heap[to];

    if (src.has_reference_to(to))
    {
        return false;
    }

    src.add_outgoing_ref(to);
    dst.ref_count++;
    logger.log_add_ref(from, to, dst.ref_count);

    return true;
}

bool ReferenceCounter::remove_ref_no_cascade(int from, int to)
{
    if (!heap.count(from) || !heap.count(to))
    {
        std::cerr << "remove_ref error: invalid object\n";
        return false;
    }

    RCObject &src = heap[from];
    RCObject &dst = heap[to];

    if (!src.has_reference_to(to))
    {
        return false;
    }

    src.remove_outgoing_ref(to);
    dst.ref_count--;
    logger.log_remove_ref(from, to, dst.ref_count);

    // НЕ вызываем cascade_delete здесь
    return true;
}
bool ReferenceCounter::remove_ref(int from, int to)
{
    if (!heap.count(from) || !heap.count(to))
    {
        std::cerr << "remove_ref error: invalid object\n";
        return false;
    }

    RCObject &src = heap[from];
    RCObject &dst = heap[to];

    if (!src.has_reference_to(to))
    {
        return false;
    }

    src.remove_outgoing_ref(to);
    dst.ref_count--;
    logger.log_remove_ref(from, to, dst.ref_count);

    // НЕ запускаем каскадное удаление здесь - это будет сделано в RCHeap::remove_ref
    // если объект действительно нужно удалить
    return true;
}

void ReferenceCounter::cascade_delete(int obj_id)
{
    if (!heap.count(obj_id))
    {
        return;
    }

    RCObject &obj = heap[obj_id];

    // Удаляем только если ref_count == 0
    if (obj.ref_count != 0)
    {
        std::cout << "  [CASCADE SKIP] obj_" << obj_id << " has ref_count=" << obj.ref_count << std::endl;
        return;
    }

    // Получаем детей перед удалением объекта
    std::vector<int> children = obj.references;

    // Получаем размер объекта
    int obj_size = obj.size > 0 ? obj.size : 64;

    // Удаляем объект из кучи
    heap.erase(obj_id);
    logger.log_delete(obj_id);

    std::cout << "  [CASCADE] Deleted obj_" << obj_id << " (" << obj_size << " bytes)" << std::endl;

    // Рекурсивно обрабатываем детей
    for (int child : children)
    {
        if (heap.count(child))
        {
            RCObject &child_obj = heap[child];
            child_obj.ref_count--;
            logger.log_remove_ref(obj_id, child, child_obj.ref_count);

            std::cout << "  [CASCADE] Decreased ref_count for obj_" << child
                      << " (now: " << child_obj.ref_count << ")" << std::endl;

            // Если ref_count стал 0, удаляем рекурсивно
            if (child_obj.ref_count == 0)
            {
                cascade_delete(child);
            }
        }
    }
}