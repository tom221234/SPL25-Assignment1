#include "DJLibraryService.h"
#include "SessionFileParser.h"
#include "MP3Track.h"
#include "WAVTrack.h"
#include <iostream>
#include <memory>
#include <filesystem>
#include <algorithm>


DJLibraryService::DJLibraryService(const Playlist& playlist) 
    : playlist(playlist), library() {}
/**
 * @brief Load a playlist from track indices referencing the library
 * @param library_tracks Vector of track info from config
 */

//Destructor
DJLibraryService::~DJLibraryService (){
    // Reset library
    for (AudioTrack* curr_track: library) {
        delete curr_track;
    }

    library.clear();
}





void DJLibraryService::buildLibrary(const std::vector<SessionConfig::TrackInfo>& library_tracks) {
    for ( const auto& track_info : library_tracks){

        AudioTrack* new_track = nullptr;

        if (track_info.type == "MP3"){
            new_track = new MP3Track(
                track_info.title,
                track_info.artists,
                track_info.duration_seconds,
                track_info.bpm,
                track_info.extra_param1,
                (bool) track_info.extra_param2
            );
        }

        else if ( track_info.type == "WAV"){
            new_track = new WAVTrack(
                track_info.title,
                track_info.artists,
                track_info.duration_seconds,
                track_info.bpm,
                track_info.extra_param1,
                track_info.extra_param2
            );
        }

        if (new_track != nullptr){
            library.push_back(new_track);
        }
    }
    std::cout << "[INFO] Track library built: " << library_tracks.size() << " tracks loaded" << std::endl;
}

/**
 * @brief Display the current state of the DJ library playlist
 * 
 */
void DJLibraryService::displayLibrary() const {
    std::cout << "=== DJ Library Playlist: " 
              << playlist.get_name() << " ===" << std::endl;

    if (playlist.is_empty()) {
        std::cout << "[INFO] Playlist is empty.\n";
        return;
    }

    // Let Playlist handle printing all track info
    playlist.display();

    std::cout << "Total duration: " << playlist.get_total_duration() << " seconds" << std::endl;
}

/**
 * @brief Get a reference to the current playlist
 * 
 * @return Playlist& 
 */
Playlist& DJLibraryService::getPlaylist() {
    // Your implementation here
    return playlist;
}

/**
 * TODO: Implement findTrack method
 * 
 * HINT: Leverage Playlist's find_track method
 */
AudioTrack* DJLibraryService::findTrack(const std::string& track_title) {
    // return pointer to the track if found, find_track() 
    // if not found find_track will return nullptr
    return playlist.find_track(track_title);
    
}

void DJLibraryService::loadPlaylistFromIndices(const std::string& playlist_name,const std::vector<int>& track_indices) {

    // create new Playlist with the given name
    std::cout << "[INFO] Loading playlist: " << playlist_name << std::endl;
    this->playlist = Playlist(playlist_name);
    
    //  iterate over track_indices
    for (int index: track_indices) {
        if (index < 1 or index > static_cast<int>(library.size())){
            std::cout << "[WARNING] Invalid track index:" << index << std ::endl;
            continue;
        }
        // get track from library using 0-based indexing
        AudioTrack* original_track = library[index -1];
        // clone the track
        PointerWrapper<AudioTrack> clone_track = original_track->clone();

        // if clone was faild raise message
        if (!clone_track){
            std::cout << "[ERROR] Failed to clone track: " << original_track->get_title() << std::endl;
            continue;
        }
        
        clone_track->load();    // loading clone track
        clone_track->analyze_beatgrid(); // beatgrid check


        // adding the unwrapped track to the playlist
        playlist.add_track(clone_track.release());

    }
        // printing the loading message 
    std::cout << "[INFO] Playlist loaded: " << playlist_name << " (" << playlist.get_track_count() << " tracks)" << std::endl;



}


/**
 * TODO: Implement getTrackTitles method
 * @return Vector of track titles in the playlist
 */
std::vector<std::string> DJLibraryService::getTrackTitles() const {
    // new vector for track titles
    std::vector<std::string> track_titles;
    // itrate on playlist and 
    for ( const auto& track_ptr : playlist.getTracks() ){
        if (track_ptr != nullptr){   // for evrey track cheking if pointer or null 
            track_titles.push_back(track_ptr->get_title());  // adding the track title to the new titles vector
        }
    }

    // Reverse to get insertion order (add_track adds to front of linked list)
    std::reverse(track_titles.begin(), track_titles.end());

    return track_titles;  // return vector of track titles as asked 
}