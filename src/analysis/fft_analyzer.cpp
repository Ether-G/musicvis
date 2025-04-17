#include <cmath>
#include <iostream>
#include <algorithm>
#include "analysis/fft_analyzer.h"

FFTAnalyzer::FFTAnalyzer()
    : m_windowSize(0)
    , m_numBins(0)
    , m_fftInput(nullptr)
    , m_fftOutput(nullptr)
    , m_fftPlan(nullptr)
{
}

FFTAnalyzer::~FFTAnalyzer() {
    if (m_fftPlan) {
        fftw_destroy_plan(m_fftPlan);
        m_fftPlan = nullptr;
    }
    
    if (m_fftInput) {
        fftw_free(m_fftInput);
        m_fftInput = nullptr;
    }
    
    if (m_fftOutput) {
        fftw_free(m_fftOutput);
        m_fftOutput = nullptr;
    }
}

bool FFTAnalyzer::initialize(int windowSize) {
    // Clean up if already initialized
    if (m_fftPlan) {
        fftw_destroy_plan(m_fftPlan);
        m_fftPlan = nullptr;
    }
    
    if (m_fftInput) {
        fftw_free(m_fftInput);
        m_fftInput = nullptr;
    }
    
    if (m_fftOutput) {
        fftw_free(m_fftOutput);
        m_fftOutput = nullptr;
    }
    
    // Set window size and number of bins
    m_windowSize = windowSize;
    m_numBins = windowSize / 2 + 1;
    
    // Allocate memory for FFT input and output
    m_fftInput = static_cast<double*>(fftw_malloc(sizeof(double) * m_windowSize));
    m_fftOutput = static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * m_numBins));
    
    if (!m_fftInput || !m_fftOutput) {
        std::cerr << "Failed to allocate memory for FFT" << std::endl;
        return false;
    }
    
    // Create FFTW plan
    m_fftPlan = fftw_plan_dft_r2c_1d(m_windowSize, m_fftInput, m_fftOutput, FFTW_MEASURE);
    
    if (!m_fftPlan) {
        std::cerr << "Failed to create FFTW plan" << std::endl;
        return false;
    }
    
    // Create Hann window function
    m_window.resize(m_windowSize);
    for (int i = 0; i < m_windowSize; ++i) {
        m_window[i] = 0.5 * (1.0 - cos(2.0 * M_PI * i / (m_windowSize - 1)));
    }
    
    // Initialize magnitude vector
    m_magnitudes.resize(m_numBins, 0.0f);
    
    std::cout << "FFT Analyzer initialized with window size: " << m_windowSize 
              << ", bins: " << m_numBins << std::endl;
    
    return true;
}

void FFTAnalyzer::processAudioData(const std::vector<float>& audioData) {
    if (!m_fftPlan || audioData.empty()) {
        return;
    }
    
    // Copy audio data to FFT input buffer
    // For stereo audio, we'll average the channels
    int channels = audioData.size() / (m_windowSize / 2);
    if (channels < 1) channels = 1;
    
    for (int i = 0; i < m_windowSize; ++i) {
        if (i < audioData.size() / channels) {
            double sum = 0.0;
            for (int c = 0; c < channels; ++c) {
                int index = i * channels + c;
                if (index < audioData.size()) {
                    sum += audioData[index];
                }
            }
            m_fftInput[i] = sum / channels;
        } else {
            m_fftInput[i] = 0.0;
        }
    }
    
    // Apply window function
    applyWindow();
    
    // Execute FFT
    fftw_execute(m_fftPlan);
    
    // Compute magnitude spectrum
    computeMagnitudes();
}

void FFTAnalyzer::applyWindow() {
    for (int i = 0; i < m_windowSize; ++i) {
        m_fftInput[i] *= m_window[i];
    }
}

void FFTAnalyzer::computeMagnitudes() {
    const double normalizationFactor = 2.0 / m_windowSize;
    
    // DC component (bin 0)
    double real = m_fftOutput[0][0];
    double imag = m_fftOutput[0][1];
    m_magnitudes[0] = normalizationFactor * sqrt(real * real + imag * imag);
    
    // Other bins
    for (int i = 1; i < m_numBins; ++i) {
        real = m_fftOutput[i][0];
        imag = m_fftOutput[i][1];
        
        // Compute magnitude
        double magnitude = normalizationFactor * sqrt(real * real + imag * imag);
        
        // Apply logarithmic scaling for better visualization
        magnitude = 20.0 * log10(magnitude + 1e-6);
        
        // Normalize to 0-1 range (assuming signals are between -96dB and 0dB)
        magnitude = std::max(0.0, magnitude + 96.0) / 96.0;
        
        // Apply some smoothing with previous value (simple low-pass filter)
        m_magnitudes[i] = 0.2f * m_magnitudes[i] + 0.8f * magnitude;
    }
}

const std::vector<float>& FFTAnalyzer::getSpectrumData() const {
    return m_magnitudes;
}

int FFTAnalyzer::getWindowSize() const {
    return m_windowSize;
}

int FFTAnalyzer::getNumBins() const {
    return m_numBins;
}