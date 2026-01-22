#ifndef RC_HEAP_H
#define RC_HEAP_H

#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstddef>
#include "rc_object.h"
#include "reference_counter.h"
#include "event_logger.h"
#include "rc_logger.h"

/**
 * @struct ScenarioOp
 * @brief Представляет одну операцию в сценарии тестирования
 */
struct ScenarioOp
{
    std::string op; // "allocate", "add_ref", "remove_ref", "delete_root"
    int id;         // Для allocate
    int from;       // Для add_ref и remove_ref
    int to;         // Для add_ref и remove_ref
    size_t size;    // Размер объекта в байтах

    ScenarioOp() : op(""), id(-1), from(-1), to(-1), size(8) {}
    ScenarioOp(const std::string &op_, int id_, int from_ = -1, int to_ = -1, size_t size_ = 8)
        : op(op_), id(id_), from(from_), to(to_), size(size_) {}
};

/**
 * @class RCHeap
 * @brief Управляет кучей объектов с подсчётом ссылок и корнями
 *
 * Инкапсулирует управление памятью, добавление/удаление ссылок,
 * управление корнями (roots) и визуализацию состояния кучи.
 *
 * **ВАЖНО: RC ONLY! Только объекты с ref_count == 0 удаляются!**
 */
class RCHeap
{
public:
    /**
     * @brief Конструктор с логгером и размером heap
     * @param logger Ссылка на логгер событий
     * @param rc_logger Ссылка на RC-специфичный логгер
     * @param heap_size_bytes Размер кучи в байтах (по умолчанию 10 MB)
     */
    explicit RCHeap(EventLogger &logger, RCLogger &rc_logger, std::size_t heap_size_bytes = 10485760);

    /**
     * @brief Выделить новый объект в куче
     * @param obj_id ID выделяемого объекта
     * @param size Размер объекта в байтах (по умолчанию 8)
     * @return true, если объект успешно выделен
     */
    bool allocate(int obj_id, size_t size = 8);

    /**
     * @brief Добавить объект в корни (root)
     * @param obj_id ID объекта для добавления в корни
     * @return true, если объект добавлен в корни
     */
    bool add_root(int obj_id);

    /**
     * @brief Удалить объект из корней (root)
     * @param obj_id ID объекта для удаления из корней
     * @return true, если объект удалён из корней
     */
    bool remove_root(int obj_id);

    /**
     * @brief Добавить ссылку от одного объекта к другому
     * @param from ID объекта-источника
     * @param to ID объекта-цели
     * @return true, если ссылка успешно добавлена
     */
    bool add_ref(int from, int to);

    /**
     * @brief Удалить ссылку между объектами
     * @param from ID объекта-источника
     * @param to ID объекта-цели
     * @return true, если ссылка успешно удалена
     */
    bool remove_ref(int from, int to);

    /**
     * @brief Вывести текущее состояние кучи в консоль
     */
    void dump_state() const;

    /**
     * @brief Выполнить последовательность операций из сценария
     * @param ops Массив операций сценария
     * @param size Размер массива операций
     */
    void run_scenario(const ScenarioOp ops[], int size);

    /**
     * @brief Получить количество объектов в куче
     * @return Размер кучи
     */
    std::size_t get_heap_size() const { return objects.size(); }

    /**
     * @brief Проверить, существует ли объект в куче
     * @param obj_id ID проверяемого объекта
     * @return true, если объект существует
     */
    bool object_exists(int obj_id) const { return objects.count(obj_id) > 0; }

    /**
     * @brief Получить ref_count объекта
     * @param obj_id ID объекта
     * @return ref_count, или -1 если объект не существует
     */
    int get_ref_count(int obj_id) const;

    /**
     * @brief Обнаружить и зарегистрировать утечки памяти
     *
     * Проверяет все объекты в куче с ref_count > 0 (которые не удалены)
     * Это объекты, участвующие в циклических ссылках!
     */
    void detect_and_log_leaks();

    /**
     * @brief Получить количество корней
     * @return Размер множества корней
     */
    std::size_t get_roots_count() const { return roots.size(); }

    /**
     * @brief Получить размер кучи в байтах
     * @return Размер кучи, переданный в конструктор
     */
    std::size_t get_heap_size_bytes() const;

    // ========== СОВМЕСТИМОСТЬ С MAIN.CPP ==========
    // Методы без подчёркивания для старого кода

    bool addroot(int obj_id);
    bool removeroot(int obj_id);
    bool addref(int from, int to);
    bool removeref(int from, int to);
    int getrefcount(int obj_id) const;
    bool objectexists(int obj_id) const;
    std::size_t getheapsize() const;
    std::size_t getrootscount() const;

private:
    std::size_t heap_size_bytes;                  ///< Размер кучи в байтах
    std::unordered_map<int, RCObject> objects;    ///< Куча объектов
    std::unordered_map<int, size_t> object_sizes; ///< Размеры объектов
    std::unordered_set<int> roots;                ///< Корни (root объекты)
    ReferenceCounter rc;                          ///< Управление ссылками
    EventLogger &logger;                          ///< Логгер событий
    RCLogger &rc_logger;                          ///< RC-специфичный логгер

    /**
     * @brief Получить объект по ID (внутренняя функция)
     * @param obj_id ID объекта
     * @return Указатель на объект, или nullptr
     */
    RCObject *get_object(int obj_id);

    /**
     * @brief Получить объект по ID (константная версия)
     * @param obj_id ID объекта
     * @return Указатель на константный объект, или nullptr
     */
    const RCObject *get_object(int obj_id) const;
};

#endif // RC_HEAP_H