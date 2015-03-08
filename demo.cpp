#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

int main(int argc, char** argv) {
	// initialize camera stream from the built-in webcam
	// and initialize FrameHistory with that stream

	VideoCapture stream(0);
	FrameHistory history(stream);

	// automatically calibrate userSensitivity

	int minimumArclength = 150;
	int userSensitivity = 255;
	int limbGracePeriod = 50;
	int minimumEdgeSpacing = 300;

	autoCalibrateSensitivity(&userSensitivity, stream, history, minimumArclength, 1, limbGracePeriod);

	// settings GUI

	int showOriginal = 0;

	namedWindow("Settings", 1);
	createTrackbar("Minimum Arc Length", "Settings", &minimumArclength, 500);
	createTrackbar("Sensitivity", "Settings", &userSensitivity, 1000);
	createTrackbar("Show original?", "Settings", &showOriginal, 1);

	Skeleton lastSkeleton; // last skeleton required for performing tracking

	for(;;) {
		Mat visualization; // initialize a backdrop for the skeleton
		Mat frame, flipped_frame;

		stream.read(flipped_frame);
		flip(flipped_frame, frame, 1);

		Mat delta = history.motion(frame);
		history.append(frame);

		resize(delta, delta, Size(0, 0), 0.1, 0.1);
		resize(frame, frame, Size(0, 0), 0.1, 0.1);
		
		Mat mask = extractUserMask(delta, userSensitivity / 256);
		Mat user = simplifyUserMask(mask, frame, minimumArclength);

		Mat edges;
		Canny(user, edges, 100, 100 * 3, 3);

		vector<vector<Point> > contours;
		vector<Vec4i> hierarchy;

		Mat contourOut = Mat::zeros(frame.size(), CV_8UC3);
		findContours(edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE);


		for(int i = 0; i < contours.size(); ++i) {
			double t_arcLength = arcLength(Mat(contours[i]), true);
			//approxPolyDP(contours[i], contours[i], t_arcLength * 0.015, true);

			if(t_arcLength > minimumArclength) { // remove tiny contours.. don't waste your time
				drawContours(contourOut, contours, i, Scalar(255, 255, 255), 1, 8, hierarchy, 0, Point()); // CV_FILLED produces filled contours to act as a mask
			
				int averageX = 0, averageY = 0, n = 0;
				Point topLeft(edges.rows, edges.cols), bottomRight(0, 0);
				vector<Point> edgePoints;

				for(int j = 0; j < contours[i].size(); ++j) {
					++n;
					averageX += contours[i][j].x;
					averageY += contours[i][j].y;

					if(contours[i][j].x < topLeft.x)
						topLeft.x = contours[i][j].x;
					if(contours[i][j].y < topLeft.y )
						topLeft.y = contours[i][j].y;

					if(contours[i][j].x > bottomRight.x)
						bottomRight.x = contours[i][j].x;
					if(contours[i][j].y > bottomRight.y)
						bottomRight.y = contours[i][j].y;
				}

				for(int j = 0; j < contours[i].size(); ++j) {
					if( (contours[i][j].x - topLeft.x < 3) || (bottomRight.x - contours[i][j].x < 3) || 
						(contours[i][j].y - topLeft.y < 3) || (bottomRight.y - contours[i][j].y < 3)) {
						
						// potentially an edge point
						// however, we would like to simplify a bit
						// if this point is too close to the last point, don't throw it on the map
						
						if(edgePoints.size() > 0) {
							Point lastEdgePoint = edgePoints[edgePoints.size() - 1];
							if( (((lastEdgePoint.x - contours[i][j].x) * (lastEdgePoint.x - contours[i][j].x))
								+ ((lastEdgePoint.y - contours[i][j].y) * (lastEdgePoint.y - contours[i][j].y))) > minimumEdgeSpacing)
									edgePoints.push_back(contours[i][j]);
						} else {
							edgePoints.push_back(contours[i][j]);
						}
					}
				}

				averageX /= n;
				averageY /= n;

				rectangle(contourOut, Point(averageX, averageY), Point(averageX, averageY), Scalar(255, 0, 0), 5);
				//rectangle(contourOut, topLeft, bottomRight, Scalar(0, 0, 255), 5);

				for(int i = 0; i < edgePoints.size(); ++i) {
					rectangle(contourOut, edgePoints[i], edgePoints[i], Scalar(0, 255, 0), 5);
				}

			}
		}

		user = contourOut;

		resize(user, user, Size(0, 0), 10, 10);
		imshow("User", user);

		/*
		if(showOriginal) {
			visualization = history.getLastFrame().clone();
		} else {
			visualization = Mat::zeros(history.getLastFrame().size(), CV_8UC3);
		}
		

		lastSkeleton.visualize(visualization); // use Skeleton::visualize for the demo
												// in a real app, we would access properties such as
												// skeleton.leftMostAbove (left hand position)
		imshow("Visualization", visualization);
*/
		if(waitKey(1) == 27) {
			break;
		}

		// get the Skeleton object
		// takes VideoStream, FrameHistory, Skeleton lastSkeleton,
		// whether to flip the frame, and some sensitivity settings

		//Skeleton skeleton = getSkeleton(stream, history, lastSkeleton, false, minimumArclength, userSensitivity, limbGracePeriod);
		//lastSkeleton = skeleton;
	}
	return 0;
}