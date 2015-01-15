#ifndef __COMMODITY_TRACKING_H_
#define __COMMODITY_TRACKING_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;
using namespace std;

class FrameHistory {
	public:
		FrameHistory(VideoCapture& stream);
		void append(Mat frame);
		Mat motion(Mat frame);
		Mat getLastFrame();
	private:
		Mat m_lastFrame, m_twoFrame;
};

class Skeleton {
public:
	Skeleton() {
		// initialize with an empty skeleton

		center_of_rect = Point(0, 0);
		rightMostAbove = Point(0, 0);
		rightMostBelow = Point(0, 0);
		leftMostAbove = Point(0, 0);
		leftMostBelow = Point(0, 0);
		topMost = Point(0, 0);
	}

	Point center_of_rect, rightMostAbove, rightMostBelow, leftMostAbove, leftMostBelow, topMost;

	void visualize(Mat visualization) {
		line(visualization, topMost, center_of_rect, Scalar(0, 255, 0), 20);
		line(visualization, rightMostAbove, center_of_rect, Scalar(0, 255, 0), 20);
		line(visualization, leftMostAbove, center_of_rect, Scalar(0, 255, 0), 20);
		line(visualization, rightMostBelow, center_of_rect, Scalar(0, 255, 0), 20);
		line(visualization, leftMostBelow, center_of_rect, Scalar(0, 255, 0), 20);
	}
};

Mat extractUserMask(Mat& delta, double sensitivity);
Skeleton getSkeleton(VideoCapture& stream, FrameHistory& history, Skeleton last, bool _flip, int minimumArclength, int userSensitivity, int limbGracePeriod);
void autoCalibrateSensitivity(int* userSensitivity, VideoCapture& stream, FrameHistory& history, int minimumArclength, int interval, int limbGracePeriod);

#endif