#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include "rc_heap.h"
#include "event_logger.h"
#include "common_utils.h"
#include "rc_logger.h"

// ============================================
// REFERENCE COUNTING SPECIFIC STRUCTURES
// ============================================
struct GCStats
{
    size_t collections_run = 0;
    size_t total_objects_collected = 0;
    size_t total_freed = 0;
    double total_collection_time_us = 0.0;

    double avg_collection_time() const
    {
        return collections_run > 0 ? total_collection_time_us / collections_run : 0.0;
    }

    double avg_objects_per_collection() const
    {
        return collections_run > 0 ? (double)total_objects_collected / collections_run : 0.0;
    }

    void print() const
    {
        std::cout << std::string(70, '=') << std::endl;
        std::cout << " GC Statistics (Reference Counting)" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
        std::cout << " Collections run: " << std::setw(12) << collections_run << std::endl;
        std::cout << " Total objects collected: " << std::setw(8) << total_objects_collected << std::endl;
        std::cout << " Total memory freed: " << std::setw(10) << total_freed << " bytes" << std::endl;
        std::cout << " Total collection time: " << std::fixed << std::setprecision(0)
                  << total_collection_time_us << " µs" << std::endl;
        std::cout << " Average collection time: " << std::fixed << std::setprecision(0)
                  << avg_collection_time() << " µs" << std::endl;
        std::cout << " Average objects per collection: " << std::fixed << std::setprecision(0)
                  << avg_objects_per_collection() << std::endl;
    }
};

// ============================================
// REFERENCE COUNTING SPECIFIC FUNCTIONS
// ============================================
void show_rc_menu()
{
    std::cout << "\n"
              << std::string(80, '=') << std::endl;
    std::cout << " Reference Counting Garbage Collector - Main Menu" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "\n [1] Run Scenario: Basic Chain" << std::endl;
    std::cout << " [2] Run Scenario: Cyclic Graph (RC Leak Demo) - EXPECT FAILURE" << std::endl;
    std::cout << " [3] Run Scenario: Cascade Tree" << std::endl;
    std::cout << "\n"
              << std::string(80, '-') << std::endl;
    std::cout << " CUSTOM GENERATION (with Interactive Config)" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    std::cout << " [4] Generate & Run: Linear Chain" << std::endl;
    std::cout << " [5] Generate & Run: Cyclic Graph (will leak)" << std::endl;
    std::cout << " [6] Generate & Run: Cascade Tree" << std::endl;
    std::cout << " [0] Exit\n"
              << std::endl;
}

ParsedScenario parse_json_scenario_rc(const std::string &filename)
{
    ParsedScenario result;
    std::ifstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Cannot open: " << filename << std::endl;
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    size_t heap_size_pos = content.find("\"heap_size\":");
    if (heap_size_pos != std::string::npos)
    {
        size_t colon_pos = content.find(":", heap_size_pos);
        size_t comma_pos = content.find(",", colon_pos);
        if (comma_pos == std::string::npos)
        {
            comma_pos = content.find("}", colon_pos);
        }
        if (colon_pos != std::string::npos && comma_pos != std::string::npos)
        {
            std::string heap_size_str = content.substr(colon_pos + 1, comma_pos - colon_pos - 1);
            heap_size_str.erase(0, heap_size_str.find_first_not_of(" \t\n\r"));
            heap_size_str.erase(heap_size_str.find_last_not_of(" \t\n\r") + 1);
            try
            {
                result.heap_size = std::stoull(heap_size_str);
            }
            catch (...)
            {
                result.heap_size = 104857600;
            }
        }
    }

    size_t pos = 0;
    while (pos < content.length())
    {
        pos = content.find('{', pos);
        if (pos == std::string::npos)
            break;
        size_t end = content.find('}', pos);
        if (end == std::string::npos)
            break;
        std::string obj = content.substr(pos, end - pos + 1);
        pos = end + 1;

        if (obj.find("\"op\"") == std::string::npos)
            continue;

        Operation op;

        if (obj.find("\"op\": \"allocate\"") != std::string::npos ||
            obj.find("\"op\":\"allocate\"") != std::string::npos)
        {
            op.type = "allocate";
            size_t id_pos = obj.find("\"id\"");
            if (id_pos != std::string::npos)
            {
                sscanf(obj.c_str() + id_pos, "\"id\": %d", &op.id);
            }
            result.operations.push_back(op);
        }
        else if (obj.find("\"op\": \"addroot\"") != std::string::npos ||
                 obj.find("\"op\":\"addroot\"") != std::string::npos)
        {
            op.type = "addroot";
            size_t id_pos = obj.find("\"id\"");
            if (id_pos != std::string::npos)
            {
                sscanf(obj.c_str() + id_pos, "\"id\": %d", &op.id);
            }
            result.operations.push_back(op);
        }
        else if (obj.find("\"op\": \"addref\"") != std::string::npos ||
                 obj.find("\"op\":\"addref\"") != std::string::npos)
        {
            op.type = "addref";
            size_t from_pos = obj.find("\"from\"");
            size_t to_pos = obj.find("\"to\"");
            if (from_pos != std::string::npos)
            {
                sscanf(obj.c_str() + from_pos, "\"from\": %d", &op.from);
            }
            if (to_pos != std::string::npos)
            {
                sscanf(obj.c_str() + to_pos, "\"to\": %d", &op.to);
            }
            result.operations.push_back(op);
        }
        else if (obj.find("\"op\": \"removeroot\"") != std::string::npos ||
                 obj.find("\"op\":\"removeroot\"") != std::string::npos)
        {
            op.type = "removeroot";
            size_t id_pos = obj.find("\"id\"");
            if (id_pos != std::string::npos)
            {
                sscanf(obj.c_str() + id_pos, "\"id\": %d", &op.id);
            }
            result.operations.push_back(op);
        }
        else if (obj.find("\"op\": \"removeref\"") != std::string::npos ||
                 obj.find("\"op\":\"removeref\"") != std::string::npos)
        {
            op.type = "removeref";
            size_t from_pos = obj.find("\"from\"");
            size_t to_pos = obj.find("\"to\"");
            if (from_pos != std::string::npos)
            {
                sscanf(obj.c_str() + from_pos, "\"from\": %d", &op.from);
            }
            if (to_pos != std::string::npos)
            {
                sscanf(obj.c_str() + to_pos, "\"to\": %d", &op.to);
            }
            result.operations.push_back(op);
        }
    }

    return result;
}

// ============================================
// REFERENCE COUNTING SIMULATION FUNCTION
// ============================================
void run_simulation_rc(const std::string &scenario_file, const std::string &scenario_name,
                       size_t object_size = 64)
{
    std::cout << "\n"
              << std::string(70, '=') << std::endl;
    std::cout << " Running: " << scenario_name << std::endl;
    std::cout << std::string(70, '=') << "\n"
              << std::endl;

    ParsedScenario parsed = parse_json_scenario_rc(scenario_file);
    std::vector<Operation> &operations = parsed.operations;
    size_t heap_size = parsed.heap_size;

    std::cout << "[*] Parsed " << operations.size() << " operations\n"
              << std::endl;
    size_t heap_size_megabits = (heap_size * 8) / 1000000;
    std::cout << "[*] Heap Size: " << heap_size_megabits << " Mbits\n"
              << std::endl;

    if (operations.empty())
    {
        std::cerr << "No operations found!" << std::endl;
        return;
    }

    EventLogger logger("simulation_events.log");
    RCLogger rc_logger("rc_log"); // ← СОЗДАТЬ RC_LOGGER!
    RCHeap heap(logger, rc_logger, heap_size);

    MemoryStats mem_stats;
    GCStats gc_stats;
    auto start_time = std::chrono::high_resolution_clock::now();

    // Счетчик созданных объектов для начальной статистики
    int objects_created = 0;

    for (size_t step = 0; step < operations.size(); step++)
    {
        const Operation &op = operations[step];

        if (op.type == "allocate")
        {
            bool success = heap.allocate(op.id, object_size);
            if (success)
            {
                objects_created++;
                mem_stats.total_allocated += object_size;
            }
            size_t current_heap_bytes = heap.getheapsize() * object_size;
            mem_stats.peak_memory = std::max(mem_stats.peak_memory, current_heap_bytes);
            std::cout << " [" << std::setw(3) << step << "] ALLOCATE object_" << op.id;
            if (success)
            {
                std::cout << " ✓" << std::endl;
            }
            else
            {
                std::cout << " ✗ FAILED" << std::endl;
            }
        }
        else if (op.type == "addroot")
        {
            bool success = heap.addroot(op.id);
            std::cout << " [" << std::setw(3) << step << "] ADDROOT object_" << op.id;
            if (success)
            {
                int refcount = heap.getrefcount(op.id);
                std::cout << " ✓ (refcount: " << refcount << ")" << std::endl;
            }
            else
            {
                std::cout << " ✗ FAILED" << std::endl;
            }
        }
        else if (op.type == "removeroot")
        {
            int old_refcount = heap.getrefcount(op.id);
            bool success = heap.removeroot(op.id);
            std::cout << " [" << std::setw(3) << step << "] REMOVEROOT object_" << op.id;
            if (success)
            {
                // ТОЛЬКО логирование, НЕ подсчет памяти
                if (!heap.objectexists(op.id))
                {
                    // Объект был удален каскадом - только сообщаем об этом
                    size_t current_heap_bytes = heap.getheapsize() * object_size;
                    mem_stats.peak_memory = std::max(mem_stats.peak_memory, current_heap_bytes);
                    std::cout << " ✓ (refcount: " << old_refcount << " -> 0) [CASCADE DELETED]" << std::endl;
                }
                else
                {
                    // Объект остался (все еще имеет другие ссылки)
                    int new_refcount = heap.getrefcount(op.id);
                    std::cout << " ✓ (refcount: " << old_refcount << " -> " << new_refcount << ")" << std::endl;
                }
            }
            else
            {
                std::cout << " ✗ FAILED" << std::endl;
            }
        }
        else if (op.type == "addref")
        {
            bool success = heap.addref(op.from, op.to);
            std::cout << " [" << std::setw(3) << step << "] ADDREF object_"
                      << op.from << " -> object_" << op.to;
            if (success)
            {
                int refcount = heap.getrefcount(op.to);
                std::cout << " ✓ (refcount: " << refcount << ")" << std::endl;
            }
            else
            {
                std::cout << " ✗ FAILED" << std::endl;
            }
        }
        else if (op.type == "removeref")
        {
            int old_refcount = heap.getrefcount(op.to);
            bool success = heap.removeref(op.from, op.to);
            std::cout << " [" << std::setw(3) << step << "] REMOVEREF object_"
                      << op.from << " -> object_" << op.to;
            if (success)
            {
                // ТОЛЬКО логирование, НЕ подсчет памяти
                if (!heap.objectexists(op.to))
                {
                    // Объект был удален каскадом - только сообщаем об этом
                    size_t current_heap_bytes = heap.getheapsize() * object_size;
                    mem_stats.peak_memory = std::max(mem_stats.peak_memory, current_heap_bytes);
                    std::cout << " ✓ (refcount: " << old_refcount << " -> 0) [DELETED]" << std::endl;
                }
                else
                {
                    // Объект остался
                    int new_refcount = heap.getrefcount(op.to);
                    std::cout << " ✓ (refcount: " << old_refcount << " -> " << new_refcount << ")" << std::endl;
                }
            }
            else
            {
                std::cout << " ✗ FAILED" << std::endl;
            }
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double exec_time = std::chrono::duration<double, std::milli>(
                           end_time - start_time)
                           .count();

    // КОРРЕКТНЫЙ подсчет освобожденной памяти (основной метод)
    size_t objects_left = heap.getheapsize();
    size_t live_bytes = objects_left * object_size;

    // Всего выделено памяти
    size_t total_allocated_bytes = objects_created * object_size;

    // Освобождено памяти
    size_t freed_bytes = total_allocated_bytes - live_bytes;

    // Обновляем статистику
    mem_stats.total_allocated = total_allocated_bytes;
    mem_stats.total_freed = freed_bytes;
    mem_stats.leaked_memory = live_bytes; // Оставшиеся объекты = потенциальная утечка

    // Расчет процента восстановления
    mem_stats.recovery_percent = (total_allocated_bytes > 0) ? (freed_bytes * 100.0 / total_allocated_bytes) : 0.0;

    // Обновляем GC статистику
    gc_stats.collections_run = 1; // Для RC каждый каскад = одна сборка
    gc_stats.total_objects_collected = objects_created - objects_left;
    gc_stats.total_freed = freed_bytes;
    gc_stats.total_collection_time_us = exec_time * 1000.0;

    // Вывод статистики
    mem_stats.print();
    gc_stats.print();

    std::cout << std::string(70, '=') << std::endl;
    std::cout << " Heap Statistics" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << " Objects created: " << std::setw(12) << objects_created << std::endl;
    std::cout << " Objects in heap: " << std::setw(12) << heap.getheapsize() << std::endl;
    std::cout << " Active roots: " << std::setw(15) << heap.getrootscount() << std::endl;
    std::cout << " Heap size configured: " << std::setw(4) << (heap.get_heap_size_bytes() / 1048576) << " MB" << std::endl;

    size_t used_bytes = heap.getheapsize() * object_size;
    size_t total_bytes = heap.get_heap_size_bytes();
    size_t used_megabits = (used_bytes * 8) / 1000000;
    size_t total_megabits = (total_bytes * 8) / 1000000;
    std::cout << " Heap usage: " << used_megabits << " / " << total_megabits
              << " Mbits (" << std::fixed << std::setprecision(1)
              << (100.0 * used_bytes / total_bytes) << "%)" << std::endl;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << " Execution Time: " << exec_time << " ms\n"
              << std::endl;

    // ========== FINAL CHECK ==========
    if (heap.getheapsize() > 0)
    {
        std::cout << "\n"
                  << std::string(70, '!') << std::endl;
        std::cout << "! TEST RESULT - " << heap.getheapsize() << " OBJECTS REMAIN IN HEAP!" << std::endl;

        // Определяем тип утечки
        if (heap.getheapsize() == objects_created)
        {
            std::cout << "! ALL objects leaked - проверьте логику каскадного удаления!" << std::endl;
        }
        else if (heap.getrootscount() > 0)
        {
            std::cout << "! " << heap.getrootscount() << " root objects still exist" << std::endl;
        }

        std::cout << "! This is EXPECTED for cyclic reference tests!" << std::endl;
        std::cout << "! Simple RC cannot handle cycles without cycle detection." << std::endl;
        std::cout << std::string(70, '!') << std::endl;

        // Детальная информация об оставшихся объектах
        std::cout << "\nRemaining objects details:" << std::endl;
        for (int i = 0; i < objects_created; i++)
        {
            if (heap.objectexists(i))
            {
                int refcount = heap.getrefcount(i);
                std::cout << "  Object " << i << ": ref_count = " << refcount;
                if (heap.getrootscount() > 0)
                {
                    // Проверяем, является ли объект корнем
                    std::cout << " (ROOT)" << std::endl;
                }
                else
                {
                    std::cout << " (non-root)" << std::endl;
                }
            }
        }
    }
    else
    {
        std::cout << "\n"
                  << std::string(70, '=') << std::endl;
        std::cout << " TEST PASSED - ALL OBJECTS PROPERLY DELETED" << std::endl;
        std::cout << std::string(70, '=') << std::endl;
    }

    // Детектируем утечки и логируем их
    heap.detect_and_log_leaks();
}

int rc_main(int argc, char *argv[])
{
    std::cout << "\n"
              << std::string(80, '=') << std::endl;
    std::cout << " REFERENCE COUNTING GARBAGE COLLECTOR" << std::endl;
    std::cout << " Simple RC (No Cycle Detection)" << std::endl;
    std::cout << std::string(80, '=') << "\n"
              << std::endl;

    if (argc > 1)
    {
        std::string arg = argv[1];
        if (arg == "1")
        {
            run_simulation_rc(find_scenario("basic.json"), "Basic Chain Scenario", 64);
        }
        else if (arg == "2")
        {
            run_simulation_rc(find_scenario("cycle_leak.json"), "Cyclic Graph Scenario (RC Leak)", 64);
        }
        else if (arg == "3")
        {
            run_simulation_rc(find_scenario("cascade_delete.json"), "Cascade Tree Scenario", 64);
        }
        return 0;
    }

    while (true)
    {
        show_rc_menu();
        std::cout << "Enter your choice: ";
        int choice;
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            run_simulation_rc(find_scenario("basic.json"), "Basic Chain Scenario", 64);
            break;

        case 2:
            run_simulation_rc(find_scenario("cycle_leak.json"), "Cyclic Graph Scenario (RC Leak)", 64);
            break;

        case 3:
            run_simulation_rc(find_scenario("cascade_delete.json"), "Cascade Tree Scenario", 64);
            break;

        case 4:
        {
            std::cout << "\n"
                      << std::string(80, '=') << std::endl;
            std::cout << " CUSTOM: Generate Linear Chain" << std::endl;
            std::cout << std::string(80, '=') << std::endl;
            MemoryConfig config = interactive_memory_config();
            if (config.is_valid)
            {
                print_configuration_summary(config);
                std::cout << "\n[*] Generating JSON...\n"
                          << std::endl;
                std::string json_content = generate_linear_chain_json(
                    config.num_objects,
                    config.object_size,
                    config.heap_size_bytes);
                std::string json_file = save_generated_json(json_content,
                                                            "scenarios/generated_linear_" + std::to_string(config.num_objects) + ".json");
                if (!json_file.empty())
                {
                    std::cout << "[OK] JSON saved to: " << json_file << std::endl;
                    std::cout << "[*] Running simulation...\n"
                              << std::endl;
                    run_simulation_rc(json_file, "Generated Linear Chain (" + std::to_string(config.num_objects) + " objects)",
                                      config.object_size);
                }
            }
            else
            {
                std::cout << "Configuration is invalid.\n";
            }
            break;
        }

        case 5:
        {
            std::cout << "\n"
                      << std::string(80, '=') << std::endl;
            std::cout << " CUSTOM: Generate Cyclic Graph" << std::endl;
            std::cout << std::string(80, '=') << std::endl;
            MemoryConfig config = interactive_memory_config();
            if (config.is_valid)
            {
                print_configuration_summary(config);
                std::cout << "\n[*] Generating JSON...\n"
                          << std::endl;
                std::string json_content = generate_cyclic_graph_json(
                    config.num_objects,
                    config.object_size,
                    config.heap_size_bytes);
                std::string json_file = save_generated_json(json_content,
                                                            "scenarios/generated_cyclic_" + std::to_string(config.num_objects) + ".json");
                if (!json_file.empty())
                {
                    std::cout << "[OK] JSON saved to: " << json_file << std::endl;
                    std::cout << "[*] Running simulation...\n"
                              << std::endl;
                    run_simulation_rc(json_file, "Generated Cyclic Graph (" + std::to_string(config.num_objects) + " objects)",
                                      config.object_size);
                }
            }
            else
            {
                std::cout << "Configuration is invalid.\n";
            }
            break;
        }

        case 6:
        {
            std::cout << "\n"
                      << std::string(80, '=') << std::endl;
            std::cout << " CUSTOM: Generate Cascade Tree" << std::endl;
            std::cout << std::string(80, '=') << std::endl;
            MemoryConfig config = interactive_memory_config();
            if (config.is_valid)
            {
                print_configuration_summary(config);
                std::cout << "\n[*] Generating JSON...\n"
                          << std::endl;
                std::string json_content = generate_cascade_tree_json(
                    config.num_objects,
                    config.object_size,
                    config.heap_size_bytes);
                std::string json_file = save_generated_json(json_content,
                                                            "scenarios/generated_cascade_" + std::to_string(config.num_objects) + ".json");
                if (!json_file.empty())
                {
                    std::cout << "[OK] JSON saved to: " << json_file << std::endl;
                    std::cout << "[*] Running simulation...\n"
                              << std::endl;
                    run_simulation_rc(json_file, "Generated Cascade Tree (" + std::to_string(config.num_objects) + " objects)",
                                      config.object_size);
                }
            }
            else
            {
                std::cout << "Configuration is invalid.\n";
            }
            break;
        }

        case 0:
            std::cout << "\nGoodbye!\n"
                      << std::endl;
            return 0;

        default:
            std::cout << "\nInvalid choice. Try again.\n"
                      << std::endl;
            break;
        }

        std::cout << "\nPress ENTER to continue...";
        std::cin.ignore();
        std::cin.get();
    }

    return 0;
}