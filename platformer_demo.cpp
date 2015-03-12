#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

using namespace cv;
using namespace ct;

int main(int argc, char** argv) {
	VideoCapture stream(0);
	FrameHistory history(stream);

	int minimumArclength = 200;

	int userSensitivity = autoCalibrateSensitivity(256, stream, minimumArclength, 1);

	Mat cvMan = imread("cvman.png");

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
		centers = getEdgePoints(frame, simplifiedUserMask, minimumArclength, false, edgePointsList);

		vector<Skeleton*> skeletons = skeletonFromEdgePoints(centers, edgePointsList, frame.cols, frame.rows);

		Mat output = Mat::ones(512, 512, CV_8UC3);
		output = Scalar(255, 255, 255);

		for(int i = 0; i < skeletons.size(); ++i) {
			Skeleton* skeleton = skeletons[i];
			skeleton->setMagnification(output);

			Rect roi_rect = Rect(skeleton->head().x < 448 && skeleton->head().x > 0 ? skeleton->head().x : 0,
			 					skeleton->head().y < 16 ? 256 : 448,
			 					64, 64);

			Mat roi = output(roi_rect);
			cvMan.copyTo(roi);
		}



		imshow("Output", output);

		/*Mat vis = history.getLastFrame().clone();
		resize(vis, vis, Size(0, 0), 0.1, 0.1);
		skeleton.visualize(vis);
		imshow("Frame (+skeleton)", vis);*/

		if(waitKey(1) == 27) {
			break;
		}

	}
}
