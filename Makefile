all:
	g++ -o CommodityTracking.o -c -fPIC CommodityTracking.cpp -O3
	gcc -dynamiclib -undefined suppress -flat_namespace *.o -o libcommoditytracking.dylib

	g++ -o demo demo.cpp -lopencv_core -lopencv_highgui -lopencv_imgproc  -L '.' -I. -lcommoditytracking -O3