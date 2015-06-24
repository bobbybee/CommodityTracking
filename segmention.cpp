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
        Mat simplifiedMask = simplifyUserMask(mask, frame, minimumArclength);

        // erosion the image to remove the edges
        // it doesn't quite matter how much, I don't think
        int erosionAmount = 5;

        Mat el = getStructuringElement(MORPH_RECT, Size(2 * erosionAmount + 1, 2 * erosionAmount + 1), Point(erosionAmount, erosionAmount));

        Mat thin;
        erode(simplifiedMask, thin, el);

        // AND against the original frame simply for testing purporses

        Mat test;
        bitwise_and(thin, frame, test);

        imshow("Delta", test);

        if(waitKey(10) == 27) {
            break;
        }
    }

    return 0;
}
