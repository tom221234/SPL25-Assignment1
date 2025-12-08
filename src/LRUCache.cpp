#include "LRUCache.h"
#include <iostream>

LRUCache::LRUCache(size_t capacity)
    : slots(capacity), max_size(capacity), access_counter(0) {}

bool LRUCache::contains(const std::string& track_id) const {
    return findSlot(track_id) != max_size;
}

AudioTrack* LRUCache::get(const std::string& track_id) {
    size_t idx = findSlot(track_id);
    if (idx == max_size) return nullptr;
    return slots[idx].access(++access_counter);
}

/**
 * TODO: Implement the put() method for LRUCache
 */
bool LRUCache::put(PointerWrapper<AudioTrack> track) {

    // Handle nullptr track by returning  false immediately
    if (!track) return false;

    // Update the access counter
    access_counter++;

    // Initialize bool values
    bool allSlotsAreFull = true;
    bool evictionHappend = false;

    // Iterate through all occupied slots
    for (size_t i = 0; i < max_size; i++){

        // If a track with the same title already exists in the cache
        if (slots[i].isOccupied() && slots[i].getTrack()->get_title() == track->get_title()){
            slots[i].access(access_counter);    // Updates the access time
            return false;                       // We did not remove the LRU
        }

        // If there is an empty slot
        if (!slots[i].isOccupied()) allSlotsAreFull = false;

    }

    // Cache full -> Evict LRU
    if (allSlotsAreFull) {
        evictionHappend = true; 
        evictLRU();
    }
    // Store the track in empty slot
    slots[findEmptySlot()].store(std::move(track), access_counter); // didnt understaand why move was needed here


    // Return true if an eviction occurred, false otherwise
    return evictionHappend;
}

bool LRUCache::evictLRU() {
    size_t lru = findLRUSlot();
    if (lru == max_size || !slots[lru].isOccupied()) return false;
    slots[lru].clear();
    return true;
}

size_t LRUCache::size() const {
    size_t count = 0;
    for (const auto& slot : slots) if (slot.isOccupied()) ++count;
    return count;
}

void LRUCache::clear() {
    for (auto& slot : slots) {
        slot.clear();
    }
}

void LRUCache::displayStatus() const {
    std::cout << "[LRUCache] Status: " << size() << "/" << max_size << " slots used\n";
    for (size_t i = 0; i < max_size; ++i) {
        if(slots[i].isOccupied()){
            std::cout << "  Slot " << i << ": " << slots[i].getTrack()->get_title()
                      << " (last access: " << slots[i].getLastAccessTime() << ")\n";
        } else {
            std::cout << "  Slot " << i << ": [EMPTY]\n";
        }
    }
}

size_t LRUCache::findSlot(const std::string& track_id) const {
    for (size_t i = 0; i < max_size; ++i) {
        if (slots[i].isOccupied() && slots[i].getTrack()->get_title() == track_id) return i;
    }
    return max_size;

}

/**
 * TODO: Implement the findLRUSlot() method for LRUCache - COMPLETED
 */
size_t LRUCache::findLRUSlot() const {

    // Initialize min, Index and boolean value.
    size_t min;
    size_t index = max_size; // If every slot is empty we will return Max_size.
    bool first_occupied_slot = true;


    // Iterate over slots
    for (size_t i = 0; i < max_size; i++){

        // If every slot is empty we will not enter
        if (slots[i].isOccupied()){

            // Initialize min based on the first slot that is not empty.
            if (first_occupied_slot){  // We reached the first occupied slot
            first_occupied_slot = false; 
            min = slots[i].getLastAccessTime();
            index = i;
            }

            // If there are multipul slots that are full, these lines will choose the minimal value slot 
            else {
                if (slots[i].getLastAccessTime() < min) {
                min = slots[i].getLastAccessTime(); // Saving the minimal value
                index = i; // Saving the index of the minimal value
                }
            }
        }
    }
    return index; // Placeholder - Return the index of the LRU
}

size_t LRUCache::findEmptySlot() const {
    for (size_t i = 0; i < max_size; ++i) {
        if (!slots[i].isOccupied()) return i;
    }
    return max_size;
}

void LRUCache::set_capacity(size_t capacity){
    if (max_size == capacity)
        return;
    //udpate max size
    max_size = capacity;
    //update the slots vector
    slots.resize(capacity);
}