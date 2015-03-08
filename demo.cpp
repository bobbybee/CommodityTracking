#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

int main(int argc, char** argv) {
	// initialize camera stream from the built-in webcam
	// and initialize FrameHistory with that stream

	VideoCapture stream(0);
	FrameHistory history(stream);

	// automatically calibrate userSensitivity

	int minimumArclength = 180;
	int userSensitivity = 255;
	int limbGracePeriod = 50;
	int minimumEdgeSpacing = 300;

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

		Mat delta = history.motion(frame);
		history.append(frame);

		resize(delta, delta, Size(0, 0), 0.1, 0.1);
		resize(frame, frame, Size(0, 0), 0.1, 0.1);
		
		Mat mask = extractUserMask(delta, userSensitivity / 256);
		Mat simplifiedUserMask = simplifyUserMask(mask, frame, minimumArclength);

		std::vector<Point> centers;
		std::vector<std::vector<Point> > edgePointsList;
		centers = getEdgePoints(frame, simplifiedUserMask, minimumArclength, minimumEdgeSpacing, true, edgePointsList);

		// do some visualization

		// each center is a different skeleton / blob
		for(int skeleton = 0; skeleton < centers.size(); ++skeleton) {
			// draw limbs
			for(int limb = 0; limb < edgePointsList[skeleton].size(); ++limb) {
				line(visualization, centers[skeleton] * 10, edgePointsList[skeleton][limb] * 10, Scalar(0, 255, 0), 10);
			}

			// draw the tracking dot
			rectangle(visualization, centers[skeleton] * 10, centers[skeleton] * 10, Scalar(0, 0, 255), 50);
		}

		imshow("Visualization", visualization);

		/*
		if(showOriginal) {
			visualization = history.getLastFrame().clone();
		} else {
			visualization = Mat::zeros(history.getLastFrame().size(), CV_8UC3);
		}
		

		lastSkeleton.visualize(visualization); // use Skeleton::visualize for the demo
												// in a real app, we would access properties such as
												// skeleton.leftMostAbove (left hand position)
		imshow("Visualization", visualization);
*/
		if(waitKey(1) == 27) {
			break;
		}

		// get the Skeleton object
		// takes VideoStream, FrameHistory, Skeleton lastSkeleton,
		// whether to flip the frame, and some sensitivity settings

		//Skeleton skeleton = getSkeleton(stream, history, lastSkeleton, false, minimumArclength, userSensitivity, limbGracePeriod);
		//lastSkeleton = skeleton;
	}
	return 0;
}