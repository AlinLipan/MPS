//===========================================================================
//===========================================================================
//===========================================================================
//==   Direct_Access_Image_Sample.cpp  ==  Author: Costin-Anton BOIANGIU   ==
//===========================================================================
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
#include "stdafx.h"
#include "Direct_Access_Image.h"
#include "Bernsen.h"
//===========================================================================
//===========================================================================

//===========================================================================
//===========================================================================
int _tmain(int argc, _TCHAR* argv[])
{
    //Verify command-line usage correctness
    if (argc != 4)
    {
        _tprintf(_T("Use: %s <Input_Image_File_Name (24BPP True-Color)> <Output_Image_File_Name> <Output_Confidence_File_Name>\n"), argv[0]);
        return -1;
    }

    //Buffer for the new file names
    TCHAR strNewFileName[0x100];

    //Load and verify that input image is a True-Color one
    KImage *pImage = new KImage(argv[1]);
    if (pImage == NULL || !pImage->IsValid() || pImage->GetBPP() != 24)
    {
        _tprintf(_T("File %s does is not a valid True-Color image!"), argv[0]);
        return -2;
    }
	
    //Apply a Gaussian Blur with small radius to remove obvious noise
    pImage->GaussianBlur(0.5);
    _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_blurred.TIF"), argv[1]);
    pImage->SaveAs(strNewFileName, SAVE_TIFF_LZW);

    //Convert to grayscale
    KImage *pImageGrayscale = pImage->ConvertToGrayscale();
    //Don't forget to delete the original, now useless image
    delete pImage;

    //Verify conversion success...
    if (pImageGrayscale == NULL || !pImageGrayscale->IsValid() || pImageGrayscale->GetBPP() != 8)
    {
        _tprintf(_T("Conversion to grayscale was not successfull!"));
        return -3;
    }
    //... and save grayscale image
    _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s_grayscale.TIF"), argv[1]);
    pImageGrayscale->SaveAs(strNewFileName, SAVE_TIFF_LZW);
    
    //Request direct access to image pixels in raw format
    BYTE **pDataMatrixGrayscale = NULL;
    if (pImageGrayscale->BeginDirectAccess() && (pDataMatrixGrayscale = pImageGrayscale->GetDataMatrix()) != NULL)
    {
        //If direct access is obtained get image attributes and start processing pixels
        int intWidth = pImageGrayscale->GetWidth();
        int intHeight = pImageGrayscale->GetHeight();

		//Init and run binarization algorithm
		Bernsen bernsenAlg;
		bernsenAlg.Init(intHeight, intWidth);
		bernsenAlg.Padarray(pDataMatrixGrayscale);
		bernsenAlg.Binarize();

        //Create binary image
        KImage *pImageBinary = new KImage(intWidth, intHeight, 1);
        if (pImageBinary->BeginDirectAccess())
        {
            for (int y = intHeight - 1; y >= 0; y--)
                for (int x = intWidth - 1; x >= 0; x--)
                {
					// Save the binarized image without padding in pImageBinary
                    if (bernsenAlg.binarizedPaddedImage[y+bernsenAlg.padding][x+bernsenAlg.padding] == 0)
						pImageBinary->Put1BPPPixel(x, y, false);
					else
						pImageBinary->Put1BPPPixel(x, y, true);
				}

            //Close direct access
            pImageBinary->EndDirectAccess();
            
            //Save binarized image
            _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s.TIF"), argv[2]);
            pImageBinary->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4);

            //Don't forget to delete the binary image
            delete pImageBinary;
        }
        else
        {
            _tprintf(_T("Unable to obtain direct access in binary image!"));
            return -3;
        }

		//Create confidence image
		KImage *pImageConfidence = new KImage(intWidth, intHeight, 8);
		if (pImageConfidence->BeginDirectAccess())
        {

//			bernsenAlg.ComputeConfidence();

            //Apply a threshold at half the grayscale range (0x00 is Full-Black, 0xFF is Full-White, 0x80 is the Middle-Gray)
            for (int y = intHeight - 1; y >= 0; y--)
                for (int x = intWidth - 1; x >= 0; x--)
                    pImageConfidence->Put8BPPPixel(x, y, bernsenAlg.confidenceImage[y+bernsenAlg.padding][x+bernsenAlg.padding]);

            //Close direct access
            pImageConfidence->EndDirectAccess();
            
            //Save binarized image
            _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s.TIF"), argv[3]);
            pImageConfidence->SaveAs(strNewFileName, SAVE_TIFF_CCITTFAX4);

            //Don't forget to delete the confidence image
            delete pImageConfidence;
        }
        else
        {
            _tprintf(_T("Unable to obtain direct access in confidence image!"));
            return -3;
        }

        //Close direct access
        pImageGrayscale->EndDirectAccess();

		//Clear memory used for binarization
		bernsenAlg.Clean();
    }
    else
    {
        _tprintf(_T("Unable to obtain direct access in grayscale image!"));
        return -4;
    }

    //Don't forget to delete the grayscale image
    delete pImageGrayscale;

    //Return with success
    return 0;
}
//===========================================================================
//===========================================================================
