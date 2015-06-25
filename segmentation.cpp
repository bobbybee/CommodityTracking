#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

using namespace ct;
using namespace cv;

int main(int argc, char** argv) {
    // camera stream
    
    VideoCapture stream(0);
    FrameHistory history(stream);

    // sensitivity calibration
    // TODO: phase this step out
    // it's kind of pointless, makes a lot of assumptions, and doesn't always work
	
    int minimumArclength = 100;
	int userSensitivity = autoCalibrateSensitivity(256, stream, minimumArclength, 1);

    for(;;) {
        // main loop
        // fetch an image from the camera
        
        Mat frame, flipped_frame;
        stream.read(flipped_frame);
        flip(flipped_frame, frame, 1);

        // motion delta computation
        // modified Canny et al algorithm
        
        Mat delta = history.motion(frame);
        history.append(frame);

        // extract the user mask
        Mat mask = extractUserMask(delta, userSensitivity / 256);
        //Mat simplifiedMask = simplifyUserMask(mask, frame, minimumArclength);

        imshow("Mask:", mask);
        // erode the image to remove the edges
        // it doesn't quite matter how much, I don't think
        int erosionAmount = 10;

        Mat el = getStructuringElement(MORPH_RECT, Size(2 * erosionAmount + 1, 2 * erosionAmount + 1), Point(erosionAmount, erosionAmount));

        Mat thin;
        dilate(mask, thin, el);

        cvtColor(thin, thin, CV_BGR2GRAY);

        // background
        
        floodFill(thin, Point(0,0), CV_RGB(127,127,127));

        // watershed!
        
        Mat markers;
        thin.convertTo(markers, CV_32S);

        watershed(frame, markers);
       

        Mat test;
        
        markers.convertTo(markers, CV_8U);
        cvtColor(markers, markers, CV_GRAY2BGR);

        // completely cancel out the background
        threshold(markers, markers, 254, 255, THRESH_BINARY);

        imshow("Mark", markers);
        //cvtColor(frame, frame, CV_BGR2GRAY);

        Mat pureMask = simplifyUserMask(markers, frame, minimumArclength);
        erode(pureMask, pureMask, el);

        bitwise_and(frame, pureMask, test);

        imshow("Test", test);

        if(waitKey(10) == 27) {
            break;
        }
    }

    return 0;
}
