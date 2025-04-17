#include <iostream>
#include <stdexcept>
#include <cstring>
#include "audio/audio_manager.h"
#include "audio/audio_buffer.h"

AudioManager::AudioManager()
    : m_stream(nullptr)
    , m_audioBuffer(std::make_shared<AudioBuffer>())
    , m_sampleRate(44100)
    , m_channelCount(2)
    , m_bufferSize(1024)
    , m_isCapturingInput(false)
    , m_isPlaying(false)
{
}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::initialize() {
    return initializePortAudio();
}

void AudioManager::shutdown() {
    closeStream();
    
    // Terminate PortAudio
    Pa_Terminate();
    
    m_isPlaying = false;
    m_isCapturingInput = false;
}

bool AudioManager::initializePortAudio() {
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization error: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    // List available audio devices for debugging
    int numDevices = Pa_GetDeviceCount();
    std::cout << "Number of audio devices: " << numDevices << std::endl;
    
    for (int i = 0; i < numDevices; i++) {
        const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
        if (deviceInfo) {
            std::cout << "Device " << i << ": " << deviceInfo->name 
                    << " (in: " << deviceInfo->maxInputChannels 
                    << ", out: " << deviceInfo->maxOutputChannels << ")" << std::endl;
        }
    }
    
    // Print default devices
    int defaultInput = Pa_GetDefaultInputDevice();
    int defaultOutput = Pa_GetDefaultOutputDevice();
    
    std::cout << "Default input device: " << defaultInput;
    if (defaultInput != paNoDevice) {
        std::cout << " (" << Pa_GetDeviceInfo(defaultInput)->name << ")";
    }
    std::cout << std::endl;
    
    std::cout << "Default output device: " << defaultOutput;
    if (defaultOutput != paNoDevice) {
        std::cout << " (" << Pa_GetDeviceInfo(defaultOutput)->name << ")";
    }
    std::cout << std::endl;
    
    std::cout << "PortAudio initialized successfully" << std::endl;
    return true;
}

void AudioManager::closeStream() {
    if (m_stream != nullptr) {
        PaError err = Pa_CloseStream(m_stream);
        if (err != paNoError) {
            std::cerr << "Error closing PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        }
        m_stream = nullptr;
    }
}

bool AudioManager::loadFile(const std::string& filePath) {
    // Close existing stream
    closeStream();
    
    // Reset flags
    m_isCapturingInput = false;
    m_isPlaying = false;
    
    // Load audio file
    if (!m_audioBuffer->loadFromFile(filePath)) {
        std::cerr << "Failed to load audio file: " << filePath << std::endl;
        return false;
    }
    
    m_sampleRate = m_audioBuffer->getSampleRate();
    m_channelCount = m_audioBuffer->getChannelCount();
    
    // Create output stream
    PaStreamParameters outputParams;
    memset(&outputParams, 0, sizeof(outputParams));
    
    // Get default output device
    outputParams.device = Pa_GetDefaultOutputDevice();
    if (outputParams.device == paNoDevice) {
        std::cerr << "No default output device found" << std::endl;
        return false;
    }
    
    // Print device info for debugging
    const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(outputParams.device);
    std::cout << "Using audio device: " << deviceInfo->name << std::endl;
    
    outputParams.channelCount = m_channelCount;
    outputParams.sampleFormat = paFloat32;
    outputParams.suggestedLatency = deviceInfo->defaultLowOutputLatency;
    outputParams.hostApiSpecificStreamInfo = nullptr;
    
    PaError err = Pa_OpenStream(
        &m_stream,
        nullptr,                    // No input
        &outputParams,              // Output parameters
        m_sampleRate,               // Sample rate
        m_bufferSize,               // Frames per buffer
        paNoFlag,                   // Stream flags
        &AudioManager::audioCallback,  // Callback function
        this                        // User data
    );
    
    if (err != paNoError) {
        std::cerr << "Error opening PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    std::cout << "Audio file loaded: " << filePath << std::endl;
    std::cout << "Sample rate: " << m_sampleRate << ", Channels: " << m_channelCount << std::endl;
    
    return true;
}

bool AudioManager::startInputCapture() {
    // Close existing stream
    closeStream();
    
    // Reset flags
    m_isCapturingInput = true;
    m_isPlaying = false;
    
    // Try to find the PulseAudio API
    PaHostApiIndex pulseIndex = -1;
    for (int i = 0; i < Pa_GetHostApiCount(); i++) {
        const PaHostApiInfo* apiInfo = Pa_GetHostApiInfo(i);
        if (apiInfo->type == paJACK || 
            (apiInfo->name && strstr(apiInfo->name, "PulseAudio"))) {
            pulseIndex = i;
            std::cout << "Found PulseAudio or JACK API: " << apiInfo->name << std::endl;
            break;
        }
    }
    
    // Create input stream
    PaStreamParameters inputParams;
    memset(&inputParams, 0, sizeof(inputParams));
    
    // Use PulseAudio device if available, otherwise fall back to default
    if (pulseIndex >= 0) {
        const PaHostApiInfo* apiInfo = Pa_GetHostApiInfo(pulseIndex);
        inputParams.device = apiInfo->defaultInputDevice;
        std::cout << "Using " << apiInfo->name << " input device" << std::endl;
    } else {
        inputParams.device = Pa_GetDefaultInputDevice();
        std::cout << "Using default input device (PulseAudio not found)" << std::endl;
    }
    
    if (inputParams.device == paNoDevice) {
        std::cerr << "No input device found" << std::endl;
        return false;
    }
    
    m_channelCount = 2; // Stereo input
    inputParams.channelCount = m_channelCount;
    inputParams.sampleFormat = paFloat32;
    inputParams.suggestedLatency = Pa_GetDeviceInfo(inputParams.device)->defaultLowInputLatency;
    inputParams.hostApiSpecificStreamInfo = nullptr;
    
    PaError err = Pa_OpenStream(
        &m_stream,
        &inputParams,               // Input parameters
        nullptr,                    // No output
        m_sampleRate,               // Sample rate
        m_bufferSize,               // Frames per buffer
        paNoFlag,                   // Stream flags
        &AudioManager::audioCallback,  // Callback function
        this                        // User data
    );
    
    if (err != paNoError) {
        std::cerr << "Error opening PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    // Start the stream
    err = Pa_StartStream(m_stream);
    if (err != paNoError) {
        std::cerr << "Error starting PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        closeStream();
        return false;
    }
    
    m_isPlaying = true;
    std::cout << "Audio input capture started" << std::endl;
    
    return true;
}

bool AudioManager::play() {
    if (m_isCapturingInput) {
        return true; // Already capturing, nothing to do
    }
    
    if (m_isPlaying) {
        return true; // Already playing, nothing to do
    }
    
    if (m_stream == nullptr) {
        std::cerr << "Cannot play: No stream available" << std::endl;
        return false;
    }
    
    PaError err = Pa_StartStream(m_stream);
    if (err != paNoError) {
        std::cerr << "Error starting PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    m_isPlaying = true;
    std::cout << "Audio playback started" << std::endl;
    
    return true;
}

bool AudioManager::pause() {
    if (!m_isPlaying) {
        return true; // Already paused, nothing to do
    }
    
    if (m_stream == nullptr) {
        return false;
    }
    
    PaError err = Pa_StopStream(m_stream);
    if (err != paNoError) {
        std::cerr << "Error stopping PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }
    
    m_isPlaying = false;
    std::cout << "Audio playback paused" << std::endl;
    
    return true;
}

void AudioManager::togglePlayback() {
    if (m_isPlaying) {
        pause();
    } else {
        play();
    }
}

std::vector<float> AudioManager::getAudioSamples() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentSamples;
}

int AudioManager::getSampleRate() const {
    return m_sampleRate;
}

int AudioManager::getChannelCount() const {
    return m_channelCount;
}

int AudioManager::audioCallback(
    const void* inputBuffer,
    void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData
) {
    AudioManager* audioManager = static_cast<AudioManager*>(userData);
    
    if (audioManager->m_isCapturingInput) {
        // Input capture mode
        if (inputBuffer) {
            const float* in = static_cast<const float*>(inputBuffer);
            
            std::lock_guard<std::mutex> lock(audioManager->m_mutex);
            audioManager->m_currentSamples.resize(framesPerBuffer * audioManager->m_channelCount);
            
            // Copy input data
            memcpy(
                audioManager->m_currentSamples.data(),
                in,
                framesPerBuffer * audioManager->m_channelCount * sizeof(float)
            );
        }
    } else {
        // Playback mode
        if (outputBuffer) {
            float* out = static_cast<float*>(outputBuffer);
            
            // Get samples from audio buffer
            std::vector<float> samples = audioManager->m_audioBuffer->getSamples(
                framesPerBuffer * audioManager->m_channelCount
            );
            
            // Copy to output buffer
            memcpy(
                out,
                samples.data(),
                samples.size() * sizeof(float)
            );
            
            {
                std::lock_guard<std::mutex> lock(audioManager->m_mutex);
                audioManager->m_currentSamples = samples;
            }
            
            // Check for end of file
            if (samples.size() < framesPerBuffer * audioManager->m_channelCount) {
                return paComplete;
            }
        }
    }
    
    return paContinue;
}