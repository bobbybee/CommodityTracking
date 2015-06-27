LIBRARY_BUILD_ARGS =
LINKLIB =
LIBRARY_CLEAN =

ifeq ($(shell uname),Darwin)
	LIBRARY_BUILD_ARGS += g++ *.o -dynamiclib -undefined suppress -flat_namespace -o libcommoditytracking.dylib
	LINKLIB = -L '.' -lcommoditytracking
endif

ifeq ($(shell uname),Linux)
	LIBRARY_CLEAN += - rm libcommoditytracking.a
	LIBRARY_BUILD_ARGS += ar -cvq libcommoditytracking.a *.o
	LINKLIB = -L '.' libcommoditytracking.a
endif

all:
	g++ -o CommodityTracking.o -c -fPIC CommodityTracking.cpp -O3 --std=c++11

	$(LIBRARY_CLEAN)
	$(LIBRARY_BUILD_ARGS)

	g++ -o demo demo.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc -I. $(LINKLIB) -O3
	g++ -o segmentation segmentation.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc -I. $(LINKLIB) -O3
