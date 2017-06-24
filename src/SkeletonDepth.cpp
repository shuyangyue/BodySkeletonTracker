#include "SkeletonDepth.h"
#include <stdio.h>
#include <GL/freeglut.h>
#include <opencv2/imgproc.hpp>

#include "OniSampleUtilities.h"

using namespace cv;

SkeletonDepth::SkeletonDepth(int width, int height, int subSample) {
	this->width = width;
	this->height = height;
	this->subSample = subSample;

	lineSize = width/subSample;

	diff_w = 0;
	diff_h = 0;
	diff = 180 + diff_w + diff_h;
	dist = 0;
	maxDiff = 240;
	max = 0;
}



void SkeletonDepth::paintDepthCopy(openni::RGB888Pixel*m_pTexMap, openni::VideoFrameRef depthFrame, cv::Mat * binarized) {
	calculateHistogram(m_pDepthHist, MAX_DEPTH, depthFrame);
	float factor[3] = {1, 1, 1};
	const float *f;

	const openni::DepthPixel* pDepthRow = (const openni::DepthPixel*)depthFrame.getData();
	openni::RGB888Pixel* pTexRow = m_pTexMap + depthFrame.getCropOriginY() * width;
	int rowSize = depthFrame.getStrideInBytes() / sizeof(openni::DepthPixel);

	max = 0;

	for (int y = 0; y < height; ++y)
	{
		const openni::DepthPixel* pDepth = pDepthRow;
		openni::RGB888Pixel* pTex = pTexRow + depthFrame.getCropOriginX();
		setDiffH(abs(closest.Y-y)/5); // diferenca (height) do ponto atual para o ponto mais proximo
		for (int x = 0; x < width; ++x, ++pDepth, ++pTex)
		{
			if (*pDepth != 0)
			{
				setDiffW(abs(closest.X-x)/5); // diferenca (width) do ponto atual para o ponto mais proximo
				f = paintDepthCopyPixel(pDepth, x, y, binarized);
				if (f)
					memcpy(factor, f, sizeof(float)*3);

				int nHistValue = m_pDepthHist[*pDepth];
				pTex->r = nHistValue*factor[0];
				pTex->g = nHistValue*factor[1];
				pTex->b = nHistValue*factor[2];
				factor[0] = factor[1] = factor[2] = 1;
			}
		}
		pDepthRow += rowSize;
		pTexRow += width;
	}

	if (max>0)
		maxDiff = max;
}





const float * SkeletonDepth::paintDepthCopyPixel(const openni::DepthPixel* pDepth, int x, int y, cv::Mat * binarized) {
	//diff = 180 + diff_w + diff_h;
	diff = 600 + diff_w + diff_h;

	if (*pDepth == closest.Z)
	{
		rgb[0] = 0; // R
		rgb[1] = 1; // G
		rgb[2] = 0; // B

		return rgb;
	}
	else if (*pDepth >= closest.Z && *pDepth <= closest.Z+diff)
	{
		if (y%subSample==0 && x%subSample==0)
			binarized->data[(y/subSample)*lineSize+x/subSample]=255;

		if (diff>max) {
			max = diff;
		}

		dist = ((*pDepth) - closest.Z)/maxDiff;
		if (dist<1) {
			rgb[2] = 1-dist; // R
			rgb[0] = dist;   // B
		}
		else {
			rgb[2] = 0; // R
			rgb[0] = 1; // B
		}
		rgb[1] = 0;      // G

		return rgb;
	}


	return 0;
}


void SkeletonDepth::prepareAnalisa(const closest_point::IntPoint3D& closest) {
	this->closest = closest;
}

void SkeletonDepth::setDiffH(int d) {
	diff_h = d;
}

void SkeletonDepth::setDiffW(int d) {
	diff_w = d;
}