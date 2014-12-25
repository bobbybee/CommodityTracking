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
				focusedAverage[ (j * delta.rows) + i] = (0.01 * focusedAverage[ (j * delta.rows) + i]) + hasChanged;
				
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

				// let's also get a few neighbors
				// one lone pixel is a sign of noise

				double neighborAverage = (focusedAverage[ ( (j - 1) * delta.rows) + i] / maxSum) + 
										(focusedAverage[ ( (j + 1) * delta.rows) + i] / maxSum) +
										(focusedAverage[ ( j * delta.rows) + (i - 1)] / maxSum) +
										(focusedAverage[ ( j * delta.rows) + (i + 1)] / maxSum);

				greyscaleView = (255 - (neighborAverage / 4)) + greyscaleView > 400 ? 255 : 0; // clip it for creating the silhouette

				Vec3b pixel;
				pixel[0] = greyscaleView;
				pixel[1] = greyscaleView;
				pixel[2] = greyscaleView;

				delta.at<Vec3b>(i, j) = pixel;
			}

			i = delta.rows;
		}

		for(int i = 1; i < 15; i = i + 2) {
			GaussianBlur(delta, delta, Size(i, i), 0, 0);
		}

		i = delta.rows;
		j = delta.cols;

		while(j--) {
			while(i--) {
				Vec3b blurred = delta.at<Vec3b>(i, j);
				
				if( (blurred[0] + blurred[1] + blurred[2]) > 700) {
					blurred[0] = 255;
					blurred[1] = 255;
					blurred[2] = 255;
				} else {
					blurred[0] = 0;
					blurred[1] = 0;
					blurred[2] = 0;
				}

				delta.at<Vec3b>(i, j) = blurred;
			}

			i = delta.rows;
		}

		Canny(delta, delta, 0, 50, 5);

		imshow("Focused Delta Average", delta);
		
		if(waitKey(10) == 27) {
			break;
		
		}
		lastFrame = frame;
	}

	return 0;
}