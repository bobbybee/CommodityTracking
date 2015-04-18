CommodityTracking
=========

[![Join the chat at https://gitter.im/bobbybee/CommodityTracking](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/bobbybee/CommodityTracking?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[CommodityTracking](http://bobbybee.github.io/CommodityTracking/) is a library for performing skeleton tracking with only a single, RGB camera, such as those found on many consumer laptops today.

Dependencies
=========

OpenCV must be installed to build CommodityTracking. CommodityTracking has been tested on OpenCV version 2.4.9.

Building
=========

Running the Makefile will result in the library being built in the current directory, and the skeleon tracking demo demonstrating the simplest usage of the library. Once it is built, additional samples can be built like so:

    g++ -o demo demo.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc  -L '.' -I. -lcommoditytracking -O3

Note the dependency on both the CommodityTracking library and these three OpenCV modules.

Documentation
=========

CommodityTracking has a website: http://bobbybee.github.io/CommodityTracking/

In addition, Doxygen generated documentation on the API is available:  http://bobbybee.github.io/CommodityTracking/docs/namespacect.html
