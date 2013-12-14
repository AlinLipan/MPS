
/**
 * Developed by Lipan Alin - Ionut.
 */

#ifndef __CHRISTIAN_BINARIZATION_H__
#define __CHRISTIAN_BINARIZATION_H__

class MyThread;
class MyBarrier;
class MyMutex;
class Buffers;

class ChristianBinarization
{
private:
    const float mc_alpha1;
    const float mc_k1;
    const float mc_k2;
    const float mc_gamma;

    // prevent copying and assignment
    ChristianBinarization(const ChristianBinarization&);
    ChristianBinarization& operator=(const ChristianBinarization&);

    // Threads will eventually get here and start the binarization process.
    // thisObject represents the ChristianBinarization object that will be lost
    // because of static function call (that is required for spawning threads)
    static void EntryPoint(void* thisObject, int id);

    // Binarization steps...
    void ComputeLocalMeanAndMinGray(const int& from, const int& to, const int& radius);

    void ComputeStandardDeviation(const int& from, const int& to, const int& radius);

    void ComputeLargerMean(const int& from, const int& to, const int& radius);

    void ComputeLargerStandardDeviation(const int& from, const int& t, const int& radiuso);

    void ComputeDynamicRangeSD(const int& from, const int& to, const int& radius);

    void ComputeThreshold(const int& from, const int& to, const int& id);

    void RemoveLonelyPixelsAndComputeConf(const int& from, const int& to);

    void PostProcessingAndAdjustConf(const int& from, const int& to, const int& radius);

    // internal buffers
    Buffers* buffers;

    // binarized image results
    bool* pixelBuffer;
    unsigned char* confidenceBuffer;

    int height;
    int width;

    int numberOfProcessors;

    const int smallerWindowRadius;
    const int largerWindowRadius;

    // created threads go in here
    MyThread* thread;

    // 3 barriers are needed, between two successive barriers the third is being prepared
    MyBarrier* barrier1;
    MyBarrier* barrier2;
    MyBarrier* barrier3;

    // mutex needed
    MyMutex* mutex;

    float maxThreshold;
    float minThreshold;
    float* maxT;
    float* minT;

public:
    // Constructor
    ChristianBinarization();

    // Destructor
    ~ChristianBinarization();

    // inits everything
    bool Init(const int& height, const int& width,
              unsigned char** data);

    // Starts the binarization process...
    void StartBinarization();

    // matrix wrappers over the above buffers
    bool** pixelResult;
    unsigned char** confidenceResult;

};

#endif
