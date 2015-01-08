#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>

using namespace cv;
using namespace std;

void plotPoint(Mat& mat, Point pt, Scalar colour) {
	rectangle(mat, pt, pt, colour, 30);
}

// the black-and-white user mask is found from
// the motion extracted image through a series
// of various blurs and corresponding thresholds

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

	Mat lastFrame, twoFrame, threeFrame;

	stream.read(lastFrame); // fixes a race condition in the first few frames
	stream.read(twoFrame);
	stream.read(threeFrame);

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

		// extract the user mask
		// and use it to mask out the user in the original image

		delta = extractUserMask(delta);

		Mat user;
		bitwise_and(frame, delta, user);

		// find contours in the user image after removing noise

		cvtColor(user, user, CV_BGR2GRAY);

		blur(user, user, Size(3, 3));
		Canny(user, delta, 50, 50 * 3, 3);

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;
		vector<Point> approxShape;
		vector<int> largeContours;

		findContours(delta.clone(), contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

		//Mat contourVisualization = Mat::zeros(delta.size(), CV_8UC3);
		Mat contourVisualization = frame.clone();

		Point topMost(frame.cols, frame.rows),
			  bottomMost(0, 0),
			  leftMost(frame.cols, frame.rows),
			  rightMost(0, 0);

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
			}
		}

		Point center_of_rect((leftMost.x + rightMost.x) / 2, (topMost.y + bottomMost.y) / 2);

		// find outer limb positions
		Point leftMostAbove(frame.cols, frame.rows), rightMostAbove(0, 0),
			  leftMostBelow(frame.cols, frame.rows), rightMostBelow(0, 0);

		for(int i = 0; i < largeContours.size(); ++i) {
			vector<Point> contour = contours[largeContours[i]];
				
			for(int j = 0; j < contour.size(); ++j) {
				if( (contour[j].y - center_of_rect.y ) > 50) { // below
					if(contour[j].x < leftMostBelow.x) leftMostBelow = contour[j];
					if(contour[j].x > rightMostBelow.x) rightMostBelow = contour[j];
				} else if( (center_of_rect.y - contour[j].y) > 50) { // above
					if(contour[j].x < leftMostAbove.x) leftMostAbove = contour[j];
					if(contour[j].x > rightMostAbove.x) rightMostAbove = contour[j];
				}
			}
		}

		// draw the skeleton

		line(contourVisualization, topMost, center_of_rect, Scalar(0, 255, 0), 20);
		line(contourVisualization, rightMostAbove, center_of_rect, Scalar(0, 255, 0), 20);
		line(contourVisualization, leftMostAbove, center_of_rect, Scalar(0, 255, 0), 20);
		line(contourVisualization, rightMostBelow, center_of_rect, Scalar(0, 255, 0), 20);
		line(contourVisualization, leftMostBelow, center_of_rect, Scalar(0, 255, 0), 20);

		imshow("contourVisualization", contourVisualization);

		if(waitKey(16) == 27) {
			break;
		}

		threeFrame = twoFrame;
		twoFrame = lastFrame;
		lastFrame = frame;
	}

	return 0;
}