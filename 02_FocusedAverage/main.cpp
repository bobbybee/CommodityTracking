#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;

int main(int argc, char** argv) {
	VideoCapture stream(0);

	Mat lastFrame;
	stream.read(lastFrame); // fixes a race condition

	double* focusedAverage = new double [ lastFrame.rows * lastFrame.cols ];

	for(;;) {
		Mat frame;
		stream.read(frame);
		
		Mat delta = frame.clone();

		int i = delta.rows, j = delta.cols;

		int maxSum = 0;

		while(j--) {
			while(i--) {
				Vec3b pixel = delta.at<Vec3b>(i, j);
				Vec3b oldPixel = lastFrame.at<Vec3b>(i, j);

				// check if the colour changed more than a threshold
				bool hasChanged = (abs(pixel[0] - oldPixel[0]) + abs(pixel[1] - oldPixel[1]) + abs(pixel[2] - oldPixel[2])) > 50;

				// keep a total, but slightly scale down the previous sum to keep time-based focus
				focusedAverage[ (j * delta.rows) + i] = (0.5 * focusedAverage[ (j * delta.rows) + i]) + hasChanged;
				
				if(focusedAverage[ (j * delta.rows) + i] > maxSum) {
					maxSum = focusedAverage[ (j * delta.rows) + i];
				}
			}

			i = delta.rows;
		}

		i = delta.rows;
		j = delta.cols;

		while(j--) {
			while(i--) {
				double comparison = focusedAverage[ (j * delta.rows) + i] / maxSum;
				int greyscaleView = (comparison * (double) 255);

				if(greyscaleView < 50) greyscaleView = 0; // noise reduction threshold

				Vec3b pixel;
				pixel[0] = greyscaleView;
				pixel[1] = greyscaleView;
				pixel[2] = greyscaleView;

				delta.at<Vec3b>(i, j) = pixel;
			}

			i = delta.rows;
		}

		imshow("Focused Delta Average", delta);
		
		if(waitKey(10) == 27) {
			break;
		
		}
		lastFrame = frame;
	}

	return 0;
}