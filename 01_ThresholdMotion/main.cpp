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

		printf("%d,%d\n", delta.rows, delta.cols);

		int i = delta.rows, j = delta.cols;
		while(j--) {
			while(i--) {
				Vec3b pixel = delta.at<Vec3b>(i, j);
				Vec3b oldPixel = lastFrame.at<Vec3b>(i, j);

				// check if the colour changed more than a threshold
				bool hasChanged = (abs(pixel[0] - oldPixel[0]) + abs(pixel[1] - oldPixel[1]) + abs(pixel[2] - oldPixel[2])) > 50;

				if(hasChanged) {
					pixel[0] = 255;
					pixel[1] = 255;
					pixel[2] = 255;
				} else {
					pixel[0] = 0;
					pixel[1] = 0;
					pixel[2] = 0;
				}

				delta.at<Vec3b>(i, j) = pixel;
			}

			i = delta.rows;
		}


		imshow("Delta", delta);
		
		if(waitKey(10) == 27) {
			break;
		
		}
		lastFrame = frame;
	}

	return 0;
}