#ifndef GC_STATS_H
#define GC_STATS_H

#include <chrono>
#include <vector>

struct GCStats {
    int collections_run = 0;
    int total_objects_collected = 0;
    long long total_memory_freed = 0;
    long long total_collection_time_us = 0;
    long long total_allocated = 0;
    long long peak_memory = 0;
    long long current_memory = 0;
    std::vector<long long> collection_times;

    double avg_collection_time() const {
        return collections_run > 0 ? (double)total_collection_time_us / collections_run : 0;
    }

    double avg_objects_per_collection() const {
        return collections_run > 0 ? (double)total_objects_collected / collections_run : 0;
    }

    double memory_recovery_percent() const {
        return total_allocated > 0 ? (100.0 * total_memory_freed / total_allocated) : 0;
    }

    double heap_usage_percent(long long max_heap) const {
        return max_heap > 0 ? (100.0 * current_memory / max_heap) : 0;
    }

    void add_collection(int objects_freed, long long memory_freed, long long time_us) {
        collections_run++;
        total_objects_collected += objects_freed;
        total_memory_freed += memory_freed;
        total_collection_time_us += time_us;
        collection_times.push_back(time_us);
    }
};

#endif
