all:
	g++ -std=c++11 -D_XOPEN_SOURCE=700 src/capture_image_2.cc -o capture_image.o -Iinclude/ -lraspicam -pthread -lrt
	g++ -std=c++11 -D_XOPEN_SOURCE=700 src/send_image_2.cc -o send_image.o -Iinclude/ -pthread -lrt
	g++ -std=c++11 src/execute_command.cc -o toggle_led.o -Iinclude/ -lwiringPi -pthread -lrt
	g++ -std=c++11 src/generate_log.cc -o logger.o -lrt


debug:
	# make sure LD_LIBRARY_PATH points to /usr/local/lib, otherwise raspicam.so will be missing
	g++ -g -std=c++11 -D_XOPEN_SOURCE=700 src/capture_image_2.cc -o capture_image.o -Iinclude/ -lraspicam -pthread -lrt
	g++ -g -std=c++11 -D_XOPEN_SOURCE=700 src/send_image_2.cc -o send_image.o -Iinclude/ -pthread -lrt
	g++ -g -std=c++11 src/execute_command.cc -o toggle_led.o -Iinclude/ -lwiringPi -pthread -lrt
	g++ -g -std=c++11 src/generate_log.cc -o logger.o -lrt

clean: 
	rm -rf *.o
	rm -rf *.ppm