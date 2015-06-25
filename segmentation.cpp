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
        // Collins et al algorithm
        
        Mat delta = history.motion(frame);
        history.append(frame);

        // extract the user mask
        Mat mask = extractUserMask(delta, userSensitivity / 256);
         
        // dilate the image for the watershed
        // the image will be eroded an equal amount later,
        // so the net erosion / dilation is still zero, kind of
        // but this transform has some useful properties for performing watershed segmentation

        int erosionAmount = 10;
        Mat el = getStructuringElement(MORPH_RECT, Size(2 * erosionAmount + 1, 2 * erosionAmount + 1), Point(erosionAmount, erosionAmount));

        Mat thin;
        dilate(mask, thin, el);

        cvtColor(thin, thin, CV_BGR2GRAY);

        // provide watershed a background color by running floodFIll on the markers
        
        floodFill(thin, Point(0,0), CV_RGB(127,127,127));

        // run the watershed transform itself
        // watershed operates in the esoteric CV_32S matrix type (signed 32-bit integers)
        // this makes sense for watershed; not so much for our image processing
        // as a result, we simply pad watershed with convertTo calls  (for our sanity)
        // it is also necessary to convert back to the RGB color space

        Mat markers;
        
        thin.convertTo(markers, CV_32S);
        watershed(frame, markers);
        markers.convertTo(markers, CV_8U);
        
        cvtColor(markers, markers, CV_GRAY2BGR);
       
        Mat test;

        // completely cancel out the background
        // remember, the background at the moment is actually #7F7F7F, a perfect gray
        // by thresholding the image with #FEFEFE, the grey background will become black,
        // and as the user was already white,
        // this threshold creates a perfect (slightly noisy) user mask
        
        threshold(markers, markers, 254, 255, THRESH_BINARY);

        // noise reduction is performed by finding the raw contours of the image,
        // and redrawing them depending on the size
        // unfortunately, this is an expensive call, but it is well worth it for the results!
        
        Mat pureMask = simplifyUserMask(markers, frame, minimumArclength);

        // finally, the original dilate call causes this "fatness" illusion on the mask
        // by eroding the mask by the same amount of the dilation, the mask becomes much more tight
        
        erode(pureMask, pureMask, el);

        // visualization:
        // display the mask in and of itself is interesting
        
        imshow("Spotless user mask", pureMask);
        
        // but a binary mask ANDed with the original frame
        // is just cropping the image.
        // automatic motion-based cropping for the win!

        bitwise_and(frame, pureMask, test);
        imshow("Auto-cropped user", test);


        if(waitKey(10) == 27) {
            break;
        }
    }

    return 0;
}
