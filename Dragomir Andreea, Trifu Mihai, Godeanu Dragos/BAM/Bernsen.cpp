#include "stdafx.h"
#include "Bernsen.h"

Bernsen::Bernsen()
{
}

Bernsen::~Bernsen()
{
}

// Initializeaza constantele si aloca memorie pentru stocarea imaginilor
void Bernsen::Init(int height, int width)
{
	originalHeight = height;
	originalWidth = width;
	
	// seteaza constantele si dimensiunea padding-ului
	originalWidth < originalHeight ? padding = originalWidth / 3 : padding = originalHeight / 3;
	const_threshold = 50;
	mid_gray_const = 128;

	// noua dimensiune dupa padding
	paddedHeight = originalHeight + 2 * padding;
	paddedWidth = originalWidth + 2 * padding;

	// alocare matrice cu padding, in care se va salva imaginea binarizata
	binarizedPaddedImage = new int*[paddedHeight];
	for (int i = 0; i < paddedHeight; i++)
		binarizedPaddedImage[i] = new int[paddedWidth];

	// alocare matrice cu padding, in care se va salva imaginea grayscale cu padding
	paddedImage = new int*[paddedHeight];
	for (int i = 0; i < paddedHeight; i++)
		paddedImage[i] = new int[paddedWidth];

	// alocare matrice cu padding, in care se va salva imaginea de confidenta
	confidenceImage = new int*[paddedHeight]();
	for (int i = 0; i < paddedHeight; i++)
		confidenceImage[i] = new int[paddedWidth]();

	return;
}

// Dezaloca memorie
void Bernsen::Clean()
{
	if (binarizedPaddedImage)
	{
		for (int i = 0; i < paddedHeight; i++)
			delete[] binarizedPaddedImage[i];
		delete[] binarizedPaddedImage;
	}
	
	if (paddedImage)
	{
		for (int i = 0; i < paddedHeight; i++)
			delete[] paddedImage[i];
		delete[] paddedImage;
	}

	if (confidenceImage)
	{
		for (int i = 0; i < paddedHeight; i++)
			delete[] confidenceImage[i];
		delete[] confidenceImage;
	}
}

// Realizeaza padingul imaginii grayscale initiale
// Paddingul se face pe toate laturile matricii, 
//cu oglinditul acesteia fata de marginea respectiva
void Bernsen::Padarray(BYTE** image)
{
	int i,j;

	// copiaza imaginea initiala in mijlocul imaginii cu padding
	for (i = 0; i < originalHeight; i++)
		for (j = 0; j < originalWidth; j++)
			paddedImage[i+padding][j+padding] = image[i][j];

	// realizeaza paddingul la stanga si la dreapta imaginii
	for (i = padding; i < padding + originalHeight; i++)
		for (j = 1; j <= padding; j++)
		{
			paddedImage[i][padding-j] = image[i-padding][j];
			paddedImage[i][padding+originalWidth-1+j] = image[i-padding][originalWidth-1-j];
		}

	// realizeaza paddingul in partea de sus si de jos, inclusiv colturile
	for (j = 0; j < 2*padding + originalWidth; j++)
		for (i = 1; i <= padding; i++)
		{
			paddedImage[padding-i][j] = paddedImage[padding+i][j];
			paddedImage[padding+originalHeight-1+i][j] = paddedImage[padding+originalHeight-1-i][j];
		}

	return;
}

// Returneaza pixelul cu valoarea maxima din zona data ca parametru
int Bernsen::GetMax(int** image, int upR, int dnR, int lfC, int rtC)
{
	int max = 0;

	for (int j = lfC; j < rtC; j++)
		for (int i = upR; i < dnR; i++)
			if (image[i][j] > max)
				max =image[i][j];

	return max;
}

// Returneaza pixelul cu valoarea minima din zona data ca parametru
int Bernsen::GetMin(int** image, int upR, int dnR, int lfC, int rtC)
{
	int min = 255;

	for (int j = lfC; j < rtC; j++)
		for (int i = upR; i < dnR; i++)
			if (image[i][j] < min)
				min = image[i][j];

	return min;
}

// Calculeaza matricea de confidenta
void Bernsen::ComputeConfidence()
{
	int min_th = GetMin(confidenceImage, padding, originalHeight+padding, padding, originalWidth+padding);
	int max_th = GetMax(confidenceImage, padding, originalHeight+padding, padding, originalWidth+padding);

	for (int i = padding; i < originalHeight + padding; i++)
		for (int j = padding; j < originalWidth + padding; j++)
			confidenceImage[i][j] = (float)(confidenceImage[i][j] - min_th)/max_th * 255;
}

// Realizeaza binarizarea propriu-zisa
void Bernsen::Binarize()
{
	int i, j, m, n;
	int upR, dnR, lfC, rtC;
	int aux1, aux2;
	int th_ij;
	int max_gray, min_gray, mid_gray, con_dif;
	
	for (i = padding; i < originalHeight + padding; i += padding)
	{
		for (j = padding; j < originalWidth + padding; j += padding)
		{
			aux1 = (float)padding / 2 - 0.5;
			aux2 = padding/2 + 1;

			upR =  i - aux1;
			dnR =  i + aux2;
			lfC =  j - aux1;
			rtC =  j + aux2;

			max_gray = GetMax(paddedImage, upR, dnR, lfC, rtC);
			min_gray = GetMin(paddedImage, upR, dnR, lfC, rtC);
			mid_gray = (max_gray + min_gray) / 2;

			con_dif = max_gray - min_gray;

			if (con_dif >= const_threshold)
				th_ij = mid_gray;
			else
				th_ij = mid_gray_const;

			for (m = upR; m < dnR; m++)
				for (n = lfC; n < rtC; n++)
				{
					binarizedPaddedImage[m][n] = paddedImage[m][n] > th_ij ? 1 : 0;
//					confidenceImage[m][n] = abs(mid_gray - paddedImage[m][n]);
					confidenceImage[m][n] = (float)(abs(mid_gray - paddedImage[m][n]) - min_gray) / max_gray * 255;
				}
		}
	}
}

