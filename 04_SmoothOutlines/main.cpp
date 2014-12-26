#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;

int main(int argc, char** argv) {
	VideoCapture stream(0);

	Mat lastFrame;
	stream.read(lastFrame); // fixes a race condition

	for(;;) {
		Mat frame;
		stream.read(frame);
		
		Mat delta = frame.clone();
		cvtColor(delta, delta, CV_BGR2Lab);

		int i = delta.rows, j = delta.cols;

		while(j--) {
			while(i--) {
				Vec3b pixel = delta.at<Vec3b>(i, j);
				Vec3b oldPixel = lastFrame.at<Vec3b>(i, j);

				// check if the colour changed more than a threshold
				double delta0 = pixel[0] - oldPixel[0];
				double delta1 = pixel[1] - oldPixel[1];
				double delta2 = pixel[2] - oldPixel[2];

				bool hasChanged = (delta0 * delta0) + (delta1 * delta1) + (delta2 * delta2) > 30000;

				delta.at<Vec3b>(i, j) = Vec3b(hasChanged * 255, hasChanged * 255, hasChanged * 255);

			}

			i = delta.rows;
		}


		for(int i = 1; i < 25; i = i + 2) {
			blur(delta, delta, Size(i, i), Point(-1, -1));
		}

		i = delta.rows;
		j = delta.cols;

		while(j--) {
			while(i--) {
				Vec3b blurred = delta.at<Vec3b>(i, j);
				
				if( blurred[0] > 200) {
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

		Canny(delta, delta, 100, 150, 3);

		imshow("Focused Delta Average", delta);
		
		if(waitKey(10) == 27) {
			break;
		
		}
		lastFrame = frame;
	}

	return 0;
}