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

		cvtColor(delta, delta, CV_BGR2GRAY);
		blur(delta, delta, Size(75, 75), Point(-1, -1));
		threshold(delta, delta, 5, 255, THRESH_BINARY);

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

		double highestX = 0, lowestX = delta.cols,
				highestY = 0, lowestY = delta.rows;

		for(int i = 0; i < contours.size(); ++i) {
			double my_arcLength = arcLength(Mat(contours[i]), true);
			approxPolyDP(contours[i], approxShape, my_arcLength, true);

			if(my_arcLength > 1000) {
				drawContours(contourDrawing, contours, i, userColour, 3);
			
				for(int j = 0; j < contours[i].size(); ++j) {
					if(contours[i][j].x > highestX) {
						highestX = contours[i][j].x;
					}

					if(contours[i][j].y > highestY) {
						highestY = contours[i][j].y;
					}

					if(contours[i][j].x < lowestX) {
						lowestX = contours[i][j].x;
					}

					if(contours[i][j].y < lowestY) {
						lowestY = contours[i][j].y;
					}
				}
			}
		}

		Point pt1, pt2;
		pt1.x = highestX;
		pt1.y = highestY;

		pt2.x = lowestX;
		pt2.y = lowestY;

		if(highestX > 0) {
			rectangle(contourDrawing, pt1, pt2, normalColour, 10);
			Rect roi(lowestX, lowestY, highestX - lowestX, highestY - lowestY);
			Mat image_roi = frame(roi);
			//imshow("User", image_roi);
		}


		//if(contours.size()) printf("%d\n", contours[contourNumForMax].size());
		//drawContours(contourDrawing, contours, contourNumForMax, userColour, 3);

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