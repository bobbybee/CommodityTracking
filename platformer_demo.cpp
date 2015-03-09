#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

int main(int argc, char** argv) {
	std::cout << "The platformer demo has been temporarily disabled while the Skeleton object is reworked" << endl;
	/*VideoCapture stream(0);
	FrameHistory history(stream);

	int minimumArclength = 150, userSensitivity = 255, limbGracePeriod = 50;
	autoCalibrateSensitivity(&userSensitivity, stream, history, minimumArclength, 1, limbGracePeriod);

	Skeleton lastSkeleton;

	Mat cvMan = imread("cvman.png");

	for(;;) {
		Skeleton skeleton = getSkeleton(stream, history, lastSkeleton, true, minimumArclength, userSensitivity, limbGracePeriod);
		lastSkeleton = skeleton;

		Mat output = Mat::ones(512, 512, CV_8UC3);
		output = Scalar(255, 255, 255);

		Point2d rightHand = skeleton.rightHand() * 512;
		Point2d leftHand = skeleton.leftHand() * 512;
		Point2d center = skeleton.center() * 512;
		Point2d head = skeleton.head() * 512;

		Rect roi_rect = Rect(center.x < 448 && center.x > 0? center.x : 0, head.y < 64 ? 256 : 448, 64, 64);
		Mat roi = output(roi_rect);
		cvMan.copyTo(roi);

		imshow("Output", output);

		Mat vis = history.getLastFrame().clone();
		resize(vis, vis, Size(0, 0), 0.1, 0.1);
		skeleton.visualize(vis);
		imshow("Frame (+skeleton)", vis);

		if(waitKey(1) == 27) {
			break;
		}

	}*/
}