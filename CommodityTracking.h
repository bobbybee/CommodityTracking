#ifndef __COMMODITY_TRACKING_H_
#define __COMMODITY_TRACKING_H_

#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

/**
* Main namespace for CommodityTracking functions
*/

namespace ct {
	/**
	* FrameHistory implements a storage container for multiple cv::Mat objects.
	* It provides a helper utility for motion detection.
	*/

	class FrameHistory {
		public:
			/**
			* FrameHistory constructor from a VideoCapture stream.
			* The first few frames are captured to avoid a race condition.
			*/

			FrameHistory(cv::VideoCapture& stream);

			/**
			* Appends the latest frame from the stream to the history;
			* used to compute motion
			*/

			void append(cv::Mat frame);

			/**
			* Given the latest frame from the video camera stream,
			* a mask will be computed through the difference between this frame
			* and other past frames. The mask is returned as a cv::Mat.
			*/

			cv::Mat motion(cv::Mat frame);

			/**
			* Helper utility to return the last frame added via a call to append
			*/

			cv::Mat getLastFrame();
		private:
			cv::Mat m_lastFrame, m_twoFrame, m_threeFrame, m_fourFrame;
	};

	/**
	* Skeleton is a passive container for a completely tracked skeleton.
	* It does not do any tracking itself, instead relying on cv::Point objects
	* passed in via the constructor.
	* Skeleton objects are constructed by CommodityTracking itself from the getSkeleton method.
	* Skeleton objects expose simple utility methods for accessing joint locations,
	* but it also will optionally magnify these positions (to assist visualization).
	* Magnification is controlled by setMagnification.
	*/

	class Skeleton {
	public:
		/**
		* Skeleton constructor, taking in a list of cv::Point objects
		* corresponding to each tracked joint.
		* This constructor also requires the screen width and height,
		* which is used to normalize the points.
		*/

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

		void smoothFor(Skeleton* old);
		void smoothLimb(cv::Point2d* oldLimb, cv::Point2d* newLimb, int thresh);

		/**
		* Normalizes a given point based on the screen width and height from the constructor
		*/

		cv::Point2d normalize(cv::Point2d p) {
			return cv::Point2d( (double) p.x / m_width, (double) p.y / m_height);
		}

		/**
		* Sets magnification based on the width and height of a given cv::Mat.
		* This overload is useful for performing visualization.
		*/

		void setMagnification(cv::Mat m) {
			m_magWidth = m.cols;
			m_magHeight = m.rows;
		}

		/**
		* Directly sets magnification from parameters.
		* This overload is useful for interopability with existing systems.
		*/

		void setMagnification(int width, int height) {
			m_magWidth = width;
			m_magHeight = height;
		}

		cv::Point2d magnify(cv::Point2d p) {
			return cv::Point2d(p.x * m_magWidth, p.y * m_magHeight);
		}

		// define helper utilities for accessing skeleton

		/**
		* Position of left hand, magnified accordingly.
		*/
		cv::Point2d leftHand() { return magnify(m_leftHand); };

		/**
		* Position of right hand, magnified accordingly.
		*/
		cv::Point2d rightHand() { return magnify(m_rightHand); };

		/**
		* Position of left leg, magnified accordingly.
		*/
		cv::Point2d leftLeg() { return magnify(m_leftLeg); };

		/**
		* Position of right leg, magnified accordingly.
		*/
		cv::Point2d rightLeg() { return magnify(m_rightLeg); };

		/**
		* Position of center of user's body, magnified accordingly.
		*/
		cv::Point2d center() { return magnify(m_center); };

		/**
		* Position of head magnified accordingly.
		*/
		cv::Point2d head() { return magnify(m_head); };

		// member properties
		cv::Point2d m_leftHand, m_rightHand, m_leftLeg, m_rightLeg, m_center, m_head;
		int m_width, m_height;
		int m_magWidth, m_magHeight;

	};

	/**
	* extractUserMask computes a mask of the user given a generic motion mask.
	* Delta is a motion mask, as returned by FrameHistory::motion .
	* Sensitivity is a rather arbitrary setting of how sensitive thresholding should be.
	* Sensitivity should be calibrated by autoCalibrateSensitivity in most cases.
	* extractUserMask returns a black-and-white mask of the user;
	* ideally, performing a bitwise_and on the output of this function
	* and the original frame will produce a perfect image of the user itself.
	* It is often desirable to smooth this output through simplifyUserMask,
	* but simplifyUserMask is much more expensive in terms of CPU then extractUserMask.
	*/
	cv::Mat extractUserMask(cv::Mat& delta, double sensitivity);

	/**
	* simplifyUserMask takes in a mask from extractUserMask
	* and simplifies it to improve accuracy at the cost of CPU time.
	* It performs an intelligent algorithm for finding user contours,
	* but still requires a call to extractUserMask as its input for mask.
	* frame is the original frame from the camera,
	* and minimumArclength is a threshold used for discarding distant or small user-like objects.
	* An appropiate value is 150, but you may need to experiment.
	*/

	cv::Mat simplifyUserMask(cv::Mat& mask, cv::Mat& frame, int minimumArclength);

    /**
     * highUserMask implements watershed segmentation
     * it internally leverages simplifyUserMask and extractUserMaskfor its input,
     * and manages all of the pre/post processing needed for watershed itself
     * highUserMask provides a very high-level, one-size-fits-all solution for user mask extraction,
     * and is therefore used by CommodityTracking itself.
     * additionally, it is useful for general motion-based segmentation algorithms
     */

    cv::Mat highUserMask(cv::Mat& delta, cv::Mat& frame, int minimumArclength, double sensitivity);
	
    /**
	* getEdgePoints returns a list of "interesting" points from a user mask.
	* It returns center points (later fed to Skeleton::center ),
	* and returns edge points by reference as the last parameter.
	* See the source of getSkeleton for usage.
	*/

	std::vector<cv::Point> getEdgePoints(cv::Mat frame, cv::Mat simplifiedUserMask, int minimumArclength, bool draw, std::vector<std::vector<cv::Point> >& edgePointsList);

	/**
	* skeletonFromEdgePoints analyzes the output of getEdgePoints to generate a Skeleton object.
	* Points are classified based on position relative the Skeleton center.
	* Skeleton objects are constructed from skeletonFromEdgePoints.
	* More than one user can theoretically be detected,
	* but in practice only one user works well.
	*/

	std::vector<Skeleton*> skeletonFromEdgePoints(std::vector<Skeleton*> history, std::vector<cv::Point>& centers, std::vector<std::vector<cv::Point> >& edgePointsList, int width, int height);

	/**
	* autoCalibrateSensitivty determines the optimal sensitivity level for extractUserMask.
	* initialUserSensitivity is a pointer to the initial sensitivity for calibration.
	* autoCalibrateSensitivity runs a small portion of the algorithm many times to find the minimum sensitivity.
	* It assumes that there is no motion to start with to find the setting that yields a blank user mask.
	* In other words, you must instruct users to completely exit the camera frame before and during this function call.
	* Else, the algorithm will think that the user itself is noise!
	* interval is the amount to increase the sensitivity each iteration.
	* minimumArclength should be equal to the value actually used for getSkeleton.
	* autoCalibrateSensitivity will return the optimal sensitivity
	*/
	int autoCalibrateSensitivity(int initialUserSensitivity, cv::VideoCapture& stream, int minimumArclength, int interval);

	/**
	* getSkeleton exposes a high-level API for performing skeleton tracking.
	* In most applications, you should use getSkeleton and autoCalibrateSensitivity
	* exclusively and ignore the lower level parts of the algorithm exposed.
	* getSkeleton takes in a stream (from OpenCV), a FrameHistory object,
	* calibrated userSensitivity (see autoCalibrateSensitivity)
	* and a few settings for tweaking performance and accuracy.
	* scaleFactor is some fractional factor that is used to scale each frame,
	* the idea is to minimize the amount of work required to perform tracking
	* at the expense of accuracy.
	*/

	std::vector<Skeleton*> getSkeleton
	(
		std::vector<Skeleton*> oldSkeletons, // last frame's skeletons
		cv::VideoCapture& stream, // webcam stream
		FrameHistory& history, // history for computing delta
		int userSensitivity, // precalibrated value for thresholding
		int minimumArclength, // threshold for discarding noise contours
		double scaleFactor, // (fractional) value for scaling the image (optimization)
	    bool shouldFlip // flip webcam image?
	);
}

#endif
