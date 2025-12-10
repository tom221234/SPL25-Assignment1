#include "MixingEngineService.h"
#include <iostream>
#include <memory>


/**
 * TODO: Implement MixingEngineService constructor
 */
MixingEngineService::MixingEngineService()
    : decks(), active_deck(0), auto_sync(false), bpm_tolerance(0) //reset all four componnets to initilize them 
    // deck[0] and deck[1] are two  arrys reprsented with raw pointers (at start they nullptr) 
{
    std::cout << "[MixingEngineService] Initialized with 2 empty decks." << std::endl;
}   

/**
 * TODO: Implement MixingEngineService destructor
 */
MixingEngineService::~MixingEngineService() {
    std::cout << "[MixingEngineService] Cleaning up decks..." << std::endl;
    for (size_t i = 0; i <= 1; i++){ // iterate on decks, freeing the memory and setting the pointers to nullptr again
        if (decks[i] != nullptr){
            delete decks[i];
            decks[i] = nullptr;
        }
        
    }
    
}


/**
 * TODO: Implement loadTrackToDeck method
 * @param track: Reference to the track to be loaded
 * @return: Index of the deck where track was loaded, or -1 on failure
 */
int MixingEngineService::loadTrackToDeck(const AudioTrack& track) {
    std::cout << "\n=== Loading Track to Deck ===" << std::endl;

    // creaating a clone of the song (if MISS)
    PointerWrapper<AudioTrack> wrappedClone = track.clone();
    
    // if track failed to clone rasing error messege
    if (!wrappedClone){
        std::cerr << "[ERROR] Track: \"" <<track.get_title() << "\" failed to clone" << std::endl;
        return -1;
    }

    size_t target_deck; // size t or int

    // finding the deck we want to load 
    if (decks[0] == nullptr && decks[1] == nullptr){
        target_deck = 0;
    } else {
        target_deck = 1 - active_deck;
    }

    std::cout << "[Deck Switch] Target deck: " << target_deck << std::endl;
    
    
    // checking if the wanted deck is occuiped
    if (decks[target_deck] != nullptr){
        delete decks[target_deck];
        decks[target_deck] = nullptr;
    }
    
    // prepring the song to be loaded 
    wrappedClone->load(); //loading song
    wrappedClone->analyze_beatgrid(); // beatgrid check

    // if the song in the active deck and the new song doesnt much by BPM - use sync_bpm
    if (decks[active_deck] != nullptr && auto_sync){
        if (can_mix_tracks(wrappedClone) == false){
            sync_bpm(wrappedClone);
        }
    } else if (decks[0] == nullptr || decks[1] == nullptr) {
        std::cout << "[Sync BPM] Cannot sync - one of the decks is empty." << std::endl;
    }
    
    // unwraping the pointer to be a raw pointer (.release)
    decks[target_deck] = wrappedClone.release(); // entering the song to the target deck

    std::cout << "[Load Complete] '" << decks[target_deck]->get_title() << "' is now loaded on deck " << target_deck << std::endl;  

    active_deck = target_deck;
    std::cout << "[Active Deck] Switched to deck " << target_deck << std::endl;
    
    return active_deck;
}

/**
 * @brief Display current deck status
 */
void MixingEngineService::displayDeckStatus() const {
    std::cout << "\n=== Deck Status ===\n";
    for (size_t i = 0; i < 2; ++i) {
        if (decks[i])
            std::cout << "Deck " << i << ": " << decks[i]->get_title() << "\n";
        else
            std::cout << "Deck " << i << ": [EMPTY]\n";
    }
    std::cout << "Active Deck: " << active_deck << "\n";
    std::cout << "===================\n";
}

/**
 * TODO: Implement can_mix_tracks method
 * 
 * Check if two tracks can be mixed based on BPM difference.
 * 
 * @param track: Track to check for mixing compatibility
 * @return: true if BPM difference <= tolerance, false otherwise
 */
bool MixingEngineService::can_mix_tracks(const PointerWrapper<AudioTrack>& track) const {
    if (decks[active_deck] == nullptr){   // checking if the deck conatins a song or empty
        return false;
    }

    if (!track){                     // if a vaild pointer 
        return false;
    }

    int current_BPM = decks[active_deck]->get_bpm(); // playing song bpm
    int new_BPM = track->get_bpm();                  // loading song bpm
    int gap_BPM = 0;
    
    if ( current_BPM >= new_BPM){
        gap_BPM = current_BPM - new_BPM;   // calculating the gap between playing song BPM and loading song BPM
    } else {
        gap_BPM = new_BPM - current_BPM;   // calculating the gap between playing song BPM and loading song BPM
    }
    
    return gap_BPM <= bpm_tolerance;  
}

/**
 * TODO: Implement sync_bpm method
 * @param track: Track to synchronize with active deck
 */
void MixingEngineService::sync_bpm(const PointerWrapper<AudioTrack>& track) const {
    if (decks[active_deck] ==nullptr) {
        return;
    }
    
    if (!track){
        return;
    }
    int old_bpm = track->get_bpm();
    int new_BPM = (track->get_bpm()+ decks[active_deck]->get_bpm()) / 2;
    track->set_bpm(new_BPM);
    std::cout << "[Sync BPM] Syncing BPM from " << old_bpm << " to " << new_BPM << std::endl;   
}