#include "CommodityTracking.h"

FrameHistory::FrameHistory(VideoCapture& stream) {
	stream.read(m_lastFrame); // fixes a race condition in the first few frames
	stream.read(m_twoFrame);
}

void FrameHistory::append(Mat frame) {
	m_twoFrame = m_lastFrame;
	m_lastFrame = frame;
}

Mat FrameHistory::motion(Mat frame) {
	Mat out1, out2, delta;
	absdiff(m_twoFrame, frame, out1);
	absdiff(m_lastFrame, frame, out2);
	bitwise_and(out1, out2, delta);

	return delta;
}

Mat FrameHistory::getLastFrame() {
	return m_lastFrame;
}

// the black-and-white user mask is found from
// the motion extracted image through a series
// of various blurs and corresponding thresholds

Mat extractUserMask(Mat& delta, double sensitivity) {
	cvtColor(delta, delta, CV_BGR2GRAY);

	threshold(delta, delta, sensitivity * 20, 255, THRESH_BINARY);

	blur(delta, delta, Size(15, 15), Point(-1, -1));
	threshold(delta, delta, sensitivity * 15, 255, THRESH_BINARY);

	blur(delta, delta, Size(35, 35), Point(-1, -1));
	threshold(delta, delta, sensitivity * 25, 255, THRESH_BINARY);

	cvtColor(delta, delta, CV_GRAY2BGR);

	return delta;
}

Skeleton getSkeleton(VideoCapture& stream, FrameHistory& history, Skeleton previous, bool _flip, int minimumArclength, int userSensitivity, int limbGracePeriod) {
	Skeleton final;

	// read frame from webcam; flip orientation to natural orientation
	Mat flipped_frame, frame;
	
	if(_flip) {
		stream.read(flipped_frame);
		flip(flipped_frame, frame, 1);
	} else {
		stream.read(frame);
	}

	final.fullWidth = frame.cols;
	final.fullHeight = frame.rows;

	Mat delta = history.motion(frame);
	delta = extractUserMask(delta, userSensitivity / 256);

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

	Point topMost(frame.cols, frame.rows),
		  bottomMost(0, 0),
		  leftMost(frame.cols, frame.rows),
		  rightMost(0, 0);

	for(int i = 0; i < contours.size(); ++i) {
		double t_arcLength = arcLength(Mat(contours[i]), true);

		if(t_arcLength > minimumArclength) { // remove tiny contours.. don't waste your time
			largeContours.push_back(i);

			approxPolyDP(contours[i], contours[i], t_arcLength * 0.02, true);
			
			for(int j = 0; j < contours[i].size(); ++j) {
				if(contours[i][j].y < topMost.y) topMost = contours[i][j];
				if(contours[i][j].y > bottomMost.y) bottomMost = contours[i][j];
				if(contours[i][j].x < leftMost.x) leftMost = contours[i][j];
				if(contours[i][j].x > rightMost.x) rightMost = contours[i][j];
			}

			// drawContours(contourVisualization, contours, i, Scalar(255, 255, 255), 10);
		}
	}

	Point center_of_rect((leftMost.x + rightMost.x) / 2, (topMost.y + bottomMost.y) / 2);

	// find outer limb positions
	Point leftMostAbove(frame.cols, frame.rows), rightMostAbove(0, 0),
		  leftMostBelow(frame.cols, frame.rows), rightMostBelow(0, 0);

	for(int i = 0; i < largeContours.size(); ++i) {
		vector<Point> contour = contours[largeContours[i]];
			
		for(int j = 0; j < contour.size(); ++j) {
			if( (contour[j].y - center_of_rect.y ) > limbGracePeriod) { // below
				if(contour[j].x < leftMostBelow.x) leftMostBelow = contour[j];
				if(contour[j].x > rightMostBelow.x) rightMostBelow = contour[j];
			} else if( (center_of_rect.y - contour[j].y) > limbGracePeriod) { // above
				if(contour[j].x < leftMostAbove.x) leftMostAbove = contour[j];
				if(contour[j].x > rightMostAbove.x) rightMostAbove = contour[j];
			}
		}
	}

	// assemble skeleton structure

	// if there is a previous skeletal position that is out of range, reject it

	int distanceConstant = 250;

	if(!(previous.center_of_rect.x > 0 && norm(previous.center_of_rect - center_of_rect) > distanceConstant))
		final.center_of_rect = center_of_rect;
	
	if(!(previous.leftMostAbove.x > 0 && norm(previous.leftMostAbove - leftMostAbove) > distanceConstant))
			final.leftMostAbove = leftMostAbove;

	if(!(previous.leftMostBelow.x > 0 && norm(previous.leftMostBelow - leftMostBelow) > distanceConstant))
			final.leftMostBelow = leftMostBelow;

	if(!(previous.rightMostAbove.x > 0 && norm(previous.rightMostAbove - rightMostAbove) > distanceConstant))
			final.rightMostAbove = rightMostAbove;

	if(!(previous.rightMostBelow.x > 0 && norm(previous.rightMostBelow - rightMostBelow) > distanceConstant))
			final.rightMostBelow = rightMostBelow;

	if(!(previous.topMost.x > 0 && norm(previous.topMost - topMost) > distanceConstant))
			final.topMost = topMost;


	history.append(frame);

	return final;
}

void autoCalibrateSensitivity(int* userSensitivity, VideoCapture& stream, FrameHistory& history, int minimumArclength, int interval, int limbGracePeriod) {
	Skeleton nullSkeleton;

	while(*userSensitivity < 1000) {
		Skeleton skeleton = getSkeleton(stream, history, nullSkeleton, false, minimumArclength, *userSensitivity, limbGracePeriod);
		
		if(skeleton.rightMostAbove.x == 0) {
			// optimal calibration found
			return;
		}

		*userSensitivity += interval;
	}

	// if this point is reached, all hope is lost :(
}