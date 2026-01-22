#include "scenario_loader.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

Scenario ScenarioLoader::loadScenario(const std::string& json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open scenario file: " + json_path);
    }

    json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error("Invalid JSON in " + json_path + ": " + e.what());
    }

    Scenario scenario;
    scenario.scenario_name = j.value("scenario_name", "Unknown");
    scenario.description = j.value("description", "");
    scenario.max_heap_size = j.value("max_heap_size", 1048576);

    if (j.contains("operations") && j["operations"].is_array()) {
        for (const auto& op_json : j["operations"]) {
            scenario.operations.push_back(parseOperation(op_json));
        }
    }

    return scenario;
}

Operation ScenarioLoader::parseOperation(const json& op_json) {
    Operation op;
    op.op = op_json.value("op", "");
    op.id = op_json.value("id", -1);
    op.size = op_json.value("size", 0);
    op.from = op_json.value("from", -1);
    op.to = op_json.value("to", -1);
    op.description = op_json.value("description", "");

    if (op.op.empty()) {
        throw std::runtime_error("Operation type cannot be empty");
    }

    return op;
}

std::vector<Scenario> ScenarioLoader::loadAllScenarios(const std::string& scenarios_dir) {
    std::vector<Scenario> scenarios;

    try {
        for (const auto& entry : fs::directory_iterator(scenarios_dir)) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                try {
                    std::cout << "Loading " << entry.path().filename().string() << std::endl;
                    scenarios.push_back(loadScenario(entry.path().string()));
                } catch (const std::exception& e) {
                    std::cerr << "Error loading " << entry.path().filename().string() << ": " 
                             << e.what() << std::endl;
                }
            }
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Cannot read scenarios directory " + scenarios_dir + ": " + e.what());
    }

    if (scenarios.empty()) {
        std::cerr << "Warning: No scenarios found in " << scenarios_dir << std::endl;
    }

    return scenarios;
}
