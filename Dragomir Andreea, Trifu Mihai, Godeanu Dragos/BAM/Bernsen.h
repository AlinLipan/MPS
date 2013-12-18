#pragma once
#include "./FreeImage/FreeImage.h"

class Bernsen
{
public:

	int originalWidth;
	int originalHeight;
	int paddedWidth;
	int paddedHeight;

	int padding;

	int const_threshold;
	int mid_gray_const;

	int** paddedImage;
	int** binarizedPaddedImage;
	int** confidenceImage;

	Bernsen();
	~Bernsen();

	void Init(int height, int width);
	void Clean();
	void Binarize();
	void ComputeConfidence();
	void Padarray(BYTE** image);
	int GetMax(int** image, int upR, int dnR, int lfC, int rtC);
	int GetMin(int** image, int upR, int dnR, int lfC, int rtC);

};