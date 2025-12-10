#include "DJControllerService.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>

DJControllerService::DJControllerService(size_t cache_size)
    : cache(cache_size) {}
/**
 * TODO: Implement loadTrackToCache method
 */
int DJControllerService::loadTrackToCache(AudioTrack& track) {
    // check if track is in cache already (HIT)

    if (cache.contains(track.get_title())){   
        cache.get(track.get_title());     // if true reset the MRU 
        return 1;
    }
    // creaating a clone of the song (if MISS)
    PointerWrapper<AudioTrack> wrappedClone = track.clone();
    // unwraping the pointer to be a raw pointer (.release)
    AudioTrack* rawClone = wrappedClone.release();
    // if the unwraped pointer is null rasie error
    if(rawClone == nullptr) {
        std::cerr << "[ERROR] Track: \"" << track.get_title() << "\" failed to clone" << std::endl;
        return 0;
    }
    rawClone->load(); //loading song
    rawClone->analyze_beatgrid(); // beatgrid check

    // the result from cache.put() - True of False
    bool result = cache.put(PointerWrapper<AudioTrack>(rawClone));
    
    if (result) { 
        return -1; // if result == true it means that the cache was full removal from the cache was needed - return -1
    }
    return 0; // if no removal was needed, there was a free slot in cache - return 0
}

void DJControllerService::set_cache_size(size_t new_size) {
    cache.set_capacity(new_size);
}
//implemented
void DJControllerService::displayCacheStatus() const {
    std::cout << "\n=== Cache Status ===" << std::endl;
    cache.displayStatus();
    std::cout << "====================" << std::endl;
}

/**
 * TODO: Implement getTrackFromCache method
 */
AudioTrack* DJControllerService::getTrackFromCache(const std::string& track_title) {
    // if song name found, return it. otherwise get() will return "nullptr"
    return cache.get(track_title);
}