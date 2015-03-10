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

	int minimumArclength = 150;
	int userSensitivity = 255;

	autoCalibrateSensitivity(&userSensitivity, stream, minimumArclength, 1);

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
		centers = getEdgePoints(frame, simplifiedUserMask, minimumArclength, true, edgePointsList);

		vector<Skeleton*> skeletons = skeletonFromEdgePoints(centers, edgePointsList, frame.cols, frame.rows);

		// visualize skeletons

		int visWidth = visualization.cols, visHeight = visualization.rows;

		for(int skeletonIndex = 0; skeletonIndex < skeletons.size(); ++skeletonIndex) {
			Skeleton* skeleton = skeletons[skeletonIndex]; // pull skeleton from the vector
			skeleton->setMagnification(visualization); // adjust magnification on the skeleton for visualization properly

			// draw limbs and label according to side of body

			rectangle(visualization, skeleton->leftHand(), skeleton->leftHand(), Scalar(0, 0, 255), 50);
			putText(visualization, "L", skeleton->leftHand(), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));

			rectangle(visualization, skeleton->rightHand(), skeleton->rightHand(), Scalar(0, 0, 255), 50);
			putText(visualization, "R", skeleton->rightHand(), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));

			rectangle(visualization, skeleton->leftLeg(), skeleton->leftLeg(), Scalar(0, 255, 0), 50);
			putText(visualization, "L", skeleton->leftLeg(), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0));

			rectangle(visualization, skeleton->rightLeg(), skeleton->rightLeg(), Scalar(0, 255, 0), 50);
			putText(visualization, "R", skeleton->rightLeg(), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0));

			// draw center
			rectangle(visualization, skeleton->center(), skeleton->center(), Scalar(255, 255, 0), 50);

			// draw head
			rectangle(visualization, skeleton->head(), skeleton->head(), Scalar(255, 0, 255), 100);
		}

		imshow("Visualization", visualization);

		if(waitKey(1) == 27) {
			break;
		}
	}

	return 0;
}