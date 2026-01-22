#include "mark_sweep_gc.h"
#include "cascade_deletion_gc.h"
#include "performance_test.h"
#include "common_utils.h"
#include <iostream>
#include <memory>

// ============================================
// MARK-SWEEP SPECIFIC FUNCTIONS
// ============================================
void show_ms_menu() {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << " Mark-Sweep Garbage Collector - Main Menu" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "\n [1] Run Scenario: Basic (Mark-Sweep)" << std::endl;
    std::cout << " [2] Run Scenario: Cyclic Graph (Mark-Sweep)" << std::endl;
    std::cout << " [3] Run Scenario: Cascade Deletion" << std::endl;
    std::cout << " [4] Run Scenario: Performance Test" << std::endl;
    std::cout << " [5] Run All Scenarios" << std::endl;
    std::cout << " [6] Run All Scenarios + Performance Tests" << std::endl;
    std::cout << "\n" << std::string(80, '-') << std::endl;
    std::cout << " CUSTOM GENERATION (with Interactive Config)" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    std::cout << " [7] Generate & Run: Linear Chain" << std::endl;
    std::cout << " [8] Generate & Run: Cyclic Graph" << std::endl;
    std::cout << " [9] Generate & Run: Cascade Tree" << std::endl;
    std::cout << " [0] Exit\n" << std::endl;
}

ParsedScenario parse_json_scenario_ms(const std::string& filename) {
    ParsedScenario result;
    std::vector<Operation>& operations = result.operations;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open: " << filename << std::endl;
        return result;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // Read heap_size from JSON
    size_t heap_size_pos = content.find("\"heap_size\":");
    if (heap_size_pos != std::string::npos) {
        size_t colon_pos = content.find(":", heap_size_pos);
        size_t comma_pos = content.find(",", colon_pos);
        if (comma_pos == std::string::npos) {
            comma_pos = content.find("}", colon_pos);
        }
        if (colon_pos != std::string::npos && comma_pos != std::string::npos) {
            std::string heap_size_str = content.substr(colon_pos + 1, comma_pos - colon_pos - 1);
            heap_size_str.erase(0, heap_size_str.find_first_not_of(" \t\n\r"));
            heap_size_str.erase(heap_size_str.find_last_not_of(" \t\n\r") + 1);
            try {
                result.heap_size = std::stoull(heap_size_str);
            } catch (...) {
                result.heap_size = 1048576;
            }
        }
    }

    std::string collection_type = "mark_sweep";
    size_t type_pos = content.find("\"collection_type\":");
    if (type_pos != std::string::npos) {
        size_t start = content.find("\"", type_pos + 18);
        size_t end = content.find("\"", start + 1);
        if (start != std::string::npos && end != std::string::npos) {
            collection_type = content.substr(start + 1, end - start - 1);
        }
    }

    size_t pos = 0;
    while (pos < content.length()) {
        pos = content.find('{', pos);
        if (pos == std::string::npos) break;
        size_t end = content.find('}', pos);
        if (end == std::string::npos) break;
        std::string obj = content.substr(pos, end - pos + 1);
        pos = end + 1;

        if (obj.find("\"op\"") == std::string::npos) continue;

        Operation op;
        op.collection_type = collection_type;

        if (obj.find("\"op\": \"allocate\"") != std::string::npos ||
            obj.find("\"op\":\"allocate\"") != std::string::npos) {
            op.type = "allocate";
            size_t size_pos = obj.find("\"size\"");
            if (size_pos != std::string::npos) {
                sscanf(obj.c_str() + size_pos, "\"size\": %d", &op.param1);
            }
            if (op.param1 > 0) operations.push_back(op);

        } else if (obj.find("\"op\": \"make_root\"") != std::string::npos ||
                   obj.find("\"op\":\"make_root\"") != std::string::npos) {
            op.type = "make_root";
            size_t id_pos = obj.find("\"id\"");
            if (id_pos != std::string::npos) {
                sscanf(obj.c_str() + id_pos, "\"id\": %d", &op.param1);
            }
            operations.push_back(op);

        } else if (obj.find("\"op\": \"add_ref\"") != std::string::npos ||
                   obj.find("\"op\":\"add_ref\"") != std::string::npos) {
            op.type = "add_ref";
            size_t from_pos = obj.find("\"from\"");
            size_t to_pos = obj.find("\"to\"");
            if (from_pos != std::string::npos) {
                sscanf(obj.c_str() + from_pos, "\"from\": %d", &op.param1);
            }
            if (to_pos != std::string::npos) {
                sscanf(obj.c_str() + to_pos, "\"to\": %d", &op.param2);
            }
            operations.push_back(op);

        } else if (obj.find("\"op\": \"remove_root\"") != std::string::npos ||
                   obj.find("\"op\":\"remove_root\"") != std::string::npos) {
            op.type = "remove_root";
            size_t id_pos = obj.find("\"id\"");
            if (id_pos != std::string::npos) {
                sscanf(obj.c_str() + id_pos, "\"id\": %d", &op.param1);
            }
            operations.push_back(op);

        } else if (obj.find("\"op\": \"collect\"") != std::string::npos ||
                   obj.find("\"op\":\"collect\"") != std::string::npos) {
            op.type = "collect";
            operations.push_back(op);
        }
    }

    return result;
}


void run_simulation_ms(const std::string& scenario_file, const std::string& scenario_name) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << " Running: " << scenario_name << std::endl;
    std::cout << std::string(70, '=') << "\n" << std::endl;

    ParsedScenario parsed = parse_json_scenario_ms(scenario_file);
    std::vector<Operation>& operations = parsed.operations;
    size_t heap_size = parsed.heap_size;

    std::cout << "[*] Parsed " << operations.size() << " operations\n" << std::endl;
    std::cout << "[*] Heap Size: " << (heap_size / 1048576) << " MB\n" << std::endl;

    if (operations.empty()) {
        std::cerr << "No operations found!" << std::endl;
        return;
    }

    std::string gc_type = operations[0].collection_type;
    std::unique_ptr<GCInterface> gc;

    if (gc_type == "cascade") {
        std::cout << "[*] Garbage Collector: Cascade Deletion\n" << std::endl;
        gc = std::make_unique<CascadeDeletionGC>(heap_size);
    } else {
        std::cout << "[*] Garbage Collector: Mark-and-Sweep\n" << std::endl;
        gc = std::make_unique<MarkSweepGC>(heap_size);
    }

    MemoryStats mem_stats;
    auto start_time = std::chrono::high_resolution_clock::now();

    for (size_t step = 0; step < operations.size(); step++) {
        const Operation& op = operations[step];
        gc->set_current_step(step);

        if (op.type == "allocate") {
            int id = gc->allocate(op.param1);
            mem_stats.total_allocated += op.param1;
            size_t current_heap = gc->get_total_memory();
            if (current_heap > mem_stats.peak_memory) {
                mem_stats.peak_memory = current_heap;
            }
            std::cout << " [" << std::setw(3) << step << "] ALLOCATE "
                << std::setw(6) << op.param1 << " bytes -> object_" << id << std::endl;

        } else if (op.type == "make_root") {
            if (auto* ms_gc = dynamic_cast<MarkSweepGC*>(gc.get())) {
                ms_gc->make_root(op.param1);
            } else if (auto* c_gc = dynamic_cast<CascadeDeletionGC*>(gc.get())) {
                c_gc->make_root(op.param1);
            }
            std::cout << " [" << std::setw(3) << step << "] MAKE_ROOT object_" << op.param1 << std::endl;

        } else if (op.type == "add_ref") {
            gc->add_reference(op.param1, op.param2);
            std::cout << " [" << std::setw(3) << step << "] ADD_REF object_"
                << op.param1 << " -> object_" << op.param2 << std::endl;

        } else if (op.type == "remove_root") {
            if (auto* ms_gc = dynamic_cast<MarkSweepGC*>(gc.get())) {
                ms_gc->remove_root(op.param1);
            } else if (auto* c_gc = dynamic_cast<CascadeDeletionGC*>(gc.get())) {
                c_gc->remove_root(op.param1);
            }
            std::cout << " [" << std::setw(3) << step << "] REMOVE_ROOT object_" << op.param1 << std::endl;

        } else if (op.type == "collect") {
            size_t freed = gc->collect();
            mem_stats.total_freed += freed;
            std::cout << " [" << std::setw(3) << step << "] COLLECT -> freed "
                << freed << " bytes" << std::endl;
            std::cout << " Heap: " << gc->get_alive_objects_count()
                << " objects, " << gc->get_total_memory() << " bytes" << std::endl;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    double exec_time = std::chrono::duration<double, std::milli>(
        end_time - start_time).count();

    mem_stats.calculate();
    mem_stats.print();

    std::cout << std::string(70, '=') << std::endl;
    std::cout << " GC Statistics" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << gc->get_gc_stats();
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "\n Execution Time: " << exec_time << " ms\n" << std::endl;
}

int ms_main(int argc, char* argv[]) {
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << " MARK-SWEEP GARBAGE COLLECTOR" << std::endl;
    std::cout << " Integrated Test Suite" << std::endl;
    std::cout << std::string(80, '=') << "\n" << std::endl;

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "1") {
            run_simulation_ms(find_scenario("scenario_basic.json"), "Basic Scenario");
        } else if (arg == "2") {
            run_simulation_ms(find_scenario("scenario_cycle.json"), "Cyclic Graph Scenario");
        } else if (arg == "3") {
            run_simulation_ms(find_scenario("scenario_cascade.json"), "Cascade Deletion Scenario");
        } else if (arg == "4") {
            PerformanceTest perf_test("./perf_results");
            perf_test.run_all_tests(100, 1000, 10000);
            perf_test.save_results_to_json("performance_results.json");
        } else if (arg == "all") {
            run_simulation_ms(find_scenario("scenario_basic.json"), "Basic Scenario");
            run_simulation_ms(find_scenario("scenario_cycle.json"), "Cyclic Graph Scenario");
            run_simulation_ms(find_scenario("scenario_cascade.json"), "Cascade Deletion Scenario");
            run_simulation_ms(find_scenario("scenario_performance.json"), "Performance Scenario");
        } else if (arg == "perf") {
            PerformanceTest perf_test("./perf_results");
            perf_test.run_all_tests(100, 1000, 10000);
            perf_test.save_results_to_json("performance_results.json");
        }
        return 0;
    }

    while (true) {
        show_ms_menu();
        std::cout << "Enter your choice: ";
        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1:
                run_simulation_ms(find_scenario("scenario_basic.json"), "Basic Scenario");
                break;

            case 2:
                run_simulation_ms(find_scenario("scenario_cycle.json"), "Cyclic Graph Scenario");
                break;

            case 3:
                run_simulation_ms(find_scenario("scenario_cascade.json"), "Cascade Deletion Scenario");
                break;

            case 4: {
                PerformanceTest perf_test("./perf_results");
                perf_test.run_all_tests(100, 1000, 10000);
                perf_test.save_results_to_json("performance_results.json");
                break;
            }

            case 5: {
                run_simulation_ms(find_scenario("scenario_basic.json"), "Basic Scenario");
                run_simulation_ms(find_scenario("scenario_cycle.json"), "Cyclic Graph Scenario");
                run_simulation_ms(find_scenario("scenario_cascade.json"), "Cascade Deletion Scenario");
                run_simulation_ms(find_scenario("scenario_performance.json"), "Performance Scenario");
                break;
            }

            case 6: {
                run_simulation_ms(find_scenario("scenario_basic.json"), "Basic Scenario");
                run_simulation_ms(find_scenario("scenario_cycle.json"), "Cyclic Graph Scenario");
                run_simulation_ms(find_scenario("scenario_cascade.json"), "Cascade Deletion Scenario");
                run_simulation_ms(find_scenario("scenario_performance.json"), "Performance Scenario");
                std::cout << "\n\n";
                PerformanceTest perf_test("./perf_results");
                perf_test.run_all_tests(100, 1000, 10000);
                perf_test.save_results_to_json("performance_results.json");
                break;
            }

            case 7: {
                std::cout << "\n" << std::string(80, '=') << std::endl;
                std::cout << " CUSTOM: Generate Linear Chain" << std::endl;
                std::cout << std::string(80, '=') << std::endl;
                MemoryConfig config = interactive_memory_config();
                if (config.is_valid) {
                    print_configuration_summary(config);
                    std::cout << "\n[*] Generating JSON...\n" << std::endl;
                    std::string json_content = generate_linear_chain_json_ms(
                        config.num_objects, 
                        config.object_size,
                        config.heap_size_bytes
                    );
                    std::string json_file = save_generated_json(json_content, "generated_linear_" + std::to_string(config.num_objects) + ".json");
                    if (!json_file.empty()) {
                        std::cout << "[OK] JSON saved to: " << json_file << std::endl;
                        std::cout << "[*] Running simulation...\n" << std::endl;
                        run_simulation_ms(json_file, "Generated Linear Chain (" + std::to_string(config.num_objects) + " objects)");
                    }
                } else {
                    std::cout << "Configuration is invalid.\n";
                }
                break;
            }

            case 8: {
                std::cout << "\n" << std::string(80, '=') << std::endl;
                std::cout << " CUSTOM: Generate Cyclic Graph" << std::endl;
                std::cout << std::string(80, '=') << std::endl;
                MemoryConfig config = interactive_memory_config();
                if (config.is_valid) {
                    print_configuration_summary(config);
                    std::cout << "\n[*] Generating JSON...\n" << std::endl;
                    std::string json_content = generate_cyclic_graph_json_ms(
                        config.num_objects, 
                        config.object_size,
                        config.heap_size_bytes
                    );
                    std::string json_file = save_generated_json(json_content, "generated_cyclic_" + std::to_string(config.num_objects) + ".json");
                    if (!json_file.empty()) {
                        std::cout << "[OK] JSON saved to: " << json_file << std::endl;
                        std::cout << "[*] Running simulation...\n" << std::endl;
                        run_simulation_ms(json_file, "Generated Cyclic Graph (" + std::to_string(config.num_objects) + " objects)");
                    }
                } else {
                    std::cout << "Configuration is invalid.\n";
                }
                break;
            }

            case 9: {
                std::cout << "\n" << std::string(80, '=') << std::endl;
                std::cout << " CUSTOM: Generate Cascade Tree" << std::endl;
                std::cout << std::string(80, '=') << std::endl;
                MemoryConfig config = interactive_memory_config();
                if (config.is_valid) {
                    print_configuration_summary(config);
                    std::cout << "\n[*] Generating JSON...\n" << std::endl;
                    std::string json_content = generate_cascade_tree_json_ms(
                        config.num_objects, 
                        config.object_size,
                        config.heap_size_bytes
                    );
                    std::string json_file = save_generated_json(json_content, "generated_cascade_" + std::to_string(config.num_objects) + ".json");
                    if (!json_file.empty()) {
                        std::cout << "[OK] JSON saved to: " << json_file << std::endl;
                        std::cout << "[*] Running simulation...\n" << std::endl;
                        run_simulation_ms(json_file, "Generated Cascade Tree (" + std::to_string(config.num_objects) + " objects)");
                    }
                } else {
                    std::cout << "Configuration is invalid.\n";
                }
                break;
            }

            case 0:
                std::cout << "\nGoodbye!\n" << std::endl;
                return 0;

            default:
                std::cout << "\nInvalid choice. Try again.\n" << std::endl;
                break;
        }

        std::cout << "\nPress ENTER to continue...";
        std::cin.ignore();
        std::cin.get();
    }

    return 0;
}