#ifndef COMMON_UTILS_H
#define COMMON_UTILS_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <string>

// ============================================
// COMMON STRUCTURES
// ============================================
struct MemoryStats {
    size_t total_allocated = 0;
    size_t total_freed = 0;
    size_t peak_memory = 0;
    size_t leaked_memory = 0;
    double recovery_percent = 0.0;

    void calculate() {
        leaked_memory = total_allocated - total_freed;
        recovery_percent = (total_allocated > 0) ?
            (total_freed * 100.0 / total_allocated) : 0.0;
    }

    void print() const;
};

struct MemoryConfig {
    size_t heap_size_bytes = 0;
    int num_objects = 0;
    int object_size = 0;
    bool is_valid = false;

    std::string format_bytes(size_t bytes) const;
    int calculate_max_objects() const;
    int calculate_max_object_size() const;
    size_t calculate_used_memory() const;
    double calculate_heap_usage_percent() const;
    bool validate();
};

struct Operation {
    std::string type;
    int id = -1;
    int from = -1;
    int to = -1;
    int param1 = 0;
    int param2 = 0;
    std::string collection_type = "mark_sweep";
};

struct ParsedScenario {
    std::vector<Operation> operations;
    size_t heap_size = 104857600;
};

// ============================================
// COMMON FUNCTIONS
// ============================================
size_t select_heap_size();
int select_object_count(size_t heap_size_bytes);
int select_object_size(size_t heap_size_bytes, int num_objects);
void print_configuration_summary(const MemoryConfig& config);
MemoryConfig interactive_memory_config();
bool file_exists(const std::string& f);
bool ensure_directory_exists(const std::string& path);
std::string find_scenario(const std::string& name);
std::string save_generated_json(const std::string& json_content, const std::string& filename);

// JSON Generation functions (with unused parameters fixed)
std::string generate_linear_chain_json(int num_objects, int object_size, size_t heap_size);
std::string generate_cyclic_graph_json(int num_objects, int object_size, size_t heap_size);
std::string generate_cascade_tree_json(int num_objects, int object_size, size_t heap_size);
std::string generate_linear_chain_json_ms(int num_objects, int object_size, size_t heap_size);
std::string generate_cyclic_graph_json_ms(int num_objects, int object_size, size_t heap_size);
std::string generate_cascade_tree_json_ms(int num_objects, int object_size, size_t heap_size);
#endif // COMMON_UTILS_H