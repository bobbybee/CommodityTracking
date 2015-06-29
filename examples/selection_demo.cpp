#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdlib.h>
#include <CommodityTracking.h>

using namespace cv;
using namespace ct;

void drawButton(Mat& visualization, Skeleton* skeleton, int x, int y) {
    bool a1lh = (((skeleton->leftHand().x - x) * (skeleton->leftHand().x - x)) +
              ((skeleton->leftHand().y - y) * (skeleton->leftHand().y - y))
            ) <= (40 + 40) * (40 + 40);
    bool a1rh = (((skeleton->rightHand().x - x) * (skeleton->rightHand().x - x)) +
              ((skeleton->rightHand().y - y) * (skeleton->rightHand().y - y))
            ) <= (40 + 40) * (40 + 40);
    Scalar a1color = a1lh ? Scalar(0, 0, 255) : a1rh ? Scalar(255, 0, 0) : Scalar(255, 255, 255);
    circle(visualization, Point(x, y), 40, a1color, -1);
}


int main(int argc, char** argv) {
  // initialize camera stream from the built-in webcam
  // and initialize FrameHistory with that stream

  VideoCapture stream(0);
  FrameHistory history(stream, 0.25);

  // automatically calibrate userSensitivity

  int minimumArclength = 100;
  int userSensitivity = 260;

  vector<Skeleton*> oldSkeletons;

  for(;;) {
    vector<Skeleton*> skeletons = getSkeleton(oldSkeletons, stream, history, userSensitivity, minimumArclength, true);
    oldSkeletons = skeletons;

    // visualize skeletons
    Mat visualization = history.getLastFrame().clone();
    //Mat visualization = Mat::zeros(history.getLastFrame().size(), CV_8UC3);

    int visWidth = visualization.cols, visHeight = visualization.rows;

    for(int skeletonIndex = 0; skeletonIndex < skeletons.size(); ++skeletonIndex) {
      Skeleton* skeleton = skeletons[skeletonIndex]; // pull skeleton from the vector
      skeleton->setMagnification(visualization); // adjust magnification on the skeleton for visualization properly

      drawButton(visualization, skeleton, 50, 50);
      drawButton(visualization, skeleton, 150, 50);
      drawButton(visualization, skeleton, 250, 50);
      drawButton(visualization, skeleton, 350, 50);
      drawButton(visualization, skeleton, 450, 50);

      circle(visualization, skeleton->leftHand(), 40, Scalar(0, 0, 255), -1);
      circle(visualization, skeleton->rightHand(), 40, Scalar(255, 0, 0), -1);
    }

    imshow("Visualization", visualization);

    if(waitKey(20) == 27) {
      break;
    }
  }

  return 0;
}
