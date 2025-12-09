
#include "DJSession.h"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <dirent.h>

// ========== CONSTRUCTORS & RULE OF 5 ==========


DJSession::DJSession(const std::string& name, bool play_all)
    : session_name(name),
    library_service(),
    controller_service(),
    mixing_service(),
    config_manager(),
    session_config(),
    track_titles(),
    play_all(play_all),
    stats()
      {
    std::cout << "DJ Session System initialized: " << session_name << std::endl;
}


DJSession::~DJSession() {
    std::cout << "Shutting down DJ Session System: " << session_name << std::endl;
}

// ========== CORE FUNCTIONALITY ==========
bool DJSession::load_playlist(const std::string& playlist_name)  {
    std::cout << "[System] Loading playlist: " << playlist_name << "\n";
    
    // Find the playlist in the session config
    auto it = session_config.playlists.find(playlist_name);
    if (it == session_config.playlists.end()) {
        std::cerr << "[ERROR] Playlist '" << playlist_name << "' not found in configuration.\n";
        return false;
    }
    
    // Load playlist from track indices
    library_service.loadPlaylistFromIndices(playlist_name, it->second);
    
    if (library_service.getPlaylist().is_empty()) {
        return false;
    }
    
    track_titles = library_service.getTrackTitles();
    return true;
}

/**
 * TODO: Implement load_track_to_controller method
 * 
 * REQUIREMENTS:
 * 1. Track Retrieval
 *    - Find track in library using track name
 *    - Handle case when track is not found
 *    - Update error stats if track not found
 * 
 * 2. Controller Loading
 *    - Delegate loading to controller_service
 *    - Pass track by reference to controller
 * 
 * 3. Return Values
 *    1: Cache HIT
 *    0: Cache MISS (or error)
 *   -1: Cache MISS with eviction
 * 
 * @param track_name: Name of track to load
 * @return: Cache operation result code

 */
int DJSession::load_track_to_controller(const std::string& track_name) {

    // Find track in library
    AudioTrack* trackToBeLoaded = library_service.findTrack(track_name);

    // If track not found
    if (!trackToBeLoaded){
        std::cout << "[ERROR] Track: " << track_name << " not found in library" << std::endl;
        stats.errors++; // is this how to Increment stats.errors? im not sure
        return 0;       
    }

    // Log loading message
    std::cout << "[System] Loading track " << track_name << std::endl;

    // Load track to cache and save the result
    int result = controller_service.loadTrackToCache(*trackToBeLoaded);

    // increase counters based on result
    if (result == 1) stats.cache_hits++;           // Cache HIT 

    else stats.cache_misses++;                     // Else cache MISS

    if (result == -1) stats.cache_evictions++;      // Eviction Happend

    // Return the cache result
    return result;
}

/**
 * TODO: Implement load_track_to_mixer_deck method
 * 
 * @param track_title: Title of track to load to mixer
 * @return: Whether track was successfully loaded to a deck
 */
bool DJSession::load_track_to_mixer_deck(const std::string& track_title) {
    std::cout << "[System] Delegating track transfer to MixingEngineService for: " << track_title << std::endl;

    // Retrieve track from cache
    AudioTrack* track = controller_service.getTrackFromCache(track_title);

    // If track not in cache: 
    if (!track){
        std::cout << "[ERROR] Track: " << track_title << " not found in cache" << std::endl;
        stats.errors++;
        return false;
    } 

    // Initalize result
    int result = mixing_service.loadTrackToDeck(*track);

    // if loadTrackToDeck failed to load the track:
    if (result == -1){
        std::cout << "[ERROR] Failed to load track " << track_title << " to any mixer deck" << std::endl; //        // im now sure on the msg of error!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        stats.errors++;
        return false;
    }

    // Counters updates
    stats.transitions++;                         // Track loaded into one of the decks - update counter
    if (result == 0 ) stats.deck_loads_a++;      // Track Loaded into deck 0 -> update the counter a
    else if (result == 1)  stats.deck_loads_b++; // Track Loaded into deck 1 -> update the counter b

    // Track loaded -> return true
    return true;
}

/* 
 *
 * @brief Helper method to process a single playlist in the session.
 * 
 * @return: false if playlist loading failed, true otherwise.
 */
bool DJSession::process_playlist(const std::string& playlist_name){

    // Initialize boolen value representing whether the load was successful or not
    bool loadFails = !load_playlist(playlist_name);

    // If the load fails:
    if (loadFails) {
        std::cout << "[ERROR] Playlist: <" << playlist_name << "> Failed to load" << std::endl;
        stats.errors++; // not sure about that 
        return false;
    }

    // Iterate over each track in track_titles
    for (const auto& track_title : track_titles) {

        // Track Processing Phase:
        std::cout << "\n-- Processing: <" << track_title << "> --" << std::endl;
        stats.tracks_processed++;

        // Cache Loading Phase:
        load_track_to_controller(track_title);

        // Deck Loading Phase:
        bool trackFailedToLoadToDeck = !load_track_to_mixer_deck(track_title);
        if (trackFailedToLoadToDeck) continue;
    }

    // At the end of the playlist print session summary
    print_session_summary();

    // Reset all stats
    stats.tracks_processed = 0;
    stats.cache_hits       = 0;
    stats.cache_misses     = 0;
    stats.cache_evictions  = 0;
    stats.deck_loads_a     = 0;
    stats.deck_loads_b     = 0;
    stats.transitions      = 0;
    stats.errors           = 0;

    return true;
}

/**
 * @brief Main simulation loop that orchestrates the DJ performance session.
 * @note Updates session statistics (stats) throughout processing
 * @note Calls print_session_summary() to display results after playlist completion
 */
void DJSession::simulate_dj_performance() {
    std::cout << "=== DJ Controller System ===" << std::endl;
    std::cout << "Starting interactive DJ session..." << std::endl;
    // 1. Load configuration
    if (!load_configuration()) {
        std::cerr << "[ERROR] Failed to load configuration. Aborting session." << std::endl;
        return;
    }
    
    // 2. Build track library from config
    library_service.buildLibrary(session_config.library_tracks);
    
    // 3. Get available playlists from config
    if (session_config.playlists.empty()) {
        std::cerr << "[ERROR] No playlists found in configuration. Aborting session." << std::endl;
        return;
    }
    std::cout << "\nStarting DJ performance simulation..." << std::endl;
    std::cout << "BPM Tolerance: " << session_config.bpm_tolerance << " BPM" << std::endl;
    std::cout << "Auto Sync: " << (session_config.auto_sync ? "enabled" : "disabled") << std::endl;
    std::cout << "Cache Capacity: " << session_config.controller_cache_size << " slots (LRU policy)" << std::endl;
    std::cout << "\n--- Processing Tracks ---" << std::endl;

    std::cout << "TODO: Implement the DJ performance simulation workflow here." << std::endl;

    while (true) 
    {
        if (play_all) {

            // Intialize a vector that will contain the playlist names
            std::vector<std::string> playlist_names; 

            // Iterate on every playlist and save playlist name into playlists_name 
            for (const auto& pair : session_config.playlists) playlist_names.push_back(pair.first);

            // Sort playlists_names by name
            std::sort(playlist_names.begin(), playlist_names.end());

            // Load every name in playlist_names
            for (const auto& playlist_name : playlist_names) process_playlist(playlist_name);
            break;  
        }
            // If play_all is false (interactive mode):
        else {
            std::string playlist_name = display_playlist_menu_from_config();
            if (playlist_name == "") break;
            process_playlist(playlist_name);
            }
        }

    // 5. Statistics Tracking Requirements:
        // • Cache misses: Count both with and without eviction (a MISS with eviction increments both cache_misses and cache_evictions)
        // • Transitions: Increment whenever a track successfully loads to either deck
        // • Errors: Track failures at any stage (library lookup, cache access, deck loading, clone failures)
        // • Tracks processed: Total tracks attempted (regardless of success/failure)

    
    // 6. Error Handling Requirements:
        // • Abort early if configuration parsing fails or no playlists are found
        // • Log all errors with format: [ERROR] <Context>: "<identifier>" <description>
        // • Skip individual track failures gracefully without stopping the session
        // • Continue processing remaining tracks even after errors
        // • Ensure all errors are counted in stats.errors







        
        
        
        // (c) For Each Selected Playlist:
            // i. Call load_playlist(playlist_name)
            // ii. If load fails, log error and continue to next playlist (or prompt again in interactive mode)

            // iii. Track Processing Loop - for each track in track_titles:
                // • Log: \n–- Processing: <track_title> –-
                // • Increment stats.tracks_processed
                // • Cache Loading Phase:
                    // - load the track to controller using load_track_to_controller(track_title)
                    // – update the cache statistics based on the return value
                // • Deck Loading Phase:
                    // – Call load_track_to_mixer_deck(track_title) and update the deck and transition statistics based on the return value
                    // – If load fails, continue to next track (error already logged and counted)

            // iv. After all tracks processed, call print_session_summary()
            // v. Reset statistics for next playlist: set all stats members to 0

}


/* 
 * Helper method to load session configuration from file
 * 
 * @return: true if configuration loaded successfully; false on error
 */
bool DJSession::load_configuration() {
    const std::string config_path = "bin/dj_config.txt";
    
    std::cout << "Loading configuration from: " << config_path << std::endl;
    
    if (!SessionFileParser::parse_config_file(config_path, session_config)) {
        std::cerr << "[ERROR] Failed to parse configuration file: " << config_path << std::endl;
        return false;
    }
    
    std::cout << "Configuration loaded successfully." << std::endl;
    std::cout << "BPM Tolerance: " << session_config.bpm_tolerance << " BPM" << std::endl;
    std::cout << "Auto Sync: " << (session_config.auto_sync ? "enabled" : "disabled") << std::endl;
    std::cout << "Cache Size: " << session_config.controller_cache_size << " slots" << std::endl;
    mixing_service.set_auto_sync(session_config.auto_sync);
    mixing_service.set_bpm_tolerance(session_config.bpm_tolerance);

    //update cache size in LRUCache
    controller_service.set_cache_size(session_config.controller_cache_size);
    return true;
}

std::string DJSession::display_playlist_menu_from_config() {
    if (session_config.playlists.empty()) {
        return "";
    }
    
    std::cout << "\n=== Available Playlists ===" << std::endl;
    
    // Build sorted list of playlist names
    std::vector<std::string> playlist_names;
    for (const auto& pair : session_config.playlists) {
        playlist_names.push_back(pair.first);
    }
    std::sort(playlist_names.begin(), playlist_names.end());
    
    // Display numbered list
    for (size_t i = 0; i < playlist_names.size(); ++i) {
        std::cout << (i + 1) << ". " << playlist_names[i] << std::endl;
    }
    std::cout << "0. Cancel" << std::endl;
    
    // Prompt for user selection with validation
    int selection = -1;
    while (true) {
        std::cout << "\nSelect a playlist (1-" << playlist_names.size() << ", 0 to cancel): ";
        std::string input;
        
        if (!std::getline(std::cin, input)) {
            std::cout << "\n[ERROR] Input error. Cancelling session." << std::endl;
            return "";
        }
        
        std::stringstream ss(input);
        if (ss >> selection && ss.eof()) {
            if (selection == 0) {
                return "";
            } else if (selection >= 1 && selection <= static_cast<int>(playlist_names.size())) {
                std::string selected_name = playlist_names[selection - 1];
                std::cout << "Selected: " << selected_name << std::endl;
                return selected_name;
            }
        }
        
        std::cout << "Invalid selection. Please enter a number between 1 and " 
                  << playlist_names.size() << ", or 0 to cancel." << std::endl;
    }
}

void DJSession::print_session_summary() const {
    std::cout << "\n=== DJ Session Summary ===" << std::endl;
    std::cout << "Session: " << session_name << std::endl;
    std::cout << "Tracks processed: " << stats.tracks_processed << std::endl;
    std::cout << "Cache hits: " << stats.cache_hits << std::endl;
    std::cout << "Cache misses: " << stats.cache_misses << std::endl;
    std::cout << "Cache evictions: " << stats.cache_evictions << std::endl;
    std::cout << "Deck A loads: " << stats.deck_loads_a << std::endl;
    std::cout << "Deck B loads: " << stats.deck_loads_b << std::endl;
    std::cout << "Transitions: " << stats.transitions << std::endl;
    std::cout << "Errors: " << stats.errors << std::endl;
    std::cout << "=== Session Complete ===" << std::endl;
}