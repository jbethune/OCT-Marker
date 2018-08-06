#ifndef BSCANSEGALGORITHM_H
#define BSCANSEGALGORITHM_H

#include "configdata.h"

#include <octdata/datastruct/segmentationlines.h>

namespace cv
{
	class Mat;
}

namespace OctData
{
	class BScan;
}

class BScanSegmentation;

class BScanSegAlgorithm
{
public:
	typedef BScanSegmentationMarker::internalMatType PaintType;

	static void initFromSegline(const OctData::BScan& bscan, cv::Mat& segMat, OctData::Segmentationlines::SegmentlineType type);
	static void initFromThresholdDirection(const cv::Mat& image, cv::Mat& segMat, const BScanSegmentationMarker::ThresholdDirectionData& data, PaintType val0, PaintType val1);
	static PaintType getThresholdGrayValue(const cv::Mat& image, const BScanSegmentationMarker::ThresholdData& data);
	static void initFromThreshold(const cv::Mat& image, cv::Mat& segMat, const BScanSegmentationMarker::ThresholdData& data, PaintType val0, PaintType val1);
	static void openClose(cv::Mat& dest, cv::Mat* src = nullptr); /// if no src given, then dest is used as src
	static bool removeUnconectedAreas(cv::Mat& image);
	static bool extendLeftRightSpace(cv::Mat& image, int limit = 40);
};

#endif // BSCANSEGALGORITHM_H
