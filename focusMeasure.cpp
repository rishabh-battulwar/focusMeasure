// focusMeasure.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>

//using namespace cv;

//double modifiedLaplacian(const cv::Mat& src);
double varianceOfLaplacian(const cv::Mat& src);
//double normalizedGraylevelVariance(const cv::Mat& src);

int main(int argc, char *argv[])
{
	//char inFile[100] = "C:\\data\\cameraFocusData\\image001.bmp";
	char inFile[100] = "C:/data/img_color.bmp";

	char methodName[10] = "lapv"; //default
	std::string longMethodName = "Variance of Laplacian";
	int methodNumber;

	if (argc == 1)
	{
		std::cout << "\n Usage: .\\focusMeasure.exe -i <imagePath> \n\n";
		exit(0);
	}

	int arg = 0;
	while (++arg < argc)
	{
		//Input Image
		if (!strcmp(argv[arg], "-i"))
			strcpy_s(inFile, argv[++arg]);
		
		////Method to use
		//if (!strcmp(argv[arg], "-m"))
		//	strcpy_s(methodName, argv[++arg]);
	}

	//if (!strcmp(methodName, "lapm"))
	//	methodNumber = 1;
	//else if (!strcmp(methodName, "lapv"))
	//	methodNumber = 2;
	//else //(!strcmp(methodName, "glvn"))
	//	methodNumber = 3;

	cv::Mat sourceFile;
	double FM, maxFM = 0;
	sourceFile = cvLoadImage(inFile);
	//sourceFile = cv::imread(inFile);
	
	int topLeftX = 721, bottomRightX = 1641;
		for (int i = 0; i < 6; i++)
	{
		int topLeftY, bottomRightY;
		topLeftY = 120 + (i * 30); bottomRightY = topLeftY + 240;
		cv::Rect ROI(cv::Point(topLeftX,topLeftY), cv::Point(bottomRightX, bottomRightY));

		// Crop the full image to that image contained by the rectangle myROI
		// Note that this doesn't copy the data
		cv::Mat croppedRef(sourceFile, ROI);

		cv::Mat cropped;
		// Copy the data into new matrix
		croppedRef.copyTo(cropped);

		FM = 0;
		FM = varianceOfLaplacian(cropped);

		if (maxFM < FM) maxFM = FM;
	}
	std::cout << "\n MethodName: " << longMethodName << "\n Focus Measure = " << maxFM << "\n" <<std::endl;
	if (maxFM > 250.00)
		std::cout << " Good Camera \n" << std::endl;
	else
		std::cout << " Bad Camera \n" << std::endl;

	return 0;
}

//double modifiedLaplacian(const cv::Mat& src)
//{
//	cv::Mat M = (cv::Mat_<double>(3, 1) << -1, 2, -1);
//	cv::Mat G = cv::getGaussianKernel(3, -1, CV_64F);
//
//	cv::Mat Lx;
//	cv::sepFilter2D(src, Lx, CV_64F, M, G);
//
//	cv::Mat Ly;
//	cv::sepFilter2D(src, Ly, CV_64F, G, M);
//
//	cv::Mat FM = cv::abs(Lx) + cv::abs(Ly);
//
//	double focusMeasure = cv::mean(FM).val[0];
//	return focusMeasure;
//}

double varianceOfLaplacian(const cv::Mat& src)
{
	cv::Mat lap;
	cv::Laplacian(src, lap, CV_64F);

	cv::Scalar mu, sigma;
	cv::meanStdDev(lap, mu, sigma);

	double focusMeasure = sigma.val[0] * sigma.val[0];
	return focusMeasure;
}

//double normalizedGraylevelVariance(const cv::Mat& src)
//{
//	cv::Scalar mu, sigma;
//	cv::meanStdDev(src, mu, sigma);
//
//	double focusMeasure = (sigma.val[0] * sigma.val[0]) / mu.val[0];
//	return focusMeasure;
//}