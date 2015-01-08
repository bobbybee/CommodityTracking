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


		// find contours

		cvtColor(user, user, CV_BGR2GRAY);

		blur(user, user, Size(3, 3));
		Canny(user, delta, 50, 50 * 3, 3);

		//imshow("User", user);

		//imshow("Delta outline", delta);

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		vector<Point> approxShape;
		vector<int> largeContours;

		findContours(delta.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		//Mat contourVisualization = Mat::zeros(delta.size(), CV_8UC3);
		Mat contourVisualization = frame.clone();

		//double totalX = 0, totalY = 0, pointCount = 0; // for computing center
		Point topMost(frame.cols, frame.rows), bottomMost(0, 0), leftMost(frame.cols, frame.rows), rightMost(0, 0);

		Mat visualization = frame.clone(); // visualization of skeleton-tracking

		for(int i = 0; i < contours.size(); ++i) {
			double t_arcLength = arcLength(Mat(contours[i]), true);

			if(t_arcLength > 250) { // remove tiny contours.. don't waste your time
				largeContours.push_back(i);

				approxPolyDP(contours[i], contours[i], t_arcLength * 0.02, true);
				
				for(int j = 0; j < contours[i].size(); ++j) {
					if(contours[i][j].y < topMost.y) topMost = contours[i][j];
					if(contours[i][j].y > bottomMost.y) bottomMost = contours[i][j];
					if(contours[i][j].x < leftMost.x) leftMost = contours[i][j];
					if(contours[i][j].x > rightMost.x) rightMost = contours[i][j];

				}

				//drawContours(contourVisualization, contours, i, Scalar(255, 255, 255), 10);
			}
		}

		Point center_of_rect((leftMost.x + rightMost.x) / 2, (topMost.y + bottomMost.y) / 2);

		// find feet positions
		Point leftMostAbove(frame.cols, frame.rows), rightMostAbove(0, 0),
			  leftMostBelow(frame.cols, frame.rows), rightMostBelow(0, 0);

		for(int i = 0; i < contours.size(); ++i) {
			double t_arcLength = arcLength(Mat(contours[i]), true);

			if(t_arcLength > 250) { // remove tiny contours.. don't waste your time
				largeContours.push_back(i);

				approxPolyDP(contours[i], contours[i], t_arcLength * 0.02, true);
				
				for(int j = 0; j < contours[i].size(); ++j) {
					if(contours[i][j].y > center_of_rect.y) { // below
						if(contours[i][j].x < leftMostBelow.x) leftMostBelow = contours[i][j];
						if(contours[i][j].x > rightMostBelow.x) rightMostBelow = contours[i][j];
					} else { // above
						if(contours[i][j].x < leftMostAbove.x) leftMostAbove = contours[i][j];
						if(contours[i][j].x > rightMostAbove.x) rightMostAbove = contours[i][j];
					}

				}

				drawContours(contourVisualization, contours, i, Scalar(255, 255, 255), 10);
			}

		}


		/*totalX /= pointCount;
		totalY /= pointCount;

		Point local_center(totalX, totalY);*/


		/*plotPoint(contourVisualization, topMost, Scalar(0, 0, 0));
		plotPoint(contourVisualization, bottomMost, Scalar(0, 0, 0));
		plotPoint(contourVisualization, leftMost, Scalar(0, 0, 0));
		plotPoint(contourVisualization, rightMost, Scalar(0, 0, 0));

		line(contourVisualization, topMost, center_of_rect, Scalar(0, 255, 0));
		line(contourVisualization, rightMost, center_of_rect, Scalar(0, 255, 0));
		line(contourVisualization, bottomMost, center_of_rect, Scalar(0, 255, 0));
		line(contourVisualization, leftMost, center_of_rect, Scalar(0, 255, 0));*/

		line(contourVisualization, topMost, center_of_rect, Scalar(0, 255, 0));
		line(contourVisualization, bottomMost, center_of_rect, Scalar(0, 255, 0));

		line(contourVisualization, rightMostAbove, center_of_rect, Scalar(0, 255, 0));
		line(contourVisualization, leftMostAbove, center_of_rect, Scalar(0, 255, 0));
		
		line(contourVisualization, rightMostBelow, center_of_rect, Scalar(0, 255, 0));
		line(contourVisualization, leftMostBelow, center_of_rect, Scalar(0, 255, 0));

		imshow("contourVisualization", contourVisualization);


		/*

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