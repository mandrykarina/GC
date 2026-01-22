#ifndef RC_LOGGER_H
#define RC_LOGGER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * @class RCLogger
 * @brief Логгер для Reference Counting GC в формате Mark-Sweep
 * 
 * Логирует все операции RC в файл в точно таком же формате,
 * как Mark-Sweep, включая:
 * - Инициализацию GC с max_heap
 * - ALLOCATE операции с размерами
 * - MAKE_ROOT / REMOVE_ROOT операции
 * - ADD_REF / REMOVE_REF операции
 * - Cascade deletion события
 * - Session start/end маркеры
 */
class RCLogger
{
private:
    std::ofstream log_file;
    int current_step;

public:
    /**
     * @brief Конструктор логгера
     * @param log_file_path Путь к файлу логов
     */
    explicit RCLogger(const std::string& log_file_path);

    /**
     * @brief Деструктор
     */
    ~RCLogger();

    /**
     * @brief Логировать операцию
     * @param operation Текст операции
     */
    void log_operation(const std::string& operation);

    /**
     * @brief Логировать инициализацию GC
     * @param max_heap Максимальный размер кучи
     */
    void log_init(size_t max_heap);

    /**
     * @brief Логировать выделение объекта
     * @param obj_id ID объекта
     * @param size Размер в байтах
     */
    void log_allocate(int obj_id, size_t size);

    /**
     * @brief Логировать создание root объекта
     * @param obj_id ID объекта
     */
    void log_make_root(int obj_id);

    /**
     * @brief Логировать удаление root статуса
     * @param obj_id ID объекта
     */
    void log_remove_root(int obj_id);

    /**
     * @brief Логировать добавление ссылки
     * @param from ID объекта-источника
     * @param to ID объекта-цели
     */
    void log_add_ref(int from, int to);

    /**
     * @brief Логировать удаление ссылки
     * @param from ID объекта-источника
     * @param to ID объекта-цели
     */
    void log_remove_ref(int from, int to);

    /**
     * @brief Логировать каскадное удаление
     * @param obj_id ID объекта для удаления
     * @param size Размер в байтах
     */
    void log_cascade_delete(int obj_id, size_t size);

    /**
     * @brief Логировать утечку памяти
     * @param obj_id ID объекта
     */
    void log_leak(int obj_id);

    /**
     * @brief Получить номер текущего шага
     * @return Номер шага
     */
    int get_current_step() const { return current_step; }

    /**
     * @brief Увеличить номер шага
     */
    void increment_step() { current_step++; }

    /**
     * @brief Проверить, открыт ли файл
     * @return true если файл открыт
     */
    bool is_open() const { return log_file.is_open(); }
};

#endif // RC_LOGGER_H