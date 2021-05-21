all:
	g++ -g -std=c++11 -D_XOPEN_SOURCE=700 src/capture_image_2.cc -o capture_image.o -Iinclude/ -lraspicam -pthread -lrt
	g++ -g -std=c++11 -D_XOPEN_SOURCE=700 src/send_image_2.cc -o send_image.o -Iinclude/ -pthread -lrt


debug:
	# make sure LD_LIBRARY_PATH points to /usr/local/lib, otherwise raspicam.so will be missing
	g++ -g -std=c++11 src/capture_image.cc  -o capture.o -Iinclude/  -lraspicam  -pthread -lrt
	# g++ -g -std=c++11 -Iinclude/  -lraspicam src/send_image.cc -pthread -lrt -o send.o

test:
	# g++ -g -std=c++11 src/test_send.cc -o test.o -Iinclude/ -lraspicam -pthread -lrt
	# g++ -g -std=c++11 src/send_image.cc -o send_test.o -Iinclude/ -pthread -lrt

	# g++ -g -std=c++11 -D_XOPEN_SOURCE=700 src/capture_to_shared.cc -o shared_test.o -Iinclude/ -lraspicam -pthread -lrt
	# g++ -g -std=c++11 -D_XOPEN_SOURCE=700 src/write_from_shared.cc -o shared_write_test.o -Iinclude/ -pthread -lrt

	g++ -g -std=c++11 -D_XOPEN_SOURCE=700 src/capture_image_2.cc -o capture_image.o -Iinclude/ -lraspicam -pthread -lrt
	g++ -g -std=c++11 -D_XOPEN_SOURCE=700 src/send_image_2.cc -o send_image.o -Iinclude/ -pthread -lrt
clean: 
	rm -rf *.o
	rm -rf *.ppm