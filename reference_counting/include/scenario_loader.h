#ifndef SCENARIO_LOADER_H
#define SCENARIO_LOADER_H

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct Operation {
    std::string op;
    int id = -1;
    int size = 0;
    int from = -1;
    int to = -1;
    std::string description;
};

struct Scenario {
    std::string scenario_name;
    std::string description;
    int max_heap_size = 1048576;
    std::vector<Operation> operations;
};

class ScenarioLoader {
public:
    static Scenario loadScenario(const std::string& json_path);
    static std::vector<Scenario> loadAllScenarios(const std::string& scenarios_dir);

private:
    static Operation parseOperation(const json& op_json);
};

#endif
