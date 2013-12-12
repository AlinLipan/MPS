
/**
 * Developed by Tanasescu Oana - Adelina.
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
 */

#include "stdafx.h"
#include "Buffers.h"


Buffers::Buffers():
    p_imageData(NULL),
    p_sw_meanValue(NULL),
    p_sw_deviation(NULL),
    p_sw_minGray(NULL),
    p_lw_meanValue(NULL),
    p_lw_deviation(NULL),
    p_lw_dynRangeDeviation(NULL),
    p_threshold(NULL),
    m_imageData(NULL),
    m_sw_meanValue(NULL),
    m_sw_deviation(NULL),
    m_sw_minGray(NULL),
    m_lw_meanValue(NULL),
    m_lw_deviation(NULL),
    m_lw_dynRangeDeviation(NULL),
    m_threshold(NULL)
{
    // Nothing more to do here...
}


Buffers::~Buffers()
{
    DeAllocateMemory();
}


bool Buffers::AllocateMemory(const int& height, const int& width)
{
    DeAllocateMemory();

    p_height = height;
    p_width  = width;

    int size = p_height * p_width;

    // allocate buffers
    p_imageData            = new unsigned char[size];
    p_sw_meanValue         = new unsigned char[size];
    p_sw_deviation         = new float[size];
    p_sw_minGray           = new unsigned char[size];
    p_lw_meanValue         = new unsigned char[size];
    p_lw_deviation         = new float[size];
    p_lw_dynRangeDeviation = new float[size];
    p_threshold            = new float[size];

    // check buffers status
    if (p_imageData    == NULL ||
        p_sw_meanValue == NULL ||
        p_sw_deviation == NULL ||
        p_sw_minGray   == NULL ||
        p_lw_meanValue == NULL ||
        p_lw_deviation == NULL ||
        p_lw_dynRangeDeviation == NULL ||
        p_threshold    == NULL)
    {
        return false;
    }

    // allocate wrappers
    m_imageData            = new unsigned char*[p_height];
    m_sw_meanValue         = new unsigned char*[p_height];
    m_sw_deviation         = new float*[p_height];
    m_sw_minGray           = new unsigned char*[p_height];
    m_lw_meanValue         = new unsigned char*[p_height];
    m_lw_deviation         = new float*[p_height];
    m_lw_dynRangeDeviation = new float*[p_height];
    m_threshold            = new float*[p_height];

    // check wrappers status
    if (m_imageData    == NULL ||
        m_sw_meanValue == NULL ||
        m_sw_deviation == NULL ||
        m_sw_minGray   == NULL ||
        m_lw_meanValue == NULL ||
        m_lw_deviation == NULL ||
        m_lw_dynRangeDeviation == NULL ||
        m_threshold    == NULL)
    {
        return false;
    }

    // time to wrap now
    for (int i = 0; i < p_height; ++i)
    {
        int offset = i * p_width;

        m_imageData[i]            = p_imageData            + offset;
        m_sw_meanValue[i]         = p_sw_meanValue         + offset;
        m_sw_deviation[i]         = p_sw_deviation         + offset;
        m_sw_minGray[i]           = p_sw_minGray           + offset;
        m_lw_meanValue[i]         = p_lw_meanValue         + offset;
        m_lw_deviation[i]         = p_lw_deviation         + offset;
        m_lw_dynRangeDeviation[i] = p_lw_dynRangeDeviation + offset;
        m_threshold[i]            = p_threshold            + offset;
    }

    return true;
}


void Buffers::GetImageData(unsigned char** data)
{
    for (int i = 0; i < p_height; ++i)
    {
        int offset = i * p_width;

        memcpy_s(p_imageData + offset, p_width * sizeof(unsigned char), data[i], p_width * sizeof(unsigned char));
    }
}


void Buffers::DeAllocateMemory()
{
    if (p_imageData)
    {
        delete[] p_imageData;
        p_imageData = NULL;
    }

    if (p_sw_meanValue)
    {
        delete[] p_sw_meanValue;
        p_sw_meanValue = NULL;
    }

    if (p_sw_deviation)
    {
        delete[] p_sw_deviation;
        p_sw_deviation = NULL;
    }

    if (p_sw_minGray)
    {
        delete[] p_sw_minGray;
        p_sw_minGray = NULL;
    }

    if (p_lw_meanValue)
    {
        delete[] p_lw_meanValue;
        p_lw_meanValue = NULL;
    }

    if (p_lw_deviation)
    {
        delete[] p_lw_deviation;
        p_lw_deviation = NULL;
    }

    if (p_lw_dynRangeDeviation)
    {
        delete[] p_lw_dynRangeDeviation;
        p_lw_dynRangeDeviation = NULL;
    }

    if (p_threshold)
    {
        delete[] p_threshold;
        p_threshold = NULL;
    }

    if (m_imageData)
    {
        delete[] m_imageData;
        m_imageData = NULL;
    }

    if (m_sw_meanValue)
    {
        delete[] m_sw_meanValue;
        m_sw_meanValue = NULL;
    }

    if (m_sw_deviation)
    {
        delete[] m_sw_deviation;
        m_sw_deviation = NULL;
    }

    if (m_sw_minGray)
    {
        delete[] m_sw_minGray;
        m_sw_minGray = NULL;
    }

    if (m_lw_meanValue)
    {
        delete[] m_lw_meanValue;
        m_lw_meanValue = NULL;
    }

    if (m_lw_deviation)
    {
        delete[] m_lw_deviation;
        m_lw_deviation = NULL;
    }

    if (m_lw_dynRangeDeviation)
    {
        delete[] m_lw_dynRangeDeviation;
        m_lw_dynRangeDeviation = NULL;
    }

    if (m_threshold)
    {
        delete[] m_threshold;
        m_threshold = NULL;
    }
}
