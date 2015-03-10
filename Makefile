LIBRARY_BUILD_ARGS = *.o

ifeq ($(shell uname),Darwin)
	LIBRARY_BUILD_ARGS += -dynamiclib -undefined suppress -flat_namespace -o libcommoditytracking.dylib
endif

ifeq ($(shell uname),Linux)
	LIBRARY_BUILD_ARGS += -shared -o libcommoditytracking.so
endif

all:
	g++ -o CommodityTracking.o -c -fPIC CommodityTracking.cpp -O3

	g++ $(LIBRARY_BUILD_ARGS)

	g++ -o demo demo.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc  -L '.' -I. -lcommoditytracking -O3
	g++ -o platformer_demo platformer_demo.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc  -L '.' -I. -lcommoditytracking -O3
