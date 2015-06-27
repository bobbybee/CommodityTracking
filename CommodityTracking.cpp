#include "CommodityTracking.h"
#include <functional>

using namespace cv;

namespace ct {
	void Skeleton::smoothLimb(cv::Point2d* oldLimb, cv::Point2d* newLimb, int thresh) {
		if( (newLimb->x == 0 && oldLimb->x != 0) || (newLimb->y == 0 && oldLimb->y != 0)) {
			newLimb->x = oldLimb->x;
			newLimb->y = oldLimb->y;
			return;
		}

		if(oldLimb->x > 0 && oldLimb->y > 0) {
			if(abs(newLimb->x - oldLimb->x) > thresh) {
				if(newLimb->x > oldLimb->x)
					newLimb->x -= thresh;
				else
					newLimb->x += thresh;
			}

			if(abs(newLimb->y - oldLimb->y) > thresh) {
				if(newLimb->y > oldLimb->y)
					newLimb->y -= thresh;
				else
					newLimb->y += thresh;
			}
		} else {
			std::cout << "OH NO\a\n";
		}
	}

	void Skeleton::smoothFor(Skeleton* old) {
		smoothLimb(&old->m_leftHand, &m_leftHand, 4);
		smoothLimb(&old->m_rightHand, &m_rightHand, 4);
		smoothLimb(&old->m_leftLeg, &m_leftLeg, 2);
		smoothLimb(&old->m_rightLeg, &m_rightLeg, 2);
		smoothLimb(&old->m_center, &m_center, 3);
		smoothLimb(&old->m_head, &m_head, 2);
	}

	FrameHistory::FrameHistory(VideoCapture& stream) {
		stream.read(m_lastFrame); // fixes a race condition in the first few frames
		stream.read(m_threeFrame); // fixes a race condition in the first few frames
		stream.read(m_twoFrame);
		stream.read(m_fourFrame);
	}

	void FrameHistory::append(Mat frame) {
		m_fourFrame = m_threeFrame;
		m_threeFrame = m_twoFrame;
		m_twoFrame = m_lastFrame;
		m_lastFrame = frame;
	}

	Mat FrameHistory::motion(Mat frame) {
		Mat out1, out2, out3, out4, delta;
		absdiff(m_twoFrame, frame, out1);
		absdiff(m_lastFrame, frame, out2);
		absdiff(m_threeFrame, frame, out3);
		absdiff(m_fourFrame, frame, out4);

		bitwise_or(out2, out3, delta);
		bitwise_or(delta, out1, delta);
		bitwise_or(delta, out4, delta);

        /*Mat out1, out2, delta;
        absdiff(m_twoFrame, frame, out1);
        absdiff(m_lastFrame, frame, out2);

        bitwise_or(out1, out2, delta);*/

		return delta;
	}

	Mat FrameHistory::getLastFrame() {
		return m_lastFrame;
	}

	// the black-and-white user mask is found from
	// the motion extracted image through a series
	// of various blurs and corresponding thresholds

	cv::Mat extractUserMask(cv::Mat& delta, double sensitivity) {
		cvtColor(delta, delta, CV_BGR2GRAY);

		blur(delta, delta, Size(4, 4), Point(-1, -1));
		threshold(delta, delta, sensitivity * 20, 255, THRESH_BINARY);

		cvtColor(delta, delta, CV_GRAY2BGR);

		return delta;
	}

	// NOTE: may trash original mask. clone if preservation is needed
	cv::Mat simplifyUserMask(cv::Mat& mask, cv::Mat& frame, int minimumArclength) {
		// prepare for Canny + contour detection
		cvtColor(mask, mask, CV_BGR2GRAY);

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		// extract edges using Canny
		Mat edges;
		Canny(mask, edges, 20, 20 * 3, 3);

		cvtColor(edges, edges, CV_GRAY2BGR);

		// find contours, simplify and draw large contours to contourOut
		Mat contourOut = Mat::zeros(frame.size(), CV_8UC3);
		findContours(mask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

		for(int i = 0; i < contours.size(); ++i) {
			double t_arcLength = arcLength(Mat(contours[i]), true);
			approxPolyDP(contours[i], contours[i], t_arcLength * 0.005, true);

			if(t_arcLength > minimumArclength) { // remove tiny contours.. don't waste your time
				drawContours(contourOut, contours, i, Scalar(255, 255, 255), CV_FILLED, 8, hierarchy, 0, Point()); // CV_FILLED produces filled contours to act as a mask
			}
		}

		return contourOut;
	}

    cv::Mat highUserMask(cv::Mat& delta, cv::Mat& frame, int minimumArclength, double sensitivity) {
        // extract the user mask using delta blur-threshold
        
        Mat mask = extractUserMask(delta, sensitivity);

        // dilate the image for the watershed
        // the image will be eroded an equal amount later,
        // so the net erosion / dilation is still zero, kind of
        // but this transform has some useful properties for performing watershed segmentation

        int erosionAmount = 10;
        Mat el = getStructuringElement(MORPH_RECT, Size(2 * erosionAmount + 1, 2 * erosionAmount + 1), Point(erosionAmount, erosionAmount));

        Mat thin;
        dilate(mask, thin, el);

        cvtColor(thin, thin, CV_BGR2GRAY);


        // provide watershed a background color by running floodFill on the markers
        
        floodFill(thin, Point(0, 0), CV_RGB(127,127,127));
        floodFill(thin, Point(0, thin.rows - 1), CV_RGB(127,127,127));
        floodFill(thin, Point(thin.cols - 1, 0), CV_RGB(127,127,127));
        floodFill(thin, Point(thin.cols - 1, thin.rows - 1), CV_RGB(127,127,127));

        // run the watershed transform itself
        // watershed operates in the esoteric CV_32S matrix type (signed 32-bit integers)
        // this makes sense for watershed; not so much for our image processing
        // as a result, we simply pad watershed with convertTo calls  (for our sanity)
        // it is also necessary to convert back to the RGB color space

        Mat markers;
        
        thin.convertTo(markers, CV_32S);
        watershed(frame, markers);
        markers.convertTo(markers, CV_8U);
        
        cvtColor(markers, markers, CV_GRAY2BGR);
       
        Mat test;

        // completely cancel out the background
        // remember, the background at the moment is actually #7F7F7F, a perfect gray
        // by thresholding the image with #FEFEFE, the grey background will become black,
        // and as the user was already white,
        // this threshold creates a perfect (slightly noisy) user mask
        
        threshold(markers, markers, 254, 255, THRESH_BINARY);

        // noise reduction is performed by finding the raw contours of the image,
        // and redrawing them depending on the size
        // unfortunately, this is an expensive call, but it is well worth it for the results!
        
        Mat pureMask = simplifyUserMask(markers, frame, minimumArclength);

        // finally, the original dilate call causes this "fatness" illusion on the mask
        // by eroding the mask by the same amount of the dilation, the mask becomes much more tight
        
        erode(pureMask, pureMask, el);

        // let the calling application / CT function work with the mask as desired
        
        return pureMask;
    }

	std::vector<cv::Point> getEdgePoints(cv::Mat frame, cv::Mat simplifiedUserMask, int minimumArclength, bool draw, std::vector<std::vector<cv::Point> >& edgePointsList) {
		Mat edges;
		Canny(simplifiedUserMask, edges, 300, 300 * 3, 3);

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		Mat contourOut;

		if(draw)
			contourOut = Mat::zeros(frame.size(), CV_8UC3);

		findContours(edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);

		vector<Point> centers;

		for(int i = 0; i < contours.size(); ++i) {
			double t_arcLength = arcLength(Mat(contours[i]), true);
			//approxPolyDP(contours[i], contours[i], t_arcLength * 0.015, true);

			if(t_arcLength > minimumArclength) { // remove tiny contours.. don't waste your time
				if(draw)
					drawContours(contourOut, contours, i, Scalar(255, 255, 255), 1, 8, hierarchy, 0, Point()); // CV_FILLED produces filled contours to act as a mask

				int averageX = 0, averageY = 0, n = 0;
				Point topLeft(edges.rows, edges.cols), bottomRight(0, 0);
				vector<Point> edgePoints;

				for(int j = 0; j < contours[i].size(); ++j) {
					++n;
					averageX += contours[i][j].x;
					averageY += contours[i][j].y;

					if(contours[i][j].x < topLeft.x)
						topLeft.x = contours[i][j].x;
					if(contours[i][j].y < topLeft.y )
						topLeft.y = contours[i][j].y;

					if(contours[i][j].x > bottomRight.x)
						bottomRight.x = contours[i][j].x;
					if(contours[i][j].y > bottomRight.y)
						bottomRight.y = contours[i][j].y;
				}

				for(int j = 0; j < contours[i].size(); ++j) {
					if( (contours[i][j].x - topLeft.x < 4) || (bottomRight.x - contours[i][j].x < 4) ||
						(contours[i][j].y - topLeft.y < 4) || (bottomRight.y - contours[i][j].y < 4)) {

						edgePoints.push_back(contours[i][j]);
					}
				}

				averageX /= n;
				averageY /= n;

				centers.push_back(Point(averageX, averageY));

				if(draw)
					rectangle(contourOut, Point(averageX, averageY), Point(averageX, averageY), Scalar(255, 0, 0), 5);

				if(draw) {
					for(int i = 0; i < edgePoints.size(); ++i) {
						rectangle(contourOut, edgePoints[i], edgePoints[i], Scalar(0, 255, 0), 5);
					}
				}

				edgePointsList.push_back(edgePoints);

			}
		}

		if(draw) {
			resize(contourOut, contourOut, Size(0, 0), 3, 3);
			imshow("Edge Point Sketch", contourOut);
		}

		return centers;
	}

	// computes the mean of a vector of Point's

	static Point averagePoints(std::vector<Point> points) {
		if(points.size()) {
			int sumX = 0, sumY = 0;

			for(int i = 0; i < points.size(); ++i) {
				sumX += points[i].x;
				sumY += points[i].y;
			}

			return Point(sumX / points.size(), sumY / points.size());
		} else {
			return Point(0, 0);
		}
	}

    // iterates through the haystack, runs the scoring function, and returns the highest scorer
    
    static Point findLimb(std::vector<Point>& haystack, cv::Point center, std::function<double(cv::Point, cv::Point)> scorer ) {
        if(haystack.size() == 0) return Point(0, 0); // equivalent of returning null
        
        double maxScore = 0; // maintain a running max
        int maxScoreIndex = 0; 
        
        for(int i = 0; i < haystack.size(); ++i) {
            double score = scorer(center, haystack[i]);

            if(score > maxScore) {
                maxScore = score;
                maxScoreIndex = i;
            }
        }

        return haystack[maxScoreIndex];
    }

    // scoring functions to be used with findLimb
    // implement some fancy hand chosen equations :)
    
    static double scoreHead(cv::Point center, cv::Point point) {
        double alpha = 1.5;
        
        return (
                alpha * (
                    (center.y - point.y)
                ) / (center.y)
               ) - (
                  abs(center.x - point.x)
                 / center.x
                 );
    }

    static double scoreLeftHand(cv::Point center, cv::Point point) {
        double alpha = 1.5;
        return (alpha * (center.x - point.x)) + (center.y - point.y);
    }

    static double scoreRightHand(cv::Point center, cv::Point point) {
        double alpha = 1.5;
        return (alpha * (point.x - center.x)) + (center.y - point.y);
    }

    static double scoreLeftLeg(cv::Point center, cv::Point point) {
        double alpha = 1.5;
        return (alpha * (center.x - point.x)) + (point.y - center.y);
    }

    static double scoreRightLeg(cv::Point center, cv::Point point) {
        double alpha = 1.5;
        return (alpha * (point.x - center.x)) + (point.y - center.y);
    }

	std::vector<Skeleton*> skeletonFromEdgePoints(std::vector<Skeleton*> history, std::vector<cv::Point>& centers, std::vector<std::vector<cv::Point> >& edgePointsList, int width, int height) {
		vector<Skeleton*> skeletons;

		// each center corresponds to a skeleton { mostly }
		for(int skeleton = 0; skeleton < centers.size(); ++skeleton) {
            // the actual limb calculations are done outside of this function

            Point head, leftHand, rightHand, leftLeg, rightLeg;

            // we classify limbs based on their position relative to the center
            
            // naively, we could say:
            // heads have a much higher Y coordinate but identical X,
            // hands have a somewhat higher Y coordinate but large absolute X
            // however, it is more effective to somehow score the limbs
            // at a high-level, we rate each limb as a candidate for each limb type,
            // and choose the highest ranking limb for each category to be the actual limb
           
            // we'll pass these off to other functions 
   
            head = findLimb(edgePointsList[skeleton], centers[skeleton], scoreHead);
            leftHand = findLimb(edgePointsList[skeleton], centers[skeleton], scoreLeftHand);
            rightHand = findLimb(edgePointsList[skeleton], centers[skeleton], scoreRightHand);
            leftLeg = findLimb(edgePointsList[skeleton], centers[skeleton], scoreLeftLeg);
            rightLeg = findLimb(edgePointsList[skeleton], centers[skeleton], scoreRightLeg);

			Skeleton* skel = new Skeleton(leftHand, rightHand, leftLeg, rightLeg, centers[skeleton], head, width, height);

			if(skeleton < history.size()) {
				Skeleton* old = history[skeleton];
				skel->smoothFor(old);
			}

			// populate the skeleton object
			skeletons.push_back(skel);
		}

		return skeletons;
	}

	// auto-calibration works by running a trivial part of the actual code;
	// it's main body is derived from the demo itself
	// however, it is constantly changing its sensitivity parameter
	// in order to minimize noise without compromising flexibility

	int autoCalibrateSensitivity(int initialUserSensitivity, cv::VideoCapture& stream, int minimumArclength, int interval) {
		FrameHistory history(stream);
		int sensitivity = initialUserSensitivity;

		while(sensitivity < 1000) {
			Mat frame;
			stream.read(frame);

			Mat delta = history.motion(frame);
			history.append(frame);

			Mat mask = extractUserMask(delta, sensitivity / 256);
			Mat simplifiedUserMask = simplifyUserMask(mask, frame, minimumArclength);

			sensitivity += interval;

			cvtColor(simplifiedUserMask, simplifiedUserMask, CV_BGR2GRAY);

			if(countNonZero(simplifiedUserMask) == 0) {
				// optimal calibration found, but make it a bit more sensitive than needed
				// noise fluctuates massively, after all

				sensitivity += interval * 2;
				break;
			}
		}

		return sensitivity;
	}

	// unless you have some special case requiring internal functions,
	// use getSkeleton in your application's main loop

	std::vector<Skeleton*> getSkeleton
	(
		std::vector<Skeleton*> oldSkeletons,
		cv::VideoCapture& stream, // webcam stream
		FrameHistory& history, // history for computing delta
		int userSensitivity, // precalibrated value for thresholding
		int minimumArclength, // threshold for discarding noise contours
		double scaleFactor, // (fractional) value for scaling the image (optimization)
	    bool shouldFlip // flip webcam image?
	) {
		// read a frame and optionally flip it

		Mat frame, flipped_frame;
		stream.read(flipped_frame);

		if(shouldFlip) {
			flip(flipped_frame, frame, 1);
		} else {
			frame = flipped_frame; // flipping was not requested
		}

		// get motion delta
		Mat delta = history.motion(frame);
		history.append(frame);

		Mat outMask;

		imshow("Webcam", frame);

		// resize down image to speed up calculations
		resize(frame, frame, Size(0, 0), scaleFactor, scaleFactor);
		resize(delta, delta, Size(0, 0), scaleFactor, scaleFactor);

		//resize(delta, outMask, Size(0, 0), 0.5 / scaleFactor, 0.5 / scaleFactor);
		//imshow("Delta", outMask);

		//resize(mask, outMask, Size(0, 0), 0.5 / scaleFactor, 0.5 / scaleFactor);
		//imshow("Mask", outMask);

		//Mat simplifiedUserMask = simplifyUserMask(mask, frame, minimumArclength);

		//resize(simplifiedUserMask, outMask, Size(0, 0), 0.5 / scaleFactor, 0.5 / scaleFactor);
		//imshow("Simplified Mask", outMask);

        // compute mask using Collins et al + delta blur-threshold + watershed + contour discrimination
        
        Mat mask = highUserMask(delta, frame, minimumArclength, userSensitivity / 256);

        imshow("High-level mask", mask);

		std::vector<Point> centers;
		std::vector<std::vector<Point> > edgePointsList;
		centers = getEdgePoints(frame, mask, minimumArclength, true, edgePointsList);

		return skeletonFromEdgePoints(oldSkeletons, centers, edgePointsList, frame.cols, frame.rows);
	}
};
