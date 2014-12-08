// laserWidth.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <cv.h>
#include <highgui.h>
#include <windows.h>



void findBoundaries(double &, cv::Point &, cv::Point &, cv::Mat &, int &, int &);
void merge(int *, int, int, int);
void mergesort(int *, int, int);
int GetMedian(int *, int);



int _tmain(int argc, _TCHAR* argv[])
{
	std::vector<std::wstring> list;
	LPCWSTR a = L"C:/data/LaserData/withLaser2/*.bmp";
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(a, &fd);

	if (h == INVALID_HANDLE_VALUE)
	{
		return 0; // no files found
	}

	while (1)
	{
		list.push_back(fd.cFileName);

		if (FindNextFile(h, &fd) == FALSE)
			break;
	}

	std::vector<std::string> ListOfFiles;
	for (int pos = 0; pos < list.size(); pos++)
	{
		std::wstreampos wpos = pos;
		std::wstring ws = list.at(wpos);
		std::string s(ws.begin(), ws.end());
		ListOfFiles.push_back(s);
	}

	std::vector<int> WidthTable;
	std::vector<float> avgBrightness;

	for (int index = 0; index < ListOfFiles.size(); index++)
	{
		std::string path = "C:/data/LaserData/withLaser2/" + ListOfFiles.at(index);
		cv::Mat src = cv::imread(path, 1);
		cv::Mat gray;
		cv::cvtColor(src, gray, CV_BGR2GRAY);
		cv::Mat srcRow = cv::Mat::zeros(1, src.cols, CV_8UC1);

		int avgWidth = 0, width_sum = 0;
		int rowNumbersArr[4] = { 5, 180, 550, 850 };
		std::vector<int> rowNumbers;
		for (int init = 0; init < 4; init++) rowNumbers.push_back(rowNumbersArr[init]);
		std::vector<int> medianWidths;
		for (int init = 0; init < 4; init++) medianWidths.push_back(0);
		int widthArray[11] = { 0 };

		for (int j = 0; j < rowNumbers.size(); j++)
		{
			for (int i = (rowNumbers.at(j) - 5), index = 0; i < (rowNumbers.at(j) + 6); i++, index++) //j should be atleast 5 and atmost 1913, else i < 0, >1919
			{
				double min = 0.0, max = 0.0;
				cv::Point minl; cv::Point maxl;
				int rightBoundary = 0, leftBoundary = 0;

				gray.row(i).copyTo(srcRow);
				cv::minMaxLoc(srcRow, &min, &max, &minl, &maxl);
				cv::Point maxlCopy = maxl;

				findBoundaries(max, maxl, maxlCopy, srcRow, rightBoundary, leftBoundary);

				widthArray[index] = (rightBoundary - leftBoundary + 1);
				//width.at(j) += (rightBoundary - leftBoundary + 1); //for average instead of median
			}

			mergesort(widthArray, 0, 10);
			medianWidths.at(j) = GetMedian(widthArray, 11);
			//width.at(j) = width.at(j) / 10; //for average instead of median
		}

		for (int k = 0; k < rowNumbers.size(); k++)
		{
			width_sum += medianWidths.at(k);
		}

		avgWidth = width_sum / rowNumbers.size();

		WidthTable.push_back(avgWidth);

		//#########################################################################################
		
		std::string path2 = "C:/data/LaserData/withoutLaser2/" + ListOfFiles.at(index);
		cv::Mat src_nolaser = cv::imread(path2, 1);
		cv::Mat gray_nolaser;
		cv::cvtColor(src_nolaser, gray_nolaser, CV_BGR2GRAY);

		cv::Scalar avg = cv::mean(gray_nolaser);

		avgBrightness.push_back(avg[0]);
		
	}

	//int maxWidth = *std::max_element(WidthTable.begin(), WidthTable.end());
	//float maxBrightness = *std::max_element(avgBrightness.begin(), avgBrightness.end());

	//float factor = (float)(maxBrightness / maxWidth);

	//Normalize Brightness values to max of Width
	//for (int index = 0; index < ListOfFiles.size(); index++)
	//	avgBrightness.at(index) = avgBrightness.at(index) / factor;

	//std::vector<float> diff;
	float diff_prev, diff_curr;
	int bestWidth = 7;
	float brightnessCap = 3.25;

	//choose the frame that gives the lowest difference
	/*for (int index = 0; index < ListOfFiles.size(); index++)
	{
		difference = ((bestWidth - WidthTable.at(index)) + (bestBrightness - avgBrightness.at(index)));
		difference = std::abs(difference);
		diff.push_back(difference);
	}

	std::vector<float>::iterator result = std::min_element(diff.begin(), diff.end());
	int frameNumber = std::distance(diff.begin(), result);*/
	int i = 0; int frameNumber = 0; int flag = 0; float min_diff = 100;
	while (avgBrightness.at(i) < brightnessCap\
		&& i < avgBrightness.size())
	{
		diff_curr = WidthTable.at(i) - bestWidth;
		if (min_diff > std::abs(diff_curr))
		{
			min_diff = std::abs(diff_curr);
			frameNumber = i;
			flag = 1;

		}
		i++;
		/*if (diff_curr == 0)
		{
			frameNumber = i;  flag = 1;
			break;
		}
		else if (diff_curr < 0)
		{
			i++;	diff_prev = diff_curr;
			continue;
		}
		else
		{
			if (diff_curr > std::abs(diff_prev)) frameNumber = i - 1;
			else frameNumber = i;
			flag = 1;
			break;
		}*/
	}

	if (flag == 1)	std::cout << "Frame to choose: " << frameNumber + 1 << std::endl;
	else std::cout << "Unable to get thick laser at ideal range of brightness" << std::endl;

	return 0;
}


void findBoundaries
(double &max, cv::Point &maxl, cv::Point &maxlCopy, cv::Mat &srcRow, int &rightBoundary, int &leftBoundary)
{
	uchar grayValue;
	bool flagL, flagR;

	double minR, minL, maxR, maxL;
	cv::Point minLocR, minLocL, maxLocR, maxLocL;

	//Right-bound
	flagR = true;
	while (flagR)
	{
		grayValue = srcRow.at<uchar>(++(maxl.x));
		if (grayValue > (0.25*max)) rightBoundary = maxl.x;
		else						flagR = false; 
	}

	//checking for false boundary on the right
	cv::Mat srcRowRight = cv::Mat::zeros(1, (srcRow.cols - rightBoundary - 1), CV_8UC1);
	for (int i = 0; i < (srcRow.cols - rightBoundary - 1); i++)
		srcRowRight.at<uchar>(0, i) = srcRow.at<uchar>(0, (rightBoundary + 1 + i));

	cv::minMaxLoc(srcRowRight, &minR, &maxR, &minLocR, &maxLocR);
	if (maxR > 0.25 * max)
	{
		if (maxLocR.x < 4)	rightBoundary += maxLocR.x;
	}

	//Left-bound
	flagL = true;
	while (flagL)
	{
		grayValue = srcRow.at<uchar>(--(maxlCopy.x));
		if (grayValue > (0.25*max)) leftBoundary = maxlCopy.x;
		else						flagL = false; 
	}

	//checking for false boundary on the left
	cv::Mat srcRowLeft = cv::Mat::zeros(1, leftBoundary, CV_8UC1);
	for (int i = 0; i < (leftBoundary); i++)
		srcRowLeft.at<uchar>(0, i) = srcRow.at<uchar>(0, i);

	cv::minMaxLoc(srcRowLeft, &minL, &maxL, &minLocL, &maxLocL);
	if (maxL > 0.25 * max)
	{
		if ((leftBoundary - maxLocL.x) < 4)	leftBoundary = maxLocL.x;
	}

}

void mergesort(int *a, int low, int high)
{
	int mid;
	if (low < high)
	{
		mid = (low + high) / 2;
		mergesort(a, low, mid);
		mergesort(a, mid + 1, high);
		merge(a, low, high, mid);
	}
	return;
}

void merge(int *a, int low, int high, int mid)
{
	int i, j, k, c[50];
	i = low;
	k = low;
	j = mid + 1;
	while (i <= mid && j <= high)
	{
		if (a[i] < a[j])
		{
			c[k] = a[i];
			k++;	i++;
		}
		else
		{
			c[k] = a[j];
			k++;	j++;
		}
	}
	while (i <= mid)
	{
		c[k] = a[i];
		k++;	i++;
	}
	while (j <= high)
	{
		c[k] = a[j];
		k++;	j++;
	}
	for (i = low; i < k; i++)
	{
		a[i] = c[i];
	}
}

int GetMedian(int *SortedArray, int iSize) 
{
	// Middle or average of middle values in the sorted array.
	double dMedian = 0.0;
	if ((iSize % 2) == 0) {
		dMedian = (SortedArray[iSize / 2] + SortedArray[(iSize / 2) - 1]) / 2.0;
	}
	else {
		dMedian = SortedArray[iSize / 2];
	}

	return dMedian;
}