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
		Mat m_lastFrame, m_twoFrame, m_threeFrame, m_fourFrame;
};

class Skeleton {
public:
	Skeleton(Point leftHand, Point rightHand, Point leftLeg, Point rightLeg, Point center, Point head, int width, int height) {
		m_width = width;
		m_height = height;

		m_leftHand = normalize(leftHand);
		m_rightHand = normalize(rightHand);
		m_leftLeg = normalize(leftLeg);
		m_rightLeg = normalize(rightLeg);
		m_center = normalize(center);
		m_head = normalize(head);

		// initialize magnification to prevent confusing problems
		m_magWidth = 1;
		m_magHeight = 1;
	}

	Point2d normalize(Point2d p) {
		return Point2d( (double) p.x / m_width, (double) p.y / m_height);
	}

	void setMagnification(Mat m) {
		m_magWidth = m.cols;
		m_magHeight = m.rows;
	}

	Point2d magnify(Point2d p) {
		return Point2d(p.x * m_magWidth, p.y * m_magHeight);
	}

	// define helper utilities for accessing skeleton
	Point2d leftHand() { return magnify(m_leftHand); };
	Point2d rightHand() { return magnify(m_rightHand); };
	Point2d leftLeg() { return magnify(m_leftLeg); };
	Point2d rightLeg() { return magnify(m_rightLeg); };
	Point2d center() { return magnify(m_center); };
	Point2d head() { return magnify(m_head); };

	// member properties
	Point2d m_leftHand, m_rightHand, m_leftLeg, m_rightLeg, m_center, m_head;
	int m_width, m_height;
	int m_magWidth, m_magHeight;

};

Mat extractUserMask(Mat& delta, double sensitivity);
Mat simplifyUserMask(Mat& mask, Mat& frame, int minimumArclength);
std::vector<Point> getEdgePoints(Mat frame, Mat simplifiedUserMask, int minimumArclength, bool draw, std::vector<std::vector<Point> >& edgePointsList);
std::vector<Skeleton*> skeletonFromEdgePoints(std::vector<Point>& centers, std::vector<std::vector<Point> >& edgePointsList, int width, int height);
void autoCalibrateSensitivity(int* userSensitivity, VideoCapture& stream, int minimumArclength, int interval);

#endif