#include "AudioTrack.h"
#include <iostream>
#include <cstring>
#include <random>

AudioTrack::AudioTrack(const std::string& title, const std::vector<std::string>& artists, 
                      int duration, int bpm, size_t waveform_samples)
    : title(title), artists(artists), duration_seconds(duration), bpm(bpm), 
      waveform_data(nullptr), waveform_size(waveform_samples) {

    // Allocate memory for waveform analysis
    waveform_data = new double[waveform_size];

    // Generate some dummy waveform data for testing
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(-1.0, 1.0);

    for (size_t i = 0; i < waveform_size; ++i) {
        waveform_data[i] = dis(gen);
    }
    #ifdef DEBUG
    std::cout << "AudioTrack created: " << title << " by " << std::endl;
    for (const auto& artist : artists) {
        std::cout << artist << " ";
    }
    std::cout << std::endl;
    #endif
}

// ========== TODO: STUDENTS IMPLEMENT RULE OF 5 ==========

// Destructor
AudioTrack::~AudioTrack() {
    #ifdef DEBUG
    std::cout << "AudioTrack destructor called for: " << title << std::endl;
    #endif

    // Reset the WaveForm Data
    delete[] waveform_data;
    waveform_data = nullptr;
    waveform_size = 0;
}

// Copy Constructor
AudioTrack::AudioTrack(const AudioTrack& other)
{
    #ifdef DEBUG
    std::cout << "AudioTrack copy constructor called for: " << other.title << std::endl;
    #endif

    // Copy all of the data
    this->title = other.title;
    this->artists = other.artists;
    this->duration_seconds = other.duration_seconds;
    this->bpm = other.bpm;
    this->waveform_size = other.waveform_size;

    // Deep copy waveform_data
    this->waveform_data = new double[this->waveform_size];
    for (size_t i = 0; i < this->waveform_size; i++)  {
        this->waveform_data[i] = other.waveform_data[i];
    }
}

// Copy Assigment Operator
AudioTrack& AudioTrack::operator=(const AudioTrack& other) {    
    #ifdef DEBUG
    std::cout << "AudioTrack copy assignment called for: " << other.title << std::endl;
    #endif

    // Self Assignment Guard
    if (this == &other){
        return *this;
    }

    // Delete the old data
    delete[] waveform_data;
    waveform_data = nullptr;

    // Copy all of the data
    this->title = other.title;
    this->artists = other.artists;
    this->duration_seconds = other.duration_seconds;
    this->bpm = other.bpm;
    this->waveform_size = other.waveform_size;

    // Deep copy waveform_data
    this->waveform_data = new double[this->waveform_size];
    for (size_t i = 0; i < this->waveform_size; i++)  {
        this->waveform_data[i] = other.waveform_data[i];
    }

    // Return this
    return *this;
}

// Move Constructor
AudioTrack::AudioTrack(AudioTrack&& other) noexcept {
    #ifdef DEBUG
    std::cout << "AudioTrack move constructor called for: " << other.title << std::endl;
    #endif

    // Move all of the data
    this->title = std::move(other.title);
    this->artists = std::move(other.artists);
    this->duration_seconds = std::move(other.duration_seconds);
    this->bpm = std::move(other.bpm);
    this->waveform_size = std::move(other.waveform_size);

    // Steal the pointer from Other
    this->waveform_data = other.waveform_data;

    // Delete Data from Other
    other.waveform_data = nullptr;
    other.waveform_size = 0;
}

// Move Assigment Operator
AudioTrack& AudioTrack::operator=(AudioTrack&& other) noexcept {
    #ifdef DEBUG
    std::cout << "AudioTrack move assignment called for: " << other.title << std::endl;
    #endif

    // Self Assignment Guard
    if (this == &other){
        return *this;
    }
    // Reset the WaveForm Data
    delete[] waveform_data;
    waveform_data = nullptr;
    waveform_size = 0;

    // Move all of the data
    this->title = std::move(other.title);
    this->artists = std::move(other.artists);
    this->duration_seconds = std::move(other.duration_seconds);
    this->bpm = std::move(other.bpm);
    this->waveform_size = std::move(other.waveform_size);

    // Steal the pointer from Other
    this->waveform_data = other.waveform_data;

    // Delete Data from Other
    other.waveform_data = nullptr;
    other.waveform_size = 0;
    return *this;
}

void AudioTrack::get_waveform_copy(double* buffer, size_t buffer_size) const {
    if (buffer && waveform_data && buffer_size <= waveform_size) {
        std::memcpy(buffer, waveform_data, buffer_size * sizeof(double));
    }
}