#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;
using namespace std;

void plotPoint(Mat& mat, Point pt, Scalar colour) {
	rectangle(mat, pt, pt, colour, 30);
}

Mat extractUserMask(Mat& delta) {
	cvtColor(delta, delta, CV_BGR2GRAY);

	threshold(delta, delta, 12, 255, THRESH_BINARY);

	blur(delta, delta, Size(15, 15), Point(-1, -1));
	threshold(delta, delta, 9, 255, THRESH_BINARY);

	blur(delta, delta, Size(35, 35), Point(-1, -1));
	threshold(delta, delta, 19, 255, THRESH_BINARY);

	cvtColor(delta, delta, CV_GRAY2BGR);

	return delta;
}

int main(int argc, char** argv) {
	VideoCapture stream(0);

	Mat lastFrame;
	Mat twoFrame;
	Mat threeFrame;

	stream.read(lastFrame); // fixes a race condition
	stream.read(twoFrame);
	stream.read(threeFrame);

	bool leftHandActive = false;
	Point leftHand(0, 0);
	
	bool rightHandActive = false;
	Point rightHand(0, 0);

	Point center(lastFrame.cols / 2, lastFrame.rows / 2);

	for(;;) {
		// read frame from webcam; flip orientation to natural orientation
		Mat flipped_frame, frame;
	
		stream.read(flipped_frame);
		flip(flipped_frame, frame, 1);
		
		// find pixels in motion
		Mat out1, out2, delta;
		absdiff(twoFrame, frame, out1);
		absdiff(lastFrame, frame, out2);
		bitwise_and(out1, out2, delta);


		delta = extractUserMask(delta);

		Mat user;
		bitwise_and(frame, delta, user);

		imshow("User", user);

		// find contours

		cvtColor(user, user, CV_BGR2GRAY);

		Canny(user, delta, 20, 20 * 3, 3);

		/*vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		vector<Point> approxShape;
		vector<int> largeContours;

		findContours(delta.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		Mat contourVisualization = Mat::zeros(delta.size(), CV_8UC3);

		double totalX = 0, totalY = 0, pointCount = 0; // for computing center

		Mat visualization = frame.clone(); // visualization of skeleton-tracking

		for(int i = 0; i < contours.size(); ++i) {
			double t_arcLength = arcLength(Mat(contours[i]), true);

			if(t_arcLength > 1000) { // remove tiny contours.. don't waste your time
				largeContours.push_back(i);

				//approxPolyDP(contours[i], contours[i], t_arcLength * 0.02, true);
				
				/*for(int j = 0; j < contours[i].size(); ++j) {
					totalX += contours[i][j].x;
					totalY += contours[i][j].y;
					pointCount++;
				}*/

				/*drawContours(contourVisualization, contours, i, Scalar(255, 255, 255), 10);
			}
		}*/


		/*totalX /= pointCount;
		totalY /= pointCount;

		Point local_center(totalX, totalY);

		if(norm(center - local_center) > 50 && local_center.x > 0)
			center = local_center;

		Point upperRight = Point(0, 0), upperLeft = Point(frame.cols, frame.rows);

		for(int i = 0; i < largeContours.size(); ++i) {
			vector<Point> contour = contours[largeContours[i]];

			for(int j = 0; j < contour.size(); ++j) {
				if(contour[j].x > upperRight.x && contour[j].y - totalY < 50 && contour[j].x - center.x > 75)
					upperRight = contour[j];
				if(contour[j].x < upperLeft.x && contour[j].y - totalY < 50 && center.x - contour[j].x > 75)
					upperLeft = contour[j];
			}
		}

		if(upperLeft.x == frame.cols && upperLeft.y == frame.rows) {
			//leftHandActive = false;
		} else {
			leftHand = upperLeft;
			leftHandActive = true;
		}

		if(upperRight.x == 0 && upperRight.y == 0) {
			//rightHandActive = false;
		} else {
			rightHand = upperRight;
			rightHandActive = true;
		}


		plotPoint(visualization, center, Scalar(255, 0, 0));
		
		if(leftHandActive) {
			line(visualization, leftHand, center, Scalar(0, 255, 0));
			plotPoint(visualization, leftHand, Scalar(0, 0, 255));
		}

		if(rightHandActive) {
			line(visualization, rightHand, center, Scalar(255, 255, 0));
			plotPoint(visualization, rightHand, Scalar(0, 255, 255));
		}

		//imshow("Visualization", visualization);*/

		if(waitKey(16) == 27) {
			break;
		}

		threeFrame = twoFrame;
		twoFrame = lastFrame;
		lastFrame = frame;
	}

	return 0;
}