# How to run project

1. install raspicam library  from github (https://github.com/rmsalinas/raspicam)

    After install make sure that lib path is in LD_LIBRARY_PATH env variable 
        (in my case it's /usr/local/lib) in every terminal!

2. Compile both files (`make`)

    two files are created (`capture_image.o` and `send_image.o`)

3. run `./capture_image.o` to begin saving images to shared memory

4. run `./send_image.o` to read from shared memory and save to *.ppm files
