
/**
 * Developed by Tanasescu Oana - Adelina.
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
 */

#ifndef __BUFFERS_H__
#define __BUFFERS_H__

class Buffers
{
private:
    // prevent copying and assignment
    Buffers(const Buffers&);
    Buffers& operator=(const Buffers&);

    // Internal Buffers !
    // image data
    unsigned char* p_imageData;

    // smaller window mean value for every pixel
    unsigned char* p_sw_meanValue;

    // smaller window standard deviation for every pixel
    float* p_sw_deviation;

    // smaller window minimum gray value for every pixel
    unsigned char* p_sw_minGray;

    // larger window mean value for every pixel
    unsigned char* p_lw_meanValue;

    // larger window standard deviation for every pixel
    float* p_lw_deviation;

    // larger window dynamic range of standard deviation
    float* p_lw_dynRangeDeviation;

    // threshold value
    float* p_threshold;

    int p_height;
    int p_width;

public:
    // Constructor
    Buffers();

    // Destructor
    ~Buffers();

    // Allocate memory
    bool AllocateMemory(const int& height, const int& width);

    // Attach image data
    void GetImageData(unsigned char** data);

    // DeAllocate memory
    void DeAllocateMemory();

    // Matrix wrappers over the above private buffers !
    unsigned char** m_imageData;
    unsigned char** m_sw_meanValue;
    float**         m_sw_deviation;
    unsigned char** m_sw_minGray;
    unsigned char** m_lw_meanValue;
    float**         m_lw_deviation;
    float**         m_lw_dynRangeDeviation;
    float**         m_threshold;
};

#endif
