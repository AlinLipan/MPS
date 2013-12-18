#include "stdafx.h"
#include "Direct_Access_Image.h"
#include <iostream>
#include <limits>
#include <ctime>

using namespace std;

// get min value from a matrix
double getMinValue(KImage *im)
{
	BYTE min = numeric_limits<BYTE>::max();
	int nr_rows = im->GetHeight();
	int nr_cols = im->GetWidth();

	for(int i = 0; i < nr_rows; ++i)
		for(int j = 0; j < nr_cols; ++j)
			if(im->GetDataMatrix()[i][j] < min)
				min = im->GetDataMatrix()[i][j];

	return static_cast<double>(min);;
}


// *************************************************************
// glide a window across the image and
// create two maps: mean and standard deviation.
// *************************************************************
double calcLocalStats (KImage *im, double **map_m, double **map_s, int winx, int winy)
{

	double m, s, max_s, sum, sum_sq, foo;
	int nr_rows = im->GetHeight();
	int nr_cols = im->GetWidth();
	int wxh	= winx / 2;
	int wyh	= winy / 2;
	int x_firstth= wxh;
	int y_lastth = nr_rows - wyh - 1;
	int y_firstth= wyh;
	double winarea = winx * winy;
	
	max_s = 0;
	for	(int j = y_firstth ; j <= y_lastth; ++j) 
	{
		// Calculate the initial window at the beginning of the line
		sum = sum_sq = 0;

		for	(int wy = 0 ; wy < winy; ++wy)
			for	(int wx = 0 ; wx < winx; ++wx)
			{
				foo = im->GetDataMatrix()[j - wyh + wy][wx];
				sum    += foo;
				sum_sq += foo * foo;
			}

		m  = sum / winarea;
		s  = sqrt ((sum_sq - (sum*sum)/winarea)/winarea);

		if (s > max_s)
			max_s = s;

		map_m[j][x_firstth] = m;		
		map_s[j][x_firstth] = s;

		// Shift the window, add and remove	new/old values to the histogram
		for	(int i = 1 ; i <= nr_cols - winx; ++i)
		{
			// Remove the left old column and add the right new column
			for (int wy = 0; wy < winy; ++wy)
			{
				//foo = im->GetDataMatrix()[i - 1][j - wyh + wy];
				foo = im->GetDataMatrix()[j - wyh + wy][i - 1];
				sum    -= foo;
				sum_sq -= foo * foo;
				//foo = im->GetDataMatrix()[i + winx - 1][j - wyh + wy];
				foo = im->GetDataMatrix()[j - wyh + wy][i + winx - 1];
				sum    += foo;
				sum_sq += foo*foo;
			}

			m  = sum / winarea;
			s  = sqrt ((sum_sq - (sum * sum) / winarea) / winarea);
			if (s > max_s)
				max_s = s;
			map_m[j][i + wxh] = m;
			map_s[j][i + wxh] = s;
		}
	}
	
	return max_s;
}

/**********************************************************
 * The binarization routine
 **********************************************************/

void Wolf(KImage *im, KImage *output, KImage *conf, int winx, 
	int winy, double k)
{	
	double m, s, max_s;
	int nr_rows = im->GetHeight();
	int nr_cols = im->GetWidth();
	double th = 0;
	double min_I;
	int wxh	= winx / 2;
	int wyh	= winy / 2;
	int x_firstth= wxh;
	int x_lastth = nr_cols - wxh - 1;
	int y_lastth = nr_rows - wyh - 1;
	int y_firstth= wyh;	

	// Create local statistics and store them in a double matrices	
	double **map_m = new double * [nr_rows];
	for(int i = 0; i < nr_rows; ++i)
		map_m[i] = new double[nr_cols];

	double **map_s = new double * [nr_rows];
	for(int i = 0; i < nr_rows; ++i)
		map_s[i] = new double[nr_cols];

	max_s = calcLocalStats (im, map_m, map_s, winx, winy);

	// compute max/min value from a matrix
	min_I = getMinValue(im);

	double **thsurf = new double * [nr_rows];
	for(int i = 0; i < nr_rows; ++i)
		thsurf[i] = new double[nr_cols];
			
	// Create the threshold surface, including border processing
	// ----------------------------------------------------

	for	(int j = y_firstth ; j <= y_lastth; ++j)
	{
		// NORMAL, NON-BORDER AREA IN THE MIDDLE OF THE WINDOW:
		for	(int i = 0 ; i <= nr_cols - winx; ++i)
		{

			m  = map_m[j][i + wxh];
	    	s  = map_s[j][i + wxh] ;
			th = m + k * (s/max_s-1) * (m - min_I);	    	
			thsurf[j][i + wxh] = th;

	    	if (i == 0)
			{
				// LEFT BORDER
				for (int i = 0; i <= x_firstth; ++i)		        	
					thsurf[j][i] = th;

				// LEFT-UPPER CORNER
				if (j == y_firstth)
					for (int u = 0; u < y_firstth; ++u)
						for (int i = 0; i <= x_firstth; ++i)							
							thsurf[u][i] = th;

				// LEFT-LOWER CORNER
				if (j == y_lastth)
					for (int u = y_lastth + 1; u < nr_rows; ++u)
						for (int i = 0; i <= x_firstth; ++i)							
							thsurf[u][i] = th;
	    	}

			// UPPER BORDER
			if (j == y_firstth)
				for (int u = 0; u < y_firstth; ++u)					
					thsurf[u][i + wxh] = th;

			// LOWER BORDER
			if (j == y_lastth)
				for (int u = y_lastth + 1; u < nr_rows; ++u)					
					thsurf[u][i + wxh] = th;
		}

		// RIGHT BORDER
		for (int i = x_lastth; i < nr_cols; ++i)        		
				thsurf[j][i] = th;

  		// RIGHT-UPPER CORNER
		if (j == y_firstth)
			for (int u = 0; u < y_firstth; ++u)
				for (int i = x_lastth; i < nr_cols; ++i)					
					thsurf[u][i] = th;

		// RIGHT-LOWER CORNER
		if (j == y_lastth)
			for (int u = y_lastth + 1; u < nr_rows; ++u)
				for (int i = x_lastth; i < nr_cols; ++i)					
					thsurf[u][i] = th;
	}
	
	if (im->BeginDirectAccess() && im->GetDataMatrix() != NULL && 
		output->BeginDirectAccess() && output->GetDataMatrix() != NULL &&
		conf->BeginDirectAccess() && conf->GetDataMatrix() != NULL)
	{
		for(int y = 0; y < nr_rows; ++y) 
			for(int x = 0; x < nr_cols; ++x) 
			{
		    		if (static_cast<double>(im->GetDataMatrix()[y][x]) >= thsurf[y][x])
		    		{
		    			output->GetDataMatrix()[y][x] = 255;
						conf->GetDataMatrix()[y][x] = (BYTE)(im->GetDataMatrix()[y][x] - thsurf[y][x]);							
		    		}
		    		else
		    		{						
						output->GetDataMatrix()[y][x] = 0;
						conf->GetDataMatrix()[y][x] = (BYTE)(thsurf[y][x] - im->GetDataMatrix()[y][x]);								    	   
		    		}					
	    		}

			im->EndDirectAccess();
			output->EndDirectAccess();
			conf->EndDirectAccess();
	}

	
	// free memory
	int i = 0;
	while(i != nr_rows)
	{
		delete [] map_m[i];
		delete [] map_s[i];
		delete [] thsurf[i];
		++i;
	}

	delete map_m;
	delete map_s;
	delete thsurf;

}


int _tmain(int argc, _TCHAR* argv[])
{
    //Verify command-line usage correctness
    if (argc != 4)
    {
        _tprintf(_T("Usage : bam.exe input output binary_image\n"));		
        return -1;
    }	
	
    //Buffer for the new file names
    TCHAR strNewFileName[0x100];
	 _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s"), argv[1]);


    //Load and verify that input image is a True-Color one
    KImage *pImage = new KImage(argv[1]);
    if (pImage == NULL || !pImage->IsValid() || pImage->GetBPP() != 24)
    {
        _tprintf(_T("File %s does is not a valid True-Color image!"), argv[0]);		
        return -2;
    }	
	
    //Apply a Gaussian Blur with small radius to remove obvious noise
    pImage->GaussianBlur(0.5);   

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

	int winx = 0, winy = 0;
	double optK = 0.5;

	winy = (int) (2.0 * pImageGrayscale->GetHeight() - 1) / 3;
    winx = (int) pImageGrayscale->GetWidth() - 1 < winy ? pImageGrayscale->GetWidth() - 1 : winy;

    // if the window is too big, than we asume that the image
    // is not a single text box, but a document page: set
    // the window size to a fixed constant.
    if (winx > 100)	
		winx = winy = 40;	

    //Request direct access to image pixels in raw format
    BYTE **pDataMatrixGrayscale = NULL;
    if (pImageGrayscale->BeginDirectAccess() && (pDataMatrixGrayscale = pImageGrayscale->GetDataMatrix()) != NULL)
    {
        //If direct access is obtained get image attributes and start processing pixels
        int intWidth = pImageGrayscale->GetWidth();
        int intHeight = pImageGrayscale->GetHeight();

        //Create binary image      
		KImage *output = new KImage(intWidth, intHeight, 8);
		KImage *conf = new KImage(intWidth, intHeight, 8);
		Wolf(pImageGrayscale, output, conf, winx, winy, optK);
		 _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s"), argv[2]);
		 output->SaveAs(strNewFileName, SAVE_TIFF_LZW);	
		 _stprintf_s(strNewFileName, sizeof(strNewFileName) / sizeof(TCHAR), _T("%s"), argv[3]);		
		 conf->SaveAs(strNewFileName, SAVE_TIFF_LZW);

		 delete output;		
		 delete conf;
        //Close direct access
        pImageGrayscale->EndDirectAccess();
    }
    else
    {
        _tprintf(_T("Unable to obtain direct access in grayscale image!"));		
        return -4;
    }

    // delete the grayscale image
    delete pImageGrayscale;
	
    //Return with success
    return 0;
}
