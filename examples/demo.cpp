#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

using namespace cv;
using namespace ct;

int main(int argc, char** argv) {
    SkeletonTracker tracker;

    for(;;) {
        vector<Skeleton*> skeletons = tracker.getSkeletons();

        // visualize skeletons
        
        // to see the background, uncomment the next 3 lines
        
        // Mat visualization = tracker.cloneFrame(); 
        // resize(visualization, visualization, Size(0, 0), 2, 2); // scale up :) 
        
        // and comment the next 2 lines
         
        Size size = tracker.webcamDimensions();
        Mat visualization = Mat::zeros(size.height * 2, size.width * 2, CV_8UC3);
        

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

        if(waitKey(16) == 27) {
            break;
        }
    }

    return 0;
}
