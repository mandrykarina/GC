#ifndef RC_OBJECT_H
#define RC_OBJECT_H

#include <vector>
#include <algorithm>

struct RCObject {
    int id;
    int ref_count = 0;
    int size = 0;
    std::vector<int> references;
    bool marked = false;

    RCObject() : id(-1), ref_count(0), size(0), marked(false) {}
    
    explicit RCObject(int id_, int size_ = 0) 
        : id(id_), ref_count(0), size(size_), marked(false) {}

    bool has_reference_to(int target_id) const {
        return std::find(references.begin(), references.end(), target_id) != references.end();
    }

    bool add_outgoing_ref(int target_id) {
        if (!has_reference_to(target_id)) {
            references.push_back(target_id);
            return true;
        }
        return false;
    }

    bool remove_outgoing_ref(int target_id) {
        auto it = std::find(references.begin(), references.end(), target_id);
        if (it != references.end()) {
            references.erase(it);
            return true;
        }
        return false;
    }

    size_t get_outgoing_count() const {
        return references.size();
    }
};

#endif
