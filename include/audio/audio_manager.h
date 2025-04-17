#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <string>
#include <memory>
#include <vector>
#include <memory>
#include <mutex>
#include <memory>
#include <portaudio.h>
#include <memory>

class AudioBuffer;

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    // Initialize the audio system
    bool initialize();
    
    // Shutdown and cleanup
    void shutdown();

    // Load an audio file for playback
    bool loadFile(const std::string& filePath);
    
    // Start microphone input capture
    bool startInputCapture();
    
    // Start playback of loaded audio
    bool play();
    
    // Pause playback
    bool pause();
    
    // Toggle between play and pause
    void togglePlayback();
    
    // Get the current audio samples for visualization
    std::vector<float> getAudioSamples();
    
    // Get the sample rate
    int getSampleRate() const;
    
    // Get the number of channels
    int getChannelCount() const;
    
    // Callback for PortAudio
    static int audioCallback(
        const void* inputBuffer,
        void* outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void* userData
    );

private:
    // Initialize PortAudio
    bool initializePortAudio();
    
    // Close PortAudio stream
    void closeStream();
    
    // PortAudio stream
    PaStream* m_stream;
    
    // Audio buffer
    std::shared_ptr<AudioBuffer> m_audioBuffer;
    
    // Sample rate
    int m_sampleRate;
    
    // Channel count
    int m_channelCount;
    
    // Buffer size for audio processing
    int m_bufferSize;
    
    // Flag to indicate if we're capturing from input
    bool m_isCapturingInput;
    
    // Flag to indicate if we're playing
    bool m_isPlaying;
    
    // Current audio samples for visualization
    std::vector<float> m_currentSamples;
    
    // Mutex for thread safety
    std::mutex m_mutex;
};

#endif // AUDIO_MANAGER_H