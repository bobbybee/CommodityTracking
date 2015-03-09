#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

Point averagePoints(std::vector<Point> points) {
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

int main(int argc, char** argv) {
	// initialize camera stream from the built-in webcam
	// and initialize FrameHistory with that stream

	VideoCapture stream(0);
	FrameHistory history(stream);

	// automatically calibrate userSensitivity

	int minimumArclength = 200;
	int userSensitivity = 255;
	int limbGracePeriod = 50;
	int minimumEdgeSpacing = 500;

	autoCalibrateSensitivity(&userSensitivity, stream, history, minimumArclength, 1, limbGracePeriod);

	// settings GUI

	int showOriginal = 0;

	namedWindow("Settings", 1);
	createTrackbar("Minimum Arc Length", "Settings", &minimumArclength, 500);
	createTrackbar("Sensitivity", "Settings", &userSensitivity, 1000);
	createTrackbar("Show original?", "Settings", &showOriginal, 1);

	Skeleton lastSkeleton; // last skeleton required for performing tracking

	for(;;) {
		Mat visualization; // initialize a backdrop for the skeleton
		Mat frame, flipped_frame;

		stream.read(flipped_frame);
		flip(flipped_frame, frame, 1);

		visualization = frame.clone();
		//visualization = Mat::zeros(frame.size(), CV_8UC3);

		Mat delta = history.motion(frame);
		history.append(frame);

		resize(delta, delta, Size(0, 0), 0.1, 0.1);
		resize(frame, frame, Size(0, 0), 0.1, 0.1);
		
		Mat mask = extractUserMask(delta, userSensitivity / 256);
		Mat simplifiedUserMask = simplifyUserMask(mask, frame, minimumArclength);

		std::vector<Point> centers;
		std::vector<std::vector<Point> > edgePointsList;
		centers = getEdgePoints(frame, simplifiedUserMask, minimumArclength, minimumEdgeSpacing, false, edgePointsList);

		// do some visualization

		// each center is a different skeleton / blob
		for(int skeleton = 0; skeleton < centers.size(); ++skeleton) {
			// we may have duplicates even now
			// remember which so we can assemble a skeleton
			vector<Point> leftHands, rightHands, leftLegs, rightLegs, unclassifieds;

			// draw limbs
			for(int limb = 0; limb < edgePointsList[skeleton].size(); ++limb) {			
				if( (edgePointsList[skeleton][limb].y - centers[skeleton].y) > 20) {
					if(edgePointsList[skeleton][limb].x - centers[skeleton].x > 0) {
						rightHands.push_back(edgePointsList[skeleton][limb]);
					} else {
						leftHands.push_back(edgePointsList[skeleton][limb]);
					}
				} else if( abs(edgePointsList[skeleton][limb].x - centers[skeleton].x) > 20 ) {
					if(edgePointsList[skeleton][limb].x - centers[skeleton].x > 0) {
						rightLegs.push_back(edgePointsList[skeleton][limb]);
					} else {
						leftLegs.push_back(edgePointsList[skeleton][limb]);
					}
				} else {
					unclassifieds.push_back(edgePointsList[skeleton][limb]);
					rectangle(visualization, edgePointsList[skeleton][limb] * 10, edgePointsList[skeleton][limb] * 10, Scalar(0, 0, 255), 50);
				}
			}

			Point rightHand = averagePoints(rightHands), leftHand = averagePoints(leftHands),
				  rightLeg = averagePoints(rightLegs), leftLeg = averagePoints(leftLegs);

			rectangle(visualization, leftHand * 10, leftHand * 10, Scalar(0, 0, 255), 50);
			putText(visualization, "L", leftHand * 10, FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));

			rectangle(visualization, rightHand * 10, rightHand * 10, Scalar(0, 0, 255), 50);
			putText(visualization, "R", rightHand * 10, FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));

			rectangle(visualization, leftLeg * 10, leftLeg * 10, Scalar(0, 255, 0), 50);
			putText(visualization, "L", leftLeg * 10, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0));

			rectangle(visualization, rightLeg * 10, rightLeg * 10, Scalar(0, 255, 0), 50);
			putText(visualization, "R", rightLeg * 10, FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0));

			// draw the tracking dot
			rectangle(visualization, centers[skeleton] * 10, centers[skeleton] * 10, Scalar(255, 255, 0), 50);
		}

		imshow("Visualization", visualization);

		if(waitKey(1) == 27) {
			break;
		}
	}

	return 0;
}