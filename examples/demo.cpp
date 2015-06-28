#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

using namespace cv;
using namespace ct;

int main(int argc, char** argv) {
    // initialize camera stream from the built-in webcam
    // and initialize FrameHistory with that stream

    VideoCapture stream(0);
    FrameHistory history(stream);

    int minimumArclength = 100;
    int userSensitivity = (argc == 1) ? 260 : atoi(argv[1]); 

    vector<Skeleton*> oldSkeletons;

    for(;;) {
        vector<Skeleton*> skeletons = getSkeleton(oldSkeletons, stream, history, userSensitivity, minimumArclength, 0.5, true);
        oldSkeletons = skeletons;

        // visualize skeletons
        Mat visualization = history.getLastFrame().clone();
        //Mat visualization = Mat::zeros(history.getLastFrame().size(), CV_8UC3);

        int visWidth = visualization.cols, visHeight = visualization.rows;

        for(int skeletonIndex = 0; skeletonIndex < skeletons.size(); ++skeletonIndex) {
            Skeleton* skeleton = skeletons[skeletonIndex]; // pull skeleton from the vector
            skeleton->setMagnification(visualization); // adjust magnification on the skeleton for visualization properly

            // draw limbs and label according to side of body

            line(visualization, skeleton->center(), skeleton->leftHand(), Scalar(255, 255, 255), 5);
            rectangle(visualization, skeleton->leftHand(), skeleton->leftHand(), Scalar(0, 0, 255), 50);
            putText(visualization, "L", skeleton->leftHand(), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));

            line(visualization, skeleton->center(), skeleton->rightHand(), Scalar(255, 255, 255), 5);
            rectangle(visualization, skeleton->rightHand(), skeleton->rightHand(), Scalar(0, 0, 255), 50);
            putText(visualization, "R", skeleton->rightHand(), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 255, 255));

            line(visualization, skeleton->center(), skeleton->leftLeg(), Scalar(255, 255, 255), 5);
            rectangle(visualization, skeleton->leftLeg(), skeleton->leftLeg(), Scalar(0, 255, 0), 50);
            putText(visualization, "L", skeleton->leftLeg(), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0));

            line(visualization, skeleton->center(), skeleton->rightLeg(), Scalar(255, 255, 255), 5);
            rectangle(visualization, skeleton->rightLeg(), skeleton->rightLeg(), Scalar(0, 255, 0), 50);
            putText(visualization, "R", skeleton->rightLeg(), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 0));

            // draw center
            rectangle(visualization, skeleton->center(), skeleton->center(), Scalar(255, 255, 0), 50);

            // draw head
            line(visualization, skeleton->center(), skeleton->head(), Scalar(255, 255, 255), 5);
            rectangle(visualization, skeleton->head(), skeleton->head(), Scalar(255, 0, 255), 100);

            // check for gestures
            
            if( (skeleton->rightHand().y < 64 && skeleton->leftHand().y > 64 && skeleton->rightHand().x > 64)) {
                putText(visualization, "Coin Right!", Point(100, 100), FONT_HERSHEY_SIMPLEX, 4, Scalar(255, 0, 0), 10);
            } else if( (skeleton->leftHand().y < 64 && skeleton->rightHand().y > 64 && skeleton->leftHand().x > 64)) {
                putText(visualization, "Coin Left!", Point(100, 100), FONT_HERSHEY_SIMPLEX, 4, Scalar(255, 0, 0), 10);
            }
        }

        imshow("Visualization", visualization);

        if(waitKey(1) == 27) {
            break;
        }
    }

    return 0;
}
