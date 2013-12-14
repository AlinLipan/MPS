
/**
 * Developed by Lipan Alin - Ionut.
 */
#include "stdafx.h"

#include <string>

#include "ChristianBinarization.h"
#include "Direct_Access_Image.h"

#define SUCCESS        0
#define READ_ERROR    -1
#define IMAGE_PROBLEM -2
#define WRITE_ERROR   -3
#define OUT_OF_MEMORY -4


int _tmain(int argc, _TCHAR* argv[])
{
    // argc[0] -> exe name
    // argc[1] -> input image name
    // argc[2] -> output image name (binarization)
    // argc[3] -> output confidence matrix name
    if (argc != 4)
    {
        _tprintf(_T("Use: %s inputImageFile outputImageFile outputConfidenceImageFile"), argv[0]);
        return READ_ERROR;
    }

    // Try and load input image file...
    KImage* pImage = new KImage(argv[1]);
    if (pImage == NULL ||
       !pImage->IsValid() ||
       pImage->GetBPP() == 1)
    {
        _tprintf(_T("File %s is not a valid image!"), argv[0]);
        return IMAGE_PROBLEM;
    }

    // Convert to grayscale...
    KImage* pImageGrayscale = pImage->ConvertToGrayscale();

    // And delete old image (not needed anymore)...
    delete pImage;
    pImage = NULL;

    // Verify conversion success...
    if (pImageGrayscale == NULL || !pImageGrayscale->IsValid() || pImageGrayscale->GetBPP() != 8)
    {
        _tprintf(_T("Conversion to grayscale was not successfull!"));
        return WRITE_ERROR;
    }

    // Now save our grayscale image...
    {
        std::wstring grayscale = argv[1];
        grayscale = grayscale.substr(0, grayscale.find_last_of('.'));
        grayscale += L"_grayscale.tif";
        pImageGrayscale->SaveAs(grayscale.c_str(), SAVE_TIFF_LZW);
    }

    // Get raw image data...
    BYTE** pDataMatrixGrayscale = NULL;
    if (pImageGrayscale->BeginDirectAccess() && (pDataMatrixGrayscale = pImageGrayscale->GetDataMatrix()) != NULL)
    {
        int width = pImageGrayscale->GetWidth();
        int height = pImageGrayscale->GetHeight();

        // Create the new binary image...
        KImage* pImageBinary = new KImage(width, height, 1);

        // And the confidence grayscale image...
        KImage* pConfImage = new KImage(width, height, 8);

        if (pImageBinary == NULL ||
            pConfImage   == NULL)
            return OUT_OF_MEMORY;

        ///////////////////////////////////////////////////////////////////
        // All the work goes in here...
        {
            // Create ChristianBinarization object...
            ChristianBinarization binarization;
            if (binarization.Init(height, width, pDataMatrixGrayscale))
            {
                // If everything is fine start the binarization process...
                binarization.StartBinarization();

                // And then create the required images...
                pImageBinary->BeginDirectAccess();
                pConfImage->BeginDirectAccess();
                for (int i = 0; i < width; ++i)
                {
                    for (int j = 0; j < height; ++j)
                    {
                        pConfImage->Put8BPPPixel(i, j, binarization.confidenceResult[j][i]);

                        if (binarization.pixelResult[j][i])
                            pImageBinary->Put1BPPPixel(i, j, true);
                        else
                            pImageBinary->Put1BPPPixel(i, j, false);
                    }
                }
                pConfImage->EndDirectAccess();
                pImageBinary->EndDirectAccess();
            }
            else
            {
                return OUT_OF_MEMORY;
            }
        }
        ///////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////

        // Save the confidence image...
        pConfImage->SaveAs(argv[3], SAVE_TIFF_LZW);

        // Save the binary image...
        pImageBinary->SaveAs(argv[2], SAVE_TIFF_CCITTFAX4);

        // Clean up the mess...
        delete pConfImage;
        pConfImage = NULL;

        delete pImageBinary;
        pImageBinary = NULL;
    }
    else
    {
        return WRITE_ERROR;
    }

    pImageGrayscale->EndDirectAccess();
    delete pImageGrayscale;
    pImageGrayscale = NULL;

    return SUCCESS;
}