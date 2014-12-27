#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;

static inline int labDelta(Vec3b ref, uint8_t* row, int index) {
	Vec3b oldPixel = row[index];

	// check if the colour changed more than a threshold
	int delta0 = ref[0] - oldPixel[0];
	int delta1 = ref[1] - oldPixel[1];
	int delta2 = ref[2] - oldPixel[2];
	
	return (delta0 * delta0) + (delta1 * delta1) + (delta2 * delta2);
}

int main(int argc, char** argv) {
	VideoCapture stream(0);

	Mat lastFrame;
	Mat twoFrame;

	stream.read(lastFrame); // fixes a race condition
	stream.read(twoFrame);

	for(;;) {
				printf("Frame\n");

		Mat frame;
		stream.read(frame);
		
		//Mat delta = frame.clone();
		//cvtColor(delta, delta, CV_BGR2Lab);

		/*int i = delta.cols, j = delta.rows;
		uint8_t* row;
		uint8_t* oldrow;

		while(j--) {
			row = delta.ptr<uint8_t>(j);
			oldrow = lastFrame.ptr<uint8_t>(j);

			while(i--) {
				Vec3b ref = row[i];
				int mine = labDelta(ref, oldrow, i - 1) + labDelta(ref, oldrow, i + 1);
				//int neighborlDelta = abs(abs(labDelta(ref, oldrow, i - 1) - mine) -
				//					abs(labDelta(ref, oldrow, i + 1) - mine));
									
				//bool hasChanged = neighborlDelta < 25000 && mine > 5000;
				bool highlight = !(mine > 4000);

				row[ (i * 3) ] = highlight * 255;
				row[ (i * 3) + 1] = highlight * 255;
				row[ (i * 3) + 2] = highlight * 255;

			}

			i = delta.cols;
		}*/

		Mat out1, out2, delta;

		absdiff(twoFrame, frame, out1);
		absdiff(lastFrame, frame, out2);
		bitwise_and(out1, out2, delta);

		//imshow("Delta", delta);

		//imshow("Blurred", delta);

		threshold(delta, delta, 60, 255, THRESH_BINARY);
		blur(delta, delta, Size(21, 21), Point(-1, -1));

		//imshow("threshold", delta);

		cvtColor(delta, delta, CV_BGR2GRAY);

		//adaptiveThreshold(delta, delta, 255, ADAPTIVE_THRESH_MEAN_C, THRESH_BINARY, 31, 0);

		//imshow("Not canny", delta);

		Canny(delta, delta, 40, 40 * 3, 3);
		
		//imshow("Canny", delta);


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