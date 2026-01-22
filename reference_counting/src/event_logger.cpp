#include "event_logger.h"
#include <iomanip>

EventLogger::EventLogger(const std::string& log_file) {
    log_stream.open(log_file, std::ios::app);
}

EventLogger::~EventLogger() {
    if (log_stream.is_open()) {
        log_stream.close();
    }
}

bool EventLogger::is_open() const {
    return log_stream.is_open();
}

void EventLogger::log_allocate(int obj_id, int size) {
    if (log_stream.is_open()) {
        log_stream << "[ALLOCATE] obj_" << obj_id << " (size=" << size << ")\n";
        log_stream.flush();
    }
}

void EventLogger::log_add_ref(int from, int to, int ref_count) {
    if (log_stream.is_open()) {
        log_stream << "[ADD_REF] obj_" << from << " -> obj_" << to << " (rc=" << ref_count << ")\n";
        log_stream.flush();
    }
}

void EventLogger::log_remove_ref(int from, int to, int ref_count) {
    if (log_stream.is_open()) {
        log_stream << "[REMOVE_REF] obj_" << from << " -> obj_" << to << " (rc=" << ref_count << ")\n";
        log_stream.flush();
    }
}

void EventLogger::log_delete(int obj_id) {
    if (log_stream.is_open()) {
        log_stream << "[DELETE] obj_" << obj_id << "\n";
        log_stream.flush();
    }
}

void EventLogger::log_leak(int obj_id) {
    if (log_stream.is_open()) {
        log_stream << "[LEAK] obj_" << obj_id << "\n";
        log_stream.flush();
    }
}

void EventLogger::log_collection_start() {
    if (log_stream.is_open()) {
        log_stream << "[COLLECTION_START]\n";
        log_stream.flush();
    }
}

void EventLogger::log_collection_end() {
    if (log_stream.is_open()) {
        log_stream << "[COLLECTION_END]\n";
        log_stream.flush();
    }
}

void EventLogger::log_mark(int obj_id) {
    if (log_stream.is_open()) {
        log_stream << "[MARK] obj_" << obj_id << "\n";
        log_stream.flush();
    }
}

void EventLogger::log_sweep(int obj_id, int size) {
    if (log_stream.is_open()) {
        log_stream << "[SWEEP] obj_" << obj_id << " (freed " << size << " bytes)\n";
        log_stream.flush();
    }
}
