#ifndef EVENT_LOGGER_H
#define EVENT_LOGGER_H

#include <string>
#include <fstream>
#include <iostream>

class EventLogger {
public:
    explicit EventLogger(const std::string& log_file);
    ~EventLogger();

    bool is_open() const;
    void log_allocate(int obj_id, int size = 0);
    void log_add_ref(int from, int to, int ref_count);
    void log_remove_ref(int from, int to, int ref_count);
    void log_delete(int obj_id);
    void log_leak(int obj_id);
    void log_collection_start();
    void log_collection_end();
    void log_mark(int obj_id);
    void log_sweep(int obj_id, int size);

private:
    std::ofstream log_stream;
};

#endif
