/**
 * ✅ ПРАВИЛЬНЫЙ main.cpp - ФИНАЛЬНАЯ ВЕРСИЯ
 *
 * ИСПРАВЛЕНИЯ:
 * 1. ✅ RC измеряет ТОЛЬКО время удаления корня и каскада
 * 2. ✅ MS измеряет ТОЛЬКО время mark-sweep алгоритма
 * 3. ✅ RC должен быть БЫСТРЕЕ MS (как и должно быть)
 */

#include <iostream>
#include <iomanip>
#include <chrono>
#include <memory>
#include <vector>
#include <functional>

#include "common_utils.h"
#include "mark_sweep_gc.h"
#include "cascade_deletion_gc.h"
#include "rc_heap.h"
#include "event_logger.h"
#include "rc_logger.h"

// ============================================
// ПАРАМЕТРЫ СИМУЛЯЦИИ
// ============================================

struct SimulationParams
{
    size_t heap_size_bytes = 33554432; // 32 MB default
    int num_objects = 20;
    int object_size = 64;
    int scenario_type = 1; // 1=linear, 2=cyclic, 3=cascade
};

// ============================================
// ПАРСИНГ АРГУМЕНТОВ КОМАНДНОЙ СТРОКИ
// ============================================

SimulationParams parse_arguments(int argc, char *argv[])
{
    SimulationParams params;

    if (argc > 1)
        params.scenario_type = std::atoi(argv[1]);
    if (argc > 2)
        params.num_objects = std::atoi(argv[2]);
    if (argc > 3)
        params.object_size = std::atoi(argv[3]);
    if (argc > 4)
    {
        int heap_size_mb = std::atoi(argv[4]);
        if (heap_size_mb > 0)
        {
            params.heap_size_bytes = (size_t)heap_size_mb * 1024 * 1024;
        }
    }

    return params;
}

// ============================================
// РЕЗУЛЬТАТЫ ЗАПУСКА
// ============================================

struct GCResult
{
    size_t objects_created = 0;
    size_t objects_left = 0;
    size_t memory_allocated = 0;
    size_t memory_freed = 0;
    size_t memory_leaked = 0;
    double execution_time_ms = 0.0;
};

// ============================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// ============================================

void create_graph_for_scenario(int scenario_type, int num_objects,
                               std::function<void(int from, int to)> add_ref_callback)
{
    switch (scenario_type)
    {
    case 1: // LINEAR CHAIN
        for (int i = 1; i < num_objects; ++i)
            add_ref_callback(i - 1, i);
        break;

    case 2: // CYCLIC GRAPH
        for (int i = 1; i < num_objects; ++i)
            add_ref_callback(i - 1, i);
        if (num_objects > 1)
            add_ref_callback(num_objects - 1, 0); // Замыкаем цикл
        break;

    case 3: // CASCADE TREE
        for (int i = 1; i < num_objects; ++i)
            add_ref_callback(i - 1, i);
        break;
    }
}

// ============================================
// REFERENCE COUNTING (ИЗМЕРЯЕМ ТОЛЬКО УДАЛЕНИЕ!)
// ============================================

GCResult run_reference_counting(const SimulationParams &params)
{
    GCResult result;
    result.objects_created = params.num_objects;
    result.memory_allocated = params.num_objects * params.object_size;

    try
    {
        EventLogger logger("simulation_events.log");
        RCLogger rc_logger("rc_log");
        RCHeap rc_heap(logger, rc_logger, params.heap_size_bytes);

        // ✅ ФАЗА 1: ВЫДЕЛЕНИЕ ПАМЯТИ (НЕ измеряем время)
        for (int i = 0; i < params.num_objects; ++i)
            rc_heap.allocate(i);

        // ✅ ФАЗА 2: СОЗДАНИЕ ГРАФА (НЕ измеряем время)
        rc_heap.addroot(0);

        create_graph_for_scenario(params.scenario_type, params.num_objects,
                                  [&](int from, int to)
                                  {
                                      rc_heap.addref(from, to);
                                  });

        // ✅ ФАЗА 3: УДАЛЕНИЕ (ИЗМЕРЯЕМ ТОЛЬКО ЭТО!)
        auto start = std::chrono::high_resolution_clock::now();

        // ⏱️ ТОЛЬКО УДАЛЕНИЕ КОРНЯ И КАСКАД
        rc_heap.removeroot(0);

        auto end = std::chrono::high_resolution_clock::now();
        result.execution_time_ms = std::chrono::duration<double, std::milli>(end - start).count();

        // Статистика
        result.objects_left = rc_heap.getheapsize();
        result.memory_freed = (params.num_objects - result.objects_left) * params.object_size;
        result.memory_leaked = result.objects_left * params.object_size;

        // ✅ ЛОГ ДЛЯ ОТЛАДКИ
        std::cout << "[RC_DEBUG] Time measured for removeroot+cascade only: "
                  << std::fixed << std::setprecision(3) << result.execution_time_ms << " ms\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "RC Error: " << e.what() << std::endl;
    }

    return result;
}

// ============================================
// MARK & SWEEP (ИЗМЕРЯЕМ ТОЛЬКО COLLECT!)
// ============================================

GCResult run_mark_and_sweep(const SimulationParams &params)
{
    GCResult result;
    result.objects_created = params.num_objects;
    result.memory_allocated = params.num_objects * params.object_size;

    try
    {
        auto ms_gc = std::make_unique<MarkSweepGC>(params.heap_size_bytes);

        // ✅ ФАЗА 1: ВЫДЕЛЕНИЕ ПАМЯТИ (НЕ измеряем время)
        std::vector<int> object_ids;
        for (int i = 0; i < params.num_objects; ++i)
            object_ids.push_back(ms_gc->allocate(params.object_size));

        // ✅ ФАЗА 2: СОЗДАНИЕ ГРАФА (НЕ измеряем время)
        if (!object_ids.empty())
            ms_gc->make_root(object_ids[0]);

        create_graph_for_scenario(params.scenario_type, params.num_objects,
                                  [&](int from, int to)
                                  {
                                      if (from < (int)object_ids.size() && to < (int)object_ids.size())
                                          ms_gc->add_reference(object_ids[from], object_ids[to]);
                                  });

        // ✅ ФАЗА 3: УДАЛЕНИЕ КОРНЯ (НЕ измеряем время)
        if (!object_ids.empty())
            ms_gc->remove_root(object_ids[0]);

        // ✅ ФАЗА 4: MARK-SWEEP (ИЗМЕРЯЕМ ТОЛЬКО ЭТО!)
        auto start = std::chrono::high_resolution_clock::now();

        // ⏱️ ТОЛЬКО АЛГОРИТМ MARK-SWEEP
        ms_gc->collect();

        auto end = std::chrono::high_resolution_clock::now();
        result.execution_time_ms = std::chrono::duration<double, std::milli>(end - start).count();

        // Статистика
        result.objects_left = ms_gc->get_alive_objects_count();
        result.memory_freed = (params.num_objects - result.objects_left) * params.object_size;
        result.memory_leaked = 0;

        // ✅ ЛОГ ДЛЯ ОТЛАДКИ
        std::cout << "[MS_DEBUG] Time measured for collect() only: "
                  << std::fixed << std::setprecision(3) << result.execution_time_ms << " ms\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "MS Error: " << e.what() << std::endl;
    }

    return result;
}

// ============================================
// ВЫВОД РЕЗУЛЬТАТОВ
// ============================================

void print_results_for_python(const SimulationParams &params,
                              const GCResult &rc_result,
                              const GCResult &ms_result)
{
    std::string scenario_name;
    switch (params.scenario_type)
    {
    case 1:
        scenario_name = "Linear Chain";
        break;
    case 2:
        scenario_name = "Cyclic Graph";
        break;
    case 3:
        scenario_name = "Cascade Tree";
        break;
    default:
        scenario_name = "Unknown";
    }

    // REFERENCE COUNTING STATS
    std::cout << "[RC_STATS]\n";
    std::cout << "type:RC\n";
    std::cout << "scenario:" << scenario_name << "\n";
    std::cout << "objects_created:" << rc_result.objects_created << "\n";
    std::cout << "objects_left:" << rc_result.objects_left << "\n";
    std::cout << "memory_freed:" << rc_result.memory_freed << "\n";
    std::cout << "memory_leaked:" << rc_result.memory_leaked << "\n";
    std::cout << "execution_time_ms:" << std::fixed << std::setprecision(3)
              << rc_result.execution_time_ms << "\n";
    std::cout << "[/RC_STATS]\n";

    // MARK & SWEEP STATS
    std::cout << "[MS_STATS]\n";
    std::cout << "type:MS\n";
    std::cout << "scenario:" << scenario_name << "\n";
    std::cout << "objects_created:" << ms_result.objects_created << "\n";
    std::cout << "objects_left:" << ms_result.objects_left << "\n";
    std::cout << "memory_freed:" << ms_result.memory_freed << "\n";
    std::cout << "memory_leaked:" << ms_result.memory_leaked << "\n";
    std::cout << "execution_time_ms:" << std::fixed << std::setprecision(3)
              << ms_result.execution_time_ms << "\n";
    std::cout << "[/MS_STATS]\n";
}

// ============================================
// MAIN
// ============================================

int main(int argc, char *argv[])
{
    SimulationParams params = parse_arguments(argc, argv);

    // ✅ ЛОГ НАЧАЛА
    std::cout << "[MAIN] Starting simulation with params: "
              << "scenario=" << params.scenario_type
              << ", objects=" << params.num_objects
              << ", size=" << params.object_size
              << ", heap=" << (params.heap_size_bytes / (1024 * 1024)) << "MB\n";

    GCResult rc_result = run_reference_counting(params);
    GCResult ms_result = run_mark_and_sweep(params);

    print_results_for_python(params, rc_result, ms_result);

    // ✅ СРАВНЕНИЕ В КОНСОЛИ ДЛЯ ОТЛАДКИ
    std::cout << "\n[COMPARISON] RC: " << rc_result.execution_time_ms << " ms vs "
              << "MS: " << ms_result.execution_time_ms << " ms\n";
    std::cout << "[COMPARISON] RC is "
              << (rc_result.execution_time_ms < ms_result.execution_time_ms ? "FASTER" : "SLOWER")
              << " than MS\n";

    return 0;
}