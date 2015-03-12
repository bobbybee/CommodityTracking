LIBRARY_BUILD_ARGS =
LINKLIB =

ifeq ($(shell uname),Darwin)
	LIBRARY_BUILD_ARGS += g++ *.o -dynamiclib -undefined suppress -flat_namespace -o libcommoditytracking.dylib
	LINKLIB = -L '.' -lcommoditytracking
endif

ifeq ($(shell uname),Linux)
	LIBRARY_BUILD_ARGS +=  ar -cvq libcommoditytracking.a *.o
	LINKLIB = -L '.' libcommoditytracking.a
endif

all:
	g++ -o CommodityTracking.o -c -fPIC CommodityTracking.cpp -O3

	$(LIBRARY_BUILD_ARGS)

	g++ -o demo demo.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc -I. $(LINKLIB) -O3
	g++ -o platformer_demo platformer_demo.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc -I. $(LINKLIB) -O3
