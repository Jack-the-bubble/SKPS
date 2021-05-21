#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include <fstream>
#include "semafor.h"

int main(int argc, char** argv) {
    init();
    int n;
    int max_iter = BUFFER_SIZE;
    MEM *S = memory();

    char img_name[24];
    const char *name = "/rpi-images";	// file name
    const int SIZE = 1228800;

    unsigned char *data=new unsigned char[SIZE];

    int shm_fd;		// file descriptor, from shm_open()
    char *shm_base;	// base address, from mmap()
    char *ptr_; // offset to read different images

    shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        printf("cons: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }

    /* map the shared memory segment to the address space of the process */
    shm_base = (char *)mmap(0, SIZE*BUFFER_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        printf("cons: Map failed: %s\n", strerror(errno));
        // close and unlink?
        exit(1);
    }

    // for (n= 0; n<max_iter; n++)
    while(1)
    {
        sem_wait(&S->full);
        sem_wait(&S->mutex);
        sem_getvalue(&S->full, &n);

        sprintf(img_name, "from-shared-%d.ppm", n);
        memcpy(data, shm_base+n*SIZE, SIZE*sizeof(unsigned char));

        // here we'll send to tcp server
        std::ofstream outFile(img_name, std::ios::binary);
        outFile<<"P6\n"<<1280 <<" "<<960 <<" 255\n";
        outFile.write ( ( char* ) data, SIZE);

        std::cout<<"Saved file "<<img_name<<std::endl;

        std::cout<<"Got data from semaphore"<<(S->buff)[n]<<std::endl;
        sem_post(&S->mutex);
        sem_post(&S->empty);
        sleep(CONSUMER_SLEEP_SEC);
    }


    delete data;
    /* remove the mapped shared memory segment from the address space of the process */
    if (munmap(shm_base, SIZE) == -1) {
        printf("cons: Unmap failed: %s\n", strerror(errno));
        exit(1);
    }

    /* close the shared memory segment as if it was a file */
    if (close(shm_fd) == -1) {
        printf("cons: Close failed: %s\n", strerror(errno));
        exit(1);
    }

    /* remove the shared memory segment from the file system */
    if (shm_unlink(name) == -1) {
        printf("cons: Error removing %s: %s\n", name, strerror(errno));
        exit(1);
    }


    return 0;
}