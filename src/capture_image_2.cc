// #include <fstream>
#include <fstream>
#include "semafor.h"


#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>


int main(int argc, char **argv) {
    init();
    MEM *S = memory();
    int n; //current index in image buffer?

    raspicam::RaspiCam camera; //Camera object
    camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);

    std::cout<<"Opening camera..."<<std::endl;
    if ( !camera.open()) {std::cerr<<"Error opening camera"<<std::endl;return -1;}
    sleep(1);
    


    const char *name = "/rpi-images";	// file name
    const int SIZE = camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_GRAY );  // file size

    int shm_fd;		// file descriptor, from shm_open()
    char *shm_base;	// base address, from mmap()
    
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("prod: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }

    /* configure the size of the shared memory segment */
    ftruncate(shm_fd, SIZE*BUFFER_SIZE);

    for (int n=0; n<BUFFER_SIZE; n++)
    // while (1)
    {
        // sem_wait(&S->empty);
        // sem_wait(&S->mutex);
        // sem_getvalue(&S->full, &n);


        /* map the shared memory segment to the address space of the process */
        shm_base = (char *)mmap(0, SIZE*BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shm_base == MAP_FAILED) {
            printf("prod: Map failed: %s\n", strerror(errno));
            // close and shm_unlink?
            exit(1);
        }

        camera.grab();
        unsigned char *data=new unsigned char[  camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_GRAY )];

        camera.retrieve(data, raspicam::RASPICAM_FORMAT_GRAY);

        // allocate shared memory and put image data there



        // sprintf(shm_base, "%s", data);
        memcpy(shm_base+n*SIZE, data, SIZE*sizeof(unsigned char));



        std::cout<<"Saved to shared memory "<<n<<std::endl;
        // (S->buff)[n] = n;
        // sem_post(&S->mutex);
        // sem_post(&S->full);
        sleep(PRODUCER_SLEEP_SEC);
    }

    /*
    // std::ofstream outFile("to_shared.ppm", std::ios::binary);
    // outFile<<"P6\n"<<1280 <<" "<<960 <<" 255\n";
    // outFile.write ( ( char* ) shm_base, SIZE);
    */

    /* remove the mapped memory segment from the address space of the process */
    if (munmap(shm_base, SIZE) == -1) {
        printf("prod: Unmap failed: %s\n", strerror(errno));
        exit(1);
    }

    /* close the shared memory segment as if it was a file */
    if (close(shm_fd) == -1) {
        printf("prod: Close failed: %s\n", strerror(errno));
        exit(1);
    }

    return 0;
}