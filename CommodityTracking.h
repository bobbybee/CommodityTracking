#ifndef __COMMODITY_TRACKING_H_
#define __COMMODITY_TRACKING_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

namespace ct {
	class FrameHistory {
		public:
			FrameHistory(cv::VideoCapture& stream);
			void append(cv::Mat frame);
			cv::Mat motion(cv::Mat frame);
			cv::Mat getLastFrame();
		private:
			cv::Mat m_lastFrame, m_twoFrame, m_threeFrame, m_fourFrame;
	};

	class Skeleton {
	public:
		Skeleton(cv::Point leftHand, cv::Point rightHand, cv::Point leftLeg, cv::Point rightLeg, cv::Point center, cv::Point head, int width, int height) {
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

		cv::Point2d normalize(cv::Point2d p) {
			return cv::Point2d( (double) p.x / m_width, (double) p.y / m_height);
		}

		void setMagnification(cv::Mat m) {
			m_magWidth = m.cols;
			m_magHeight = m.rows;
		}

		void setMagnification(int width, int height) {
			m_magWidth = width;
			m_magHeight = height;
		}

		cv::Point2d magnify(cv::Point2d p) {
			return cv::Point2d(p.x * m_magWidth, p.y * m_magHeight);
		}

		// define helper utilities for accessing skeleton
		cv::Point2d leftHand() { return magnify(m_leftHand); };
		cv::Point2d rightHand() { return magnify(m_rightHand); };
		cv::Point2d leftLeg() { return magnify(m_leftLeg); };
		cv::Point2d rightLeg() { return magnify(m_rightLeg); };
		cv::Point2d center() { return magnify(m_center); };
		cv::Point2d head() { return magnify(m_head); };

		// member properties
		cv::Point2d m_leftHand, m_rightHand, m_leftLeg, m_rightLeg, m_center, m_head;
		int m_width, m_height;
		int m_magWidth, m_magHeight;

	};

	cv::Mat extractUserMask(cv::Mat& delta, double sensitivity);
	cv::Mat simplifyUserMask(cv::Mat& mask, cv::Mat& frame, int minimumArclength);
	std::vector<cv::Point> getEdgePoints(cv::Mat frame, cv::Mat simplifiedUserMask, int minimumArclength, bool draw, std::vector<std::vector<cv::Point> >& edgePointsList);
	std::vector<Skeleton*> skeletonFromEdgePoints(std::vector<cv::Point>& centers, std::vector<std::vector<cv::Point> >& edgePointsList, int width, int height);
	void autoCalibrateSensitivity(int* userSensitivity, cv::VideoCapture& stream, int minimumArclength, int interval);

	std::vector<Skeleton*> getSkeleton
	(
		cv::VideoCapture& stream, // webcam stream
		FrameHistory& history, // history for computing delta
		int userSensitivity, // precalibrated value for thresholding
		int minimumArclength, // threshold for discarding noise contours
		double scaleFactor, // (fractional) value for scaling the image (optimization)
	    bool shouldFlip // flip webcam image?
	);
}

#endif