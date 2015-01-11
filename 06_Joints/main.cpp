#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include "CommodityTracking.h"

int main(int argc, char** argv) {
	VideoCapture stream(0);

	int minimumArclength = 150;
	int userSensitivity = 255;

	int showOriginal = 0, showSkeleton = 0, _flip = 0;

	namedWindow("Settings", 1);
	createTrackbar("Minimum Arc Length", "Settings", &minimumArclength, 500);
	createTrackbar("Sensitivity", "Settings", &userSensitivity, 1000);

	createTrackbar("Show original?", "Settings", &showOriginal, 1);
	createTrackbar("Show skeleton?", "Settings", &showSkeleton, 1);
	createTrackbar("Flip?", "Settings", &_flip, 1);

	FrameHistory history(stream);

	for(;;) {
		Skeleton skeleton = getSkeleton(stream, history, _flip, minimumArclength, userSensitivity);
		
		Mat visualization;

		if(showOriginal) {
			visualization = history.getLastFrame().clone();
		} else {
			visualization = Mat::zeros(history.getLastFrame().size(), CV_8UC3);
		}
		

		skeleton.visualize(visualization);
		imshow("Visualization", visualization);

		if(waitKey(1) == 27) {
			break;
		}
	}

	return 0;
}