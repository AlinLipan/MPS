
/**
 * Constructor, Destructor, Init and all ComputeFunctions developed by Tanasescu Oana - Adelina (lines 376-).
 * ComputeThreshold and PostProcessing functions developed by Lipan Alin - Ionut (lines 376 - 457).
 * EntryPoint and StartBinarization developed by Marin Andreea - Lavinia (lines 457+).
 * Code reviewed (including coding style) by Lipan Alin - Ionut.
 */

#include "stdafx.h"
#include "ChristianBinarization.h"
#include "Buffers.h"
#include "MyThread.h"
#include "MyBarrier.h"
#include "MyMutex.h"
#include <limits>

ChristianBinarization::ChristianBinarization():
    mc_alpha1(0.15f),
    mc_k1(0.25f),
    mc_k2(0.05f),
    mc_gamma(2),
    buffers(NULL),
    pixelBuffer(NULL),
    confidenceBuffer(NULL),
    width(0),
    height(0),
    numberOfProcessors(0),
    smallerWindowRadius(4),
    largerWindowRadius(12),
    thread(NULL),
    barrier1(NULL),
    barrier2(NULL),
    barrier3(NULL),
    mutex(NULL),
    maxThreshold(FLT_MIN),
    minThreshold(FLT_MAX),
    maxT(NULL),
    minT(NULL),
    pixelResult(NULL),
    confidenceResult(NULL)
{
    // Nothing else to do here...
}


ChristianBinarization::~ChristianBinarization()
{
    if (buffers)
        delete buffers;

    if (pixelBuffer)
        delete[] pixelBuffer;

    if (confidenceBuffer)
        delete[] confidenceBuffer;

    if (pixelResult)
        delete[] pixelResult;

    if (confidenceResult)
        delete[] confidenceResult;

    if (thread)
        delete[] thread;

    if (barrier1)
        delete barrier1;

    if (barrier2)
        delete barrier2;

    if (barrier3)
        delete barrier3;

    if (mutex)
        delete mutex;

    if (maxT)
        delete[] maxT;

    if (minT)
        delete[] minT;
}


bool ChristianBinarization::Init(const int& _height, const int& _width,
                                 unsigned char** data)
{
    height = _height;
    width = _width;

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);

    numberOfProcessors = sysinfo.dwNumberOfProcessors;

    buffers = new Buffers();
    if (buffers && buffers->AllocateMemory(height, width) == false)
    {
        return false;
    }

    buffers->GetImageData(data);

    thread = new MyThread[numberOfProcessors];

    barrier1 = new MyBarrier();
    barrier2 = new MyBarrier();
    barrier3 = new MyBarrier();

    mutex = new MyMutex();

    if (thread == NULL ||
        barrier1 == NULL ||
        barrier2 == NULL ||
        barrier3 == NULL ||
        mutex == NULL)
    {
        return false;
    }

    barrier1->Init(numberOfProcessors);
    barrier2->Init(numberOfProcessors);
    barrier3->Init(numberOfProcessors);

    mutex->Create();

    maxT = new float[numberOfProcessors];
    minT = new float[numberOfProcessors];

    if (maxT == NULL ||
        minT == NULL)
    {
        return false;
    }

    pixelBuffer = new bool[height * width];
    confidenceBuffer = new unsigned char[height * width];

    if (pixelBuffer == NULL ||
        confidenceBuffer == NULL)
    {
        return false;
    }

    pixelResult = new bool*[height];
    confidenceResult = new unsigned char*[height];

    if (pixelResult == NULL ||
        confidenceResult == NULL)
    {
        return false;
    }

    for (int i = 0; i < height; ++i)
    {
        int offset = i * width;

        pixelResult[i]      = pixelBuffer      + offset;
        confidenceResult[i] = confidenceBuffer + offset;
    }

    return true;
}


void ChristianBinarization::ComputeLocalMeanAndMinGray(const int& from,
                                                       const int& to,
                                                       const int& radius)
{
    for (int i = from; i < to; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int nr = 0;
            int sum = 0;
            unsigned char min = 255;

            for (int x = i - radius; x <= i + radius; ++x)
            {
                for (int y = j - radius; y <= j + radius; ++y)
                {
                    if (x < 0 || x >= height)
                        break;
                    if (y < 0 || y >= width)
                        continue;

                    if (buffers->m_imageData[x][y] < min)
                        min = buffers->m_imageData[x][y];

                    sum += buffers->m_imageData[x][y];
                    ++nr;
                }
            }

            buffers->m_sw_minGray[i][j] = min;
            buffers->m_sw_meanValue[i][j] = unsigned char(sum / nr);
        }
    }
}


void ChristianBinarization::ComputeStandardDeviation(const int& from,
                                                     const int& to,
                                                     const int& radius)
{
    for (int i = from; i < to; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int nr = -1;
            int sum = 0;

            for (int x = i - radius; x <= i + radius; ++x)
            {
                for (int y = j - radius; y <= j + radius; ++y)
                {
                    if (x < 0 || x >= height)
                        break;
                    if (y < 0 || y >= width)
                        continue;

                    int temp = buffers->m_imageData[x][y] - buffers->m_sw_meanValue[i][j];
                    sum += temp * temp;
                    ++nr;
                }
            }

            buffers->m_sw_deviation[i][j] = sqrtf(sum / (float)nr);
        }
    }
}


void ChristianBinarization::ComputeLargerMean(const int& from,
                                              const int& to,
                                              const int& radius)
{
    for (int i = from; i < to; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int nr = 0;
            int sum = 0;

            for (int x = i - radius; x <= i + radius; ++x)
            {
                for (int y = j - radius; y <= j + radius; ++y)
                {
                    if (x < 0 || x >= height)
                        break;
                    if (y < 0 || y >= width)
                        continue;

                    sum += buffers->m_imageData[x][y];
                    ++nr;
                }
            }

            buffers->m_lw_meanValue[i][j] = unsigned char(sum / nr);
        }
    }
}


void ChristianBinarization::ComputeLargerStandardDeviation(const int& from,
                                                           const int& to,
                                                           const int& radius)
{
    for (int i = from; i < to; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            int nr = -1;
            int sum = 0;

            for (int x = i - radius; x <= i + radius; ++x)
            {
                for (int y = j - radius; y <= j + radius; ++y)
                {
                    if (x < 0 || x >= height)
                        break;
                    if (y < 0 || y >= width)
                        continue;

                    int temp = buffers->m_imageData[x][y] - buffers->m_lw_meanValue[i][j];
                    sum += temp * temp;
                    ++nr;
                }
            }

            buffers->m_lw_deviation[i][j] = sqrtf(sum / (float)nr);
        }
    }
}


void ChristianBinarization::ComputeDynamicRangeSD(const int& from,
                                                  const int& to,
                                                  const int& radius)
{
    for (int i = from; i < to; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            float max = FLT_MIN;
            float min = FLT_MAX;

            for (int x = i - radius; x <= i + radius; ++x)
            {
                for (int y = j - radius; y <= j + radius; ++y)
                {
                    if (x < 0 || x >= height)
                        break;
                    if (y < 0 || y >= width)
                        continue;

                    if (buffers->m_lw_deviation[x][y] > max)
                        max = buffers->m_lw_deviation[x][y];

                    if (buffers->m_lw_deviation[x][y] < min)
                        min = buffers->m_lw_deviation[x][y];
                }
            }

            buffers->m_lw_dynRangeDeviation[i][j] = max;
            // should of been max - min according to the internet, but gives worse results
        }
    }
}


void ChristianBinarization::ComputeThreshold(const int& from,
                                             const int& to,
                                             const int& id)
{
    for (int i = from; i < to; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            float dynRangeDev = buffers->m_lw_dynRangeDeviation[i][j];
            float sDev = buffers->m_sw_deviation[i][j];
            unsigned char meanValue = buffers->m_sw_meanValue[i][j];
            unsigned char minGray = buffers->m_sw_minGray[i][j];

            float alpha2 = mc_k1 * pow(sDev / dynRangeDev, mc_gamma);
            float alpha3 = mc_k2 * pow(sDev / dynRangeDev, mc_gamma);

            float t1 = (1 - mc_alpha1) * meanValue;
            float t2 = alpha2 * (sDev / dynRangeDev) * (meanValue - minGray);
            float t3 = alpha3 * minGray;

            buffers->m_threshold[i][j] = t1 + t2 + t3;

            float absDelta = abs(buffers->m_threshold[i][j] - buffers->m_imageData[i][j]);

            if (absDelta > maxT[id])
                maxT[id] = absDelta;

            if (absDelta < minT[id])
                minT[id] = absDelta;

            if (buffers->m_imageData[i][j] >= buffers->m_threshold[i][j])
                pixelResult[i][j] = true;
            else
                pixelResult[i][j] = false;

            // threshold is not needed anymore and will now contain the delta value used for confidence matrix
            buffers->m_threshold[i][j] = absDelta;
        }
    }
}


void ChristianBinarization::RemoveLonelyPixelsAndComputeConf(const int& from,
                                                             const int& to)
{
    for (int i = from; i < to; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            confidenceResult[i][j] = static_cast<unsigned char>
                                  (((buffers->m_threshold[i][j] - minThreshold) / maxThreshold) * 255);

            if (pixelResult[i][j])
                continue;

            int blacks = 0;

            for (int x = i - 1; x <= i + 1; ++x)
            {
                for (int y = j - 1; y <= j + 1; ++y)
                {
                    if (x < 0 || x >= height)
                        break;
                    if (y < 0 || y >= width)
                        continue;

                    if (pixelResult[x][y] == false)
                        ++blacks;
                }
            }

            if (blacks == 1)
            {
                pixelResult[i][j] = true;
                confidenceResult[i][j] = 255;
            }
        }
    }
}


void ChristianBinarization::PostProcessingAndAdjustConf(const int& from,
                                                        const int& to,
                                                        const int& radius)
{
    int pixels = (radius * 2 + 1) * (radius * 2 + 1);

    for (int i = from; i < to; ++i)
    {
        for (int j = 0; j < width; ++j)
        {
            if (pixelResult[i][j])
                continue;

            int blacks = 0;
            int total = 0;

            for (int x = i - radius; x <= i + radius; ++x)
            {
                for (int y = j - radius; y <= j + radius; ++y)
                {
                    if (x < 0 || x >= height)
                        break;
                    if (y < 0 || y >= width)
                        continue;

                    ++total;

                    if (pixelResult[x][y] == false)
                        ++blacks;
                }
            }

            if (blacks <= pixels / 8)
            {
                pixelResult[i][j] = true;
                confidenceResult[i][j] = 255;
            }
        }
    }
}


void ChristianBinarization::StartBinarization()
{
    // main thread will wait for these threads to finish their work
    HANDLE* handles = new HANDLE[numberOfProcessors];

    // create optimum amount of threads for the job
    for (int i = 0; i < numberOfProcessors; ++i)
    {
        thread[i].InitThread(EntryPoint, this, i);
        thread[i].CreateThread();
        handles[i] = thread[i].GetHandle();
    }

    WaitForMultipleObjects(numberOfProcessors, handles, TRUE, INFINITE);

    delete[] handles;
}


void ChristianBinarization::EntryPoint(void* _thisObject, int id)
{
    // Threads require a static function as an entry point...
    // We get 'this' object from above as a parameter and treat this function as a member one...
    ChristianBinarization* thisObject = static_cast<ChristianBinarization*>(_thisObject);

    // each thread gets it's own image processing part
    int from = (thisObject->height / thisObject->numberOfProcessors) * id;
    int to = 0;

    // last thread computes everything that's left...
    // ...poor little thread :(
    if (id == thisObject->numberOfProcessors - 1)
    {
        to = thisObject->height;
    }
    else
    {
        to = (thisObject->height / thisObject->numberOfProcessors) * (id + 1);
    }

    thisObject->maxT[id] = FLT_MIN;
    thisObject->minT[id] = FLT_MAX;

    // step 1...
    // computes local mean value and minimum gray value in a smaller window around each pixel
    thisObject->ComputeLocalMeanAndMinGray(from, to, thisObject->smallerWindowRadius);
    thisObject->barrier1->EnterBarrier(id);
    thisObject->barrier2->Prepare(id);

    // step 2...
    // computes the standard deviation of the local mean value in a smaller window around each pixel
    thisObject->ComputeStandardDeviation(from, to, thisObject->smallerWindowRadius);
    thisObject->barrier3->EnterBarrier(id);
    thisObject->barrier1->Prepare(id);

    // step 3...
    // computes local mean value in a larger window around each pixel
    thisObject->ComputeLargerMean(from, to, thisObject->largerWindowRadius);
    thisObject->barrier2->EnterBarrier(id);
    thisObject->barrier3->Prepare(id);

    // step 4...
    // computes standard deviation value in a larger window around each pixel
    thisObject->ComputeLargerStandardDeviation(from, to, thisObject->largerWindowRadius);
    thisObject->barrier1->EnterBarrier(id);
    thisObject->barrier2->Prepare(id);

    // step 5...
    // computes the dynamic range of standard deviation from step 4 in a larger window around each pixel
    thisObject->ComputeDynamicRangeSD(from, to, thisObject->largerWindowRadius);
    thisObject->barrier3->EnterBarrier(id);
    thisObject->barrier1->Prepare(id);

    // step 6...
    // computes the threshold for each pixel, and finds the max / min of the abs(pixel value - threshold)
    thisObject->ComputeThreshold(from, to, id);
    thisObject->barrier2->EnterBarrier(id);
    thisObject->barrier3->Prepare(id);

    // step 7...
    // find the maximum and minimum delta values from step 6
    thisObject->mutex->LockMutex();

    if (thisObject->maxT[id] > thisObject->maxThreshold)
        thisObject->maxThreshold = thisObject->maxT[id];

    if (thisObject->minT[id] < thisObject->minThreshold)
        thisObject->minThreshold = thisObject->minT[id];

    thisObject->mutex->UnlockMutex();

    thisObject->barrier1->EnterBarrier(id);
    thisObject->barrier2->Prepare(id);

    // step 8...
    // adjust the [min, max] interval to [0, max]
    if (id == 0)
    {
        thisObject->maxThreshold -= thisObject->minThreshold;
    }
    thisObject->barrier3->EnterBarrier(id);
    thisObject->barrier1->Prepare(id);

    // step 9...
    // remove lonely black pixels and compute confidence based on the delta value from step 6
    // removed pixels will be set to 255 in the confidence matrix
    thisObject->RemoveLonelyPixelsAndComputeConf(from, to);
    thisObject->barrier2->EnterBarrier(id);
    thisObject->barrier3->Prepare(id);

    // step 10...
    // remove pixels that are fewer than a limit, inside a radius around them
    // removed pixels will be set to 255 in the confidence matrix
    thisObject->PostProcessingAndAdjustConf(from, to, thisObject->smallerWindowRadius);
}