#ifndef REFERENCE_COUNTER_H
#define REFERENCE_COUNTER_H

#include <unordered_map>
#include "rc_object.h"
#include "event_logger.h"

class ReferenceCounter
{
public:
    ReferenceCounter(std::unordered_map<int, RCObject> &heap, EventLogger &logger);

    bool add_ref(int from, int to);
    bool remove_ref(int from, int to);
    void cascade_delete(int obj_id);
    bool remove_ref_no_cascade(int from, int to);

private:
    std::unordered_map<int, RCObject> &heap;
    EventLogger &logger;

    friend class RCHeap;
};

#endif
