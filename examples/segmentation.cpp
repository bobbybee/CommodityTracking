#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

using namespace ct;
using namespace cv;

int main(int argc, char** argv) {
    // camera stream
    
    VideoCapture stream(0);
    FrameHistory history(stream, 0.25);

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
        // Collins et al algorithm
        
        Mat delta = history.motion(frame);
        history.append(frame);

        Mat mask = highUserMask(delta, frame, minimumArclength, userSensitivity / 256);

        // visualization:
        // display the mask in and of itself is interesting
        
        imshow("Spotless user mask", mask);
        
        // but a binary mask ANDed with the original frame
        // is just cropping the image.
        // automatic motion-based cropping for the win!

        Mat test;
        bitwise_and(frame, mask, test);
        imshow("Auto-cropped user", test);

        if(waitKey(20) == 27) {
            break;
        }
    }

    return 0;
}
