#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;

int main(int argc, char** argv) {
	VideoCapture stream(0);

	Mat lastFrame;
	Mat twoFrame;

	stream.read(lastFrame); // fixes a race condition
	stream.read(twoFrame);

	for(;;) {
		Mat frame;
		stream.read(frame);
		
		Mat out1, out2, delta;

		absdiff(twoFrame, frame, out1);
		absdiff(lastFrame, frame, out2);
		bitwise_and(out1, out2, delta);

		threshold(delta, delta, 60, 255, THRESH_BINARY);
		blur(delta, delta, Size(21, 21), Point(-1, -1));

		cvtColor(delta, delta, CV_BGR2GRAY);

		Canny(delta, delta, 40, 40 * 3, 3);

		// obtain contours
		std::vector<std::vector<Point> > contours;
		std::vector<Vec4i> hierarchy;
		std::vector<Point> approxShape;

		findContours(delta.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		Mat contourDrawing = Mat::zeros(delta.size(), CV_8UC3);

		double maxArclength = 0;
		int contourNumForMax = 0;
		
		Scalar normalColour = Scalar(255, 0, 0);
		Scalar userColour = Scalar(0, 0, 255);

		for(int i = 0; i < contours.size(); ++i) {
			approxPolyDP(contours[i], approxShape, arcLength(Mat(contours[i]), true) * 0.04, true);
			drawContours(contourDrawing, contours, i, normalColour, 3);
		}

		Mat flipped;
		flip(contourDrawing, flipped, 1);

		imshow("Contours", flipped);
		
		if(waitKey(10) == 27) {
			break;
		}

		twoFrame = lastFrame;
		lastFrame = frame;
	}

	return 0;
}