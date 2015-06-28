LIBRARY_BUILD_ARGS =
LINKLIB =
LIBRARY_CLEAN =

ifeq ($(shell uname),Darwin)
	LIBRARY_BUILD_ARGS += g++ *.o -dynamiclib -undefined suppress -flat_namespace -o lib/libcommoditytracking.dylib
	LINKLIB = -L './lib' -lcommoditytracking
endif

ifeq ($(shell uname),Linux)
	LIBRARY_CLEAN += - rm lib/libcommoditytracking.a
	LIBRARY_BUILD_ARGS += ar -cvq lib/libcommoditytracking.a *.o
	LINKLIB = -L './lib' lib/libcommoditytracking.a
endif

all:
	g++ -o CommodityTracking.o -c -fPIC src/CommodityTracking.cpp -O3 --std=c++11 -g

	$(LIBRARY_CLEAN)
	$(LIBRARY_BUILD_ARGS)

	g++ -o bin/demo examples/demo.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc -I./src $(LINKLIB) -O3 -g
	g++ -o bin/segmentation examples/segmentation.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc -I./src $(LINKLIB) -O3 -g
