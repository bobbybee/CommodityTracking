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

	int minimumArclength = 30;
	int userSensitivity = 255;
	int limbGracePeriod = 50;

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

		Mat delta = history.motion(frame);
		history.append(frame);

		resize(delta, delta, Size(0, 0), 0.1, 0.1);
		resize(frame, frame, Size(0, 0), 0.1, 0.1);
		
		Mat mask = extractUserMask(delta, userSensitivity / 256);
		Mat user = simplifyUserMask(mask, frame, minimumArclength);

		bitwise_and(frame, user, user);
		resize(user, user, Size(0, 0), 10, 10);
		imshow("User", user);

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