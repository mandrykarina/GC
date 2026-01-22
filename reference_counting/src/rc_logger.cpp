#include "rc_logger.h"

RCLogger::RCLogger(const std::string &log_file_path)
    : current_step(0)
{
    log_file.open(log_file_path, std::ios::app);
    if (log_file.is_open())
    {
        log_file << "\n=== Reference Counting GC Session Started ===" << std::endl;
    }
}

RCLogger::~RCLogger()
{
    if (log_file.is_open())
    {
        log_file << "=== Reference Counting GC Session Ended ===" << std::endl;
        log_file.close();
    }
}

void RCLogger::log_operation(const std::string &operation)
{
    if (log_file.is_open())
    {
        log_file << "[Step " << current_step << "] " << operation << std::endl;
        log_file.flush();
    }
    std::cout << "[Step " << current_step << "] " << operation << std::endl;
    current_step++;
}

void RCLogger::log_init(size_t max_heap)
{
    std::ostringstream oss;
    oss << "GC initialized with max_heap=" << max_heap;
    log_operation(oss.str());
}

void RCLogger::log_allocate(int obj_id, size_t size)
{
    std::ostringstream oss;
    oss << "ALLOCATE: obj_" << obj_id << " (size=" << size << " bytes)";
    log_operation(oss.str());
}

void RCLogger::log_make_root(int obj_id)
{
    std::ostringstream oss;
    oss << "MAKE_ROOT: obj_" << obj_id << " is now a root object";
    log_operation(oss.str());
}

void RCLogger::log_remove_root(int obj_id)
{
    std::ostringstream oss;
    oss << "REMOVE_ROOT: obj_" << obj_id << " is no longer a root";
    log_operation(oss.str());
}

void RCLogger::log_add_ref(int from, int to)
{
    std::ostringstream oss;
    oss << "ADD_REF: obj_" << from << " -> obj_" << to;
    log_operation(oss.str());
}

void RCLogger::log_remove_ref(int from, int to)
{
    std::ostringstream oss;
    oss << "REMOVE_REF: obj_" << from << " -> obj_" << to;
    log_operation(oss.str());
}

void RCLogger::log_cascade_delete(int obj_id, size_t size)
{
    std::ostringstream oss;
    oss << "Deleted obj_" << obj_id << " (" << size << " bytes)";
    log_operation(oss.str());
}

void RCLogger::log_leak(int obj_id)
{
    std::ostringstream oss;
    oss << "LEAK: obj_" << obj_id << " (memory leak detected in cycle)";
    log_operation(oss.str());
}
