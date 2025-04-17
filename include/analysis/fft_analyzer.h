#ifndef FFT_ANALYZER_H
#define FFT_ANALYZER_H

#include <vector>
#include <complex>
#include <fftw3.h>

class FFTAnalyzer {
public:
    FFTAnalyzer();
    ~FFTAnalyzer();

    // Initialize the FFT analyzer with a given window size
    bool initialize(int windowSize);
    
    // Process audio data and compute FFT
    void processAudioData(const std::vector<float>& audioData);
    
    // Get the processed spectrum data
    const std::vector<float>& getSpectrumData() const;
    
    // Get the window size
    int getWindowSize() const;
    
    // Get the number of frequency bins
    int getNumBins() const;

private:
    // Apply window function to reduce spectral leakage
    void applyWindow();
    
    // Compute magnitudes from complex FFT results
    void computeMagnitudes();
    
    // Window size (number of samples)
    int m_windowSize;
    
    // Number of frequency bins (windowSize/2 + 1)
    int m_numBins;
    
    // Input buffer for FFT
    double* m_fftInput;
    
    // Output buffer for FFT
    fftw_complex* m_fftOutput;
    
    // FFTW plan
    fftw_plan m_fftPlan;
    
    // Window function coefficients
    std::vector<double> m_window;
    
    // Processed spectrum magnitudes
    std::vector<float> m_magnitudes;
};

#endif // FFT_ANALYZER_H