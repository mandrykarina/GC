#include "common_utils.h"
#include <cmath>
#include <vector>
#include <filesystem>

// ============================================
// MemoryStats implementations
// ============================================
void MemoryStats::print() const
{
    std::cout << "\n"
              << std::string(70, '=') << std::endl;
    std::cout << " Memory Statistics" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << " Total Allocated: " << std::setw(12) << total_allocated << " bytes";
    std::cout << " (" << std::fixed << std::setprecision(2)
              << (total_allocated / 1024.0 / 1024.0) << " MB)" << std::endl;
    std::cout << " Total Freed: " << std::setw(12) << total_freed << " bytes";
    std::cout << " (" << std::fixed << std::setprecision(2)
              << (total_freed / 1024.0 / 1024.0) << " MB)" << std::endl;
    std::cout << " Peak Memory: " << std::setw(12) << peak_memory << " bytes";
    std::cout << " (" << std::fixed << std::setprecision(2)
              << (peak_memory / 1024.0 / 1024.0) << " MB)" << std::endl;
    std::cout << " Memory Leaked: " << std::setw(12) << leaked_memory << " bytes";
    std::cout << " (" << std::fixed << std::setprecision(2)
              << (leaked_memory / 1024.0 / 1024.0) << " MB)";
    if (leaked_memory == 0)
    {
        std::cout << " ✓";
    }
    else
    {
        std::cout << " ⚠️ ERROR: MEMORY LEAK DETECTED!";
    }
    std::cout << std::endl;
    std::cout << " Memory Recovery: " << std::fixed << std::setprecision(1)
              << recovery_percent << "%" << std::endl;
}

bool ensure_directory_exists(const std::string &path)
{
    namespace fs = std::filesystem;
    try
    {
        return fs::create_directories(path);
    }
    catch (...)
    {
        return false;
    }
}

// ============================================
// MemoryConfig implementations
// ============================================
std::string MemoryConfig::format_bytes(size_t bytes) const
{
    if (bytes >= 1073741824)
        return std::to_string(bytes / 1073741824.0) + " GB";
    if (bytes >= 1048576)
        return std::to_string(bytes / 1048576.0) + " MB";
    if (bytes >= 1024)
        return std::to_string(bytes / 1024.0) + " KB";
    return std::to_string(bytes) + " B";
}

int MemoryConfig::calculate_max_objects() const
{
    if (object_size <= 0)
        return 0;
    return heap_size_bytes / object_size;
}

int MemoryConfig::calculate_max_object_size() const
{
    if (num_objects <= 0)
        return 0;
    return heap_size_bytes / num_objects;
}

size_t MemoryConfig::calculate_used_memory() const
{
    return (size_t)num_objects * object_size;
}

double MemoryConfig::calculate_heap_usage_percent() const
{
    if (heap_size_bytes == 0)
        return 0.0;
    return (calculate_used_memory() * 100.0) / heap_size_bytes;
}

bool MemoryConfig::validate()
{
    is_valid = false;
    if (heap_size_bytes < 10485760 || heap_size_bytes > 1073741824)
    {
        return false;
    }
    if (num_objects < 2)
    {
        return false;
    }
    if (object_size < 8)
    {
        return false;
    }
    int max_objects = calculate_max_objects();
    if (num_objects > max_objects)
    {
        return false;
    }
    int max_size = calculate_max_object_size();
    if (object_size > max_size)
    {
        return false;
    }
    is_valid = true;
    return true;
}

// ============================================
// Common functions implementations
// ============================================
size_t select_heap_size()
{
    std::cout << "\n"
              << std::string(70, '=') << std::endl;
    std::cout << "║ STEP 1: SELECT HEAP SIZE" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "║\n";
    std::cout << "║ Recommended range: 10 MB to 1 GB\n";
    std::cout << "║ Maximum allowed: 1 GB (1073741824 bytes)\n";
    std::cout << "║\n";
    std::cout << "║ Presets:\n";
    std::cout << "║ 1) 10 MB (10485760 bytes) - Small tests\n";
    std::cout << "║ 2) 50 MB (52428800 bytes) - Medium tests\n";
    std::cout << "║ 3) 100 MB (104857600 bytes) - Standard (default)\n";
    std::cout << "║ 4) 500 MB (524288000 bytes) - Large tests\n";
    std::cout << "║ 5) 1 GB (1073741824 bytes) - Very large tests\n";
    std::cout << "║ 6) Custom - Enter custom size in MB\n";
    std::cout << std::string(70, '=') << "\n"
              << std::endl;

    int choice = 0;
    while (true)
    {
        std::cout << "Select option (1-6): ";
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input!\n";
            continue;
        }
        switch (choice)
        {
        case 1:
            std::cout << "Heap size selected: 10 MB\n";
            return 10485760;
        case 2:
            std::cout << "Heap size selected: 50 MB\n";
            return 52428800;
        case 3:
            std::cout << "Heap size selected: 100 MB\n";
            return 104857600;
        case 4:
            std::cout << "Heap size selected: 500 MB\n";
            return 524288000;
        case 5:
            std::cout << "Heap size selected: 1 GB\n";
            return 1073741824;
        case 6:
        {
            int mb = 0;
            std::cout << "Enter heap size in MB (10-1024): ";
            if (!(std::cin >> mb))
            {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                std::cout << "Invalid input!\n";
                continue;
            }
            if (mb < 10 || mb > 1024)
            {
                std::cout << "Must be between 10 and 1024 MB!\n";
                continue;
            }
            size_t bytes = (size_t)mb * 1048576;
            std::cout << "Heap size selected: " << mb << " MB\n";
            return bytes;
        }
        default:
            std::cout << "Invalid choice! Please select 1-6.\n";
        }
    }
}

int select_object_count(size_t heap_size_bytes)
{
    std::cout << "\n"
              << std::string(70, '=') << std::endl;
    std::cout << "║ STEP 2: SELECT NUMBER OF OBJECTS" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    MemoryConfig temp;
    temp.heap_size_bytes = heap_size_bytes;
    int recommended_min = 2;
    int recommended_max = std::min(20000, (int)(heap_size_bytes / 8));
    std::cout << "║\n";
    std::cout << "║ Heap Size: " << temp.format_bytes(heap_size_bytes) << "\n";
    std::cout << "║ Recommended range: " << recommended_min << " to " << recommended_max << "\n";
    std::cout << "║ Absolute maximum: " << (heap_size_bytes / 8) << "\n";
    std::cout << std::string(70, '=') << "\n"
              << std::endl;

    int count = 0;
    while (true)
    {
        std::cout << "Enter number of objects: ";
        if (!(std::cin >> count))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input!\n";
            continue;
        }
        if (count < 2)
        {
            std::cout << "Minimum 2 objects required!\n";
            continue;
        }
        if (count > (int)(heap_size_bytes / 8))
        {
            std::cout << "Too many objects for this heap size (max: "
                      << (heap_size_bytes / 8) << ")\n";
            continue;
        }
        if (count < recommended_min || count > recommended_max)
        {
            std::cout << "Warning: " << count << " is outside recommended range ("
                      << recommended_min << "-" << recommended_max << ")\n";
            std::cout << "Continue anyway? (y/n): ";
            char confirm;
            std::cin >> confirm;
            if (confirm != 'y' && confirm != 'Y')
            {
                continue;
            }
        }
        std::cout << "Object count selected: " << count << "\n";
        return count;
    }
}

int select_object_size(size_t heap_size_bytes, int num_objects)
{
    std::cout << "\n"
              << std::string(70, '=') << std::endl;
    std::cout << "║ STEP 3: SELECT OBJECT SIZE (in bytes)" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    MemoryConfig temp;
    temp.heap_size_bytes = heap_size_bytes;
    temp.num_objects = num_objects;
    int max_size = heap_size_bytes / num_objects;
    int recommended_min = 8;
    int recommended_max = std::min(10485, max_size);
    std::cout << "║\n";
    std::cout << "║ Heap Size: " << temp.format_bytes(heap_size_bytes) << "\n";
    std::cout << "║ Number of Objects: " << num_objects << "\n";
    std::cout << "║ Recommended range: " << recommended_min << " to " << recommended_max << " bytes\n";
    std::cout << "║\n";
    std::cout << "║ Presets:\n";
    std::vector<std::pair<int, std::string>> presets = {
        {8, "Tiny"},
        {16, "Very small"},
        {32, "Small"},
        {64, "Standard (default)"},
        {128, "Medium"},
        {256, "Large"},
        {512, "Very large"},
        {1024, "1 KB"}};
    int option = 1;
    for (auto &preset : presets)
    {
        if (preset.first <= max_size)
        {
            size_t total_bytes = (size_t)num_objects * preset.first;
            std::cout << "║ " << option << ") " << std::setw(5) << preset.first
                      << " bytes - Total: " << std::fixed << std::setprecision(2)
                      << (total_bytes / 1048576.0) << " MB";
            if (preset.first == 64)
            {
                std::cout << " (default)";
            }
            std::cout << "\n";
            option++;
        }
    }
    std::cout << "║ " << option << ") Custom - Enter custom size in bytes\n";
    std::cout << std::string(70, '=') << "\n"
              << std::endl;

    int choice = 0;
    while (true)
    {
        std::cout << "Select option (1-" << option << "): ";
        if (!(std::cin >> choice))
        {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Invalid input!\n";
            continue;
        }
        std::vector<int> preset_sizes = {8, 16, 32, 64, 128, 256, 512, 1024};
        if (choice >= 1 && choice <= (int)preset_sizes.size())
        {
            int size = preset_sizes[choice - 1];
            if (size <= max_size)
            {
                std::cout << "Object size selected: " << size << " bytes\n";
                return size;
            }
            else
            {
                std::cout << "Size too large for this configuration (max: " << max_size << ")\n";
                continue;
            }
        }
        else if (choice == (int)preset_sizes.size() + 1)
        {
            int custom_size = 0;
            std::cout << "Enter object size in bytes (8-" << max_size << "): ";
            if (!(std::cin >> custom_size))
            {
                std::cin.clear();
                std::cin.ignore(10000, '\n');
                std::cout << "Invalid input!\n";
                continue;
            }
            if (custom_size < 8 || custom_size > max_size)
            {
                std::cout << "Size must be between 8 and " << max_size << " bytes!\n";
                continue;
            }
            std::cout << "Object size selected: " << custom_size << " bytes\n";
            return custom_size;
        }
        else
        {
            std::cout << "Invalid choice!\n";
        }
    }
}

void print_configuration_summary(const MemoryConfig &config)
{
    std::cout << "\n"
              << std::string(70, '=') << std::endl;
    std::cout << "║ MEMORY CONFIGURATION SUMMARY\n";
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "║\n";
    std::cout << "║ Heap Size: " << config.format_bytes(config.heap_size_bytes) << "\n";
    std::cout << "║ Number of Objects: " << config.num_objects << "\n";
    std::cout << "║ Size per Object: " << config.object_size << " bytes\n";
    std::cout << "║\n";
    std::cout << "║ Total Memory Needed: " << config.format_bytes(config.calculate_used_memory()) << "\n";
    std::cout << "║ Max Objects (possible): " << config.calculate_max_objects() << "\n";
    std::cout << "║ Heap Usage: " << std::fixed << std::setprecision(1)
              << config.calculate_heap_usage_percent() << "%\n";
    std::cout << std::string(70, '=') << "\n"
              << std::endl;
}

MemoryConfig interactive_memory_config()
{
    std::cout << "\n"
              << std::string(70, '=') << std::endl;
    std::cout << "║ INTERACTIVE MEMORY & HEAP CONFIGURATION" << std::endl;
    std::cout << std::string(70, '=') << std::endl;
    std::cout << "║\n";
    std::cout << "║ Heap size (10 MB to 1 GB)\n";
    std::cout << "║ Number of objects\n";
    std::cout << "║ Size per object (dynamic limits)\n";
    std::cout << "║\n";
    std::cout << "║ Press Enter to start...\n";
    std::cout << std::string(70, '=') << std::endl;
    std::string dummy;
    std::getline(std::cin, dummy);
    std::getline(std::cin, dummy);

    size_t heap_size = select_heap_size();
    int obj_count = select_object_count(heap_size);
    int obj_size = select_object_size(heap_size, obj_count);

    MemoryConfig config;
    config.heap_size_bytes = heap_size;
    config.num_objects = obj_count;
    config.object_size = obj_size;
    config.validate();

    return config;
}

bool file_exists(const std::string &f)
{
    std::ifstream file(f);
    return file.good();
}

std::string find_scenario(const std::string &name)
{
    std::vector<std::string> paths = {
        name,
        "scenarios/" + name,
        "../scenarios/" + name,
        "../../scenarios/" + name};
    for (const auto &p : paths)
    {
        if (file_exists(p))
            return p;
    }
    return "scenarios/" + name;
}

std::string save_generated_json(const std::string &json_content, const std::string &filename)
{
    // Создаем директорию, если ее нет
    size_t last_slash = filename.find_last_of('/');
    if (last_slash != std::string::npos)
    {
        std::string dir = filename.substr(0, last_slash);
        if (!dir.empty())
        {
            ensure_directory_exists(dir);
        }
    }

    std::ofstream file(filename);
    if (!file.is_open())
    {
        std::cerr << "Cannot create " << filename << std::endl;
        return "";
    }
    file << json_content;
    file.close();
    return filename;
}

// ============================================
// JSON Generation functions
// ============================================
std::string generate_linear_chain_json(int num_objects, int object_size, size_t heap_size)
{
    (void)object_size;
    std::stringstream json;
    json << "{\n";
    json << "  \"name\": \"Generated Linear Chain\",\n";
    json << "  \"description\": \"Linear chain of " << num_objects << " objects\",\n";
    json << "  \"collection_type\": \"reference_counting\",\n";
    json << "  \"heap_size\": " << heap_size << ",\n";
    json << "  \"operations\": [\n";

    for (int i = 0; i < num_objects; ++i)
    {
        json << "    { \"op\": \"allocate\", \"id\": " << i << " }";
        if (i < num_objects - 1)
            json << ",";
        json << "\n";
    }

    json << "    ,{ \"op\": \"addroot\", \"id\": 0 }\n";

    for (int i = 1; i < num_objects; ++i)
    {
        json << "    ,{ \"op\": \"addref\", \"from\": " << (i - 1) << ", \"to\": " << i << " }\n";
    }

    json << "    ,{ \"op\": \"removeroot\", \"id\": 0 }\n";
    json << "  ]\n";
    json << "}\n";
    return json.str();
}

std::string generate_linear_chain_json_ms(int num_objects, int object_size, size_t heap_size)
{
    std::stringstream json;
    json << "{\n";
    json << " \"name\": \"Generated Linear Chain\",\n";
    json << " \"description\": \"Linear chain of " << num_objects << " objects\",\n";
    json << " \"collection_type\": \"mark_sweep\",\n";
    json << " \"heap_size\": " << heap_size << ",\n";
    json << " \"operations\": [\n";
    json << " { \"op\": \"allocate\", \"size\": " << object_size << " },\n";
    json << " { \"op\": \"make_root\", \"id\": 0 },\n";
    for (int i = 1; i < num_objects; ++i)
    {
        json << " { \"op\": \"allocate\", \"size\": " << object_size << " },\n";
        json << " { \"op\": \"add_ref\", \"from\": " << (i - 1) << ", \"to\": " << i << " }";
        if (i < num_objects - 1)
            json << ",";
        json << "\n";
    }
    json << " ,{ \"op\": \"remove_root\", \"id\": 0 },\n";
    json << " { \"op\": \"collect\" }\n";
    json << " ]\n";
    json << "}\n";
    return json.str();
}

// ============================================
// MS-specific JSON Generation functions
// ============================================

std::string generate_cyclic_graph_json_ms(int num_objects, int object_size, size_t heap_size)
{
    (void)object_size;
    std::stringstream json;
    json << "{\n";
    json << " \"name\": \"Generated Cyclic Graph\",\n";
    json << " \"description\": \"Cyclic graph of " << num_objects << " objects\",\n";
    json << " \"collection_type\": \"mark_sweep\",\n";
    json << " \"heap_size\": " << heap_size << ",\n";
    json << " \"operations\": [\n";

    // Корневой объект
    json << " { \"op\": \"allocate\", \"size\": " << object_size << " },\n";
    json << " { \"op\": \"make_root\", \"id\": 0 }";

    int cycle_length = std::min(3, num_objects - 1);
    if (cycle_length < 2)
        cycle_length = 2;

    // Создаем остальные объекты
    for (int i = 1; i < num_objects; ++i)
    {
        json << ",\n";
        json << " { \"op\": \"allocate\", \"size\": " << object_size << " }";
    }

    // Создаем циклические ссылки
    for (int i = 0; i < num_objects - 1; ++i)
    {
        int from = i;
        int to = (i + 1) % (num_objects - 1);
        if (to == 0)
            to = 1;

        json << ",\n";
        json << " { \"op\": \"add_ref\", \"from\": " << from << ", \"to\": " << to << " }";
    }

    // Замыкаем цикл
    int last = num_objects - 1;
    if (last > 0)
    {
        json << ",\n";
        json << " { \"op\": \"add_ref\", \"from\": " << last << ", \"to\": 1 }";
    }

    json << ",\n";
    json << " { \"op\": \"remove_root\", \"id\": 0 },\n";
    json << " { \"op\": \"collect\" }\n";
    json << " ]\n";
    json << "}\n";
    return json.str();
}

std::string generate_cascade_tree_json_ms(int num_objects, int object_size, size_t heap_size)
{
    (void)object_size;
    std::stringstream json;
    json << "{\n";
    json << " \"name\": \"Generated Cascade Tree\",\n";
    json << " \"description\": \"Cascade tree of " << num_objects << " objects\",\n";
    json << " \"collection_type\": \"mark_sweep\",\n";
    json << " \"heap_size\": " << heap_size << ",\n";
    json << " \"operations\": [\n";

    // Корневой объект
    json << " { \"op\": \"allocate\", \"size\": " << object_size << " },\n";
    json << " { \"op\": \"make_root\", \"id\": 0 }";

    // Дочерние объекты (каждый ссылается на следующий)
    for (int i = 1; i < num_objects; ++i)
    {
        json << ",\n";
        json << " { \"op\": \"allocate\", \"size\": " << object_size << " },\n";
        json << " { \"op\": \"add_ref\", \"from\": " << (i - 1) << ", \"to\": " << i << " }";
    }

    // Удаление корня вызовет каскадное удаление
    json << ",\n";
    json << " { \"op\": \"remove_root\", \"id\": 0 },\n";
    json << " { \"op\": \"collect\" }\n";
    json << " ]\n";
    json << "}\n";
    return json.str();
}

std::string generate_cyclic_graph_json(int num_objects, int object_size, size_t heap_size)
{
    (void)object_size;
    std::stringstream json;
    json << "{\n";
    json << "  \"name\": \"Generated Cyclic Graph\",\n";
    json << "  \"description\": \"Cyclic graph of " << num_objects << " objects - demonstrates RC leak\",\n";
    json << "  \"collection_type\": \"reference_counting\",\n";
    json << "  \"heap_size\": " << heap_size << ",\n";
    json << "  \"operations\": [\n";

    for (int i = 0; i < num_objects; ++i)
    {
        json << "    { \"op\": \"allocate\", \"id\": " << i << " }";
        if (i < num_objects - 1)
            json << ",";
        json << "\n";
    }

    json << "    ,{ \"op\": \"addroot\", \"id\": 0 }\n";

    int cycle_length = std::min(3, num_objects - 1);
    for (int i = 1; i < num_objects; ++i)
    {
        int to = (i % cycle_length) + 1;
        if (to >= num_objects)
            to = 1;
        json << "    ,{ \"op\": \"addref\", \"from\": " << (i - 1) << ", \"to\": " << to << " }\n";
    }

    for (int i = 1; i < std::min(cycle_length + 1, num_objects); ++i)
    {
        int from = i;
        int to = (i % cycle_length) == 0 ? 1 : (i % cycle_length) + 1;
        if (to >= num_objects)
            to = 1;
        json << "    ,{ \"op\": \"addref\", \"from\": " << from << ", \"to\": " << to << " }\n";
    }

    json << "    ,{ \"op\": \"removeroot\", \"id\": 0 }\n";
    json << "  ]\n";
    json << "}\n";
    return json.str();
}

std::string generate_cascade_tree_json(int num_objects, int object_size, size_t heap_size)
{
    (void)object_size;
    std::stringstream json;
    json << "{\n";
    json << "  \"name\": \"Generated Cascade Tree\",\n";
    json << "  \"description\": \"Cascade tree of " << num_objects << " objects\",\n";
    json << "  \"collection_type\": \"reference_counting\",\n";
    json << "  \"heap_size\": " << heap_size << ",\n";
    json << "  \"operations\": [\n";

    for (int i = 0; i < num_objects; ++i)
    {
        json << "    { \"op\": \"allocate\", \"id\": " << i << " }";
        if (i < num_objects - 1)
            json << ",";
        json << "\n";
    }

    json << "    ,{ \"op\": \"addroot\", \"id\": 0 }\n";

    for (int i = 1; i < num_objects; ++i)
    {
        json << "    ,{ \"op\": \"addref\", \"from\": " << (i - 1) << ", \"to\": " << i << " }\n";
    }

    json << "    ,{ \"op\": \"removeroot\", \"id\": 0 }\n";
    json << "  ]\n";
    json << "}\n";
    return json.str();
}