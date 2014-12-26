#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;

static inline double labDelta(Vec3b ref, uint8_t* row, int index) {
	Vec3b oldPixel = row[index];

	// check if the colour changed more than a threshold
	double delta0 = ref[0] - oldPixel[0];
	double delta1 = ref[1] - oldPixel[1];
	double delta2 = ref[2] - oldPixel[2];
	
	return (delta0 * delta0) + (delta1 * delta1) + (delta2 * delta2);
}

int main(int argc, char** argv) {
	VideoCapture stream(0);

	Mat lastFrame;
	stream.read(lastFrame); // fixes a race condition

	for(;;) {
		Mat frame;
		stream.read(frame);
		
		Mat delta = frame.clone();
		cvtColor(delta, delta, CV_BGR2Lab);

		int i = delta.cols, j = delta.rows;
		uint8_t* row;

		while(j--) {
			row = delta.ptr<uint8_t>(j);

			while(i--) {
				Vec3b ref = row[i];
				double totalDelta = labDelta(ref, row, i - 1) + 
									labDelta(ref, row, i + 1) + 
									labDelta(ref, row, i);

				bool hasChanged = totalDelta > 20000;

				row[ (i * 3) ] = hasChanged * 255;
				row[ (i * 3) + 1] = hasChanged * 255;
				row[ (i * 3) + 2] = hasChanged * 255;

			}

			i = delta.cols;
		}

		for(int i = 1; i < 25; i = i + 2) {
			blur(delta, delta, Size(i, i), Point(-1, -1));
		}

		threshold(delta, delta, 50, 255, THRESH_BINARY);

		Canny(delta, delta, 20, 20 * 3, 3);
		
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
			
			double len = arcLength(Mat(contours[i]), true);

			if(len > maxArclength) {
				maxArclength = len;
				contourNumForMax = i;
			}
		}

		for (int i = 0; i < contours.size(); ++i) {
			if(i == contourNumForMax)
				drawContours(contourDrawing, contours, i, userColour, 3);	
			else
				drawContours(contourDrawing, contours, i, normalColour, 3);
		}

		imshow("Contours", contourDrawing);
		
		if(waitKey(10) == 27) {
			break;
		
		}
		lastFrame = frame;
	}

	return 0;
}