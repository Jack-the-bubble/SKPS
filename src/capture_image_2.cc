// #include <fstream>
#include <fstream>
#include "semafor.h"


#include <stdlib.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>

// includes for logging
#include <chrono>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>


int main(int argc, char **argv) {
    init();
    MEM *S = memory();
    int n; //current index in image buffer?

    raspicam::RaspiCam camera; //Camera object
    camera.setFormat(raspicam::RASPICAM_FORMAT_GRAY);

    // init data for logger
    char client_queue_name [64];
    short queue_name_len;
    char og_publisher_name [64];
    mqd_t qd_server, qd_client;   // queue descriptors
    int64_t chrono_current_time;

    // create the client queue for receiving messages from server
    sprintf (client_queue_name, "/camera-capture-%d-", getpid ());
    queue_name_len = strlen(client_queue_name);

    std::cout<<"Opening camera..."<<std::endl;
    if ( !camera.open()) {std::cerr<<"Error opening camera"<<std::endl;return -1;}
    sleep(1);

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_client = mq_open (client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Client: mq_open (client)");
        exit (1);
    }

    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client: mq_open (server)");
        exit (1);
    }
    


    const char *name = "/rpi-images";	// shared memory segment
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

    /* map the shared memory segment to the address space of the process */
    shm_base = (char *)mmap(0, SIZE*BUFFER_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        printf("prod: Map failed: %s\n", strerror(errno));
        // close and shm_unlink?
        exit(1);
    }
    unsigned char *data=new unsigned char[  camera.getImageTypeSize ( raspicam::RASPICAM_FORMAT_GRAY )];

    std::cout<<"Wait before capturing images"<<std::endl;

    // for (int n=0; n<BUFFER_SIZE; n++)
    while (1)
    {
        sem_wait(&S->empty);
        sem_wait(&S->mutex);
        sem_getvalue(&S->full, &n);



        camera.grab();

        camera.retrieve(data, raspicam::RASPICAM_FORMAT_GRAY);

        // send info about capturing an image to logger
        chrono_current_time = std::chrono::system_clock::now().time_since_epoch().count();
        sprintf(client_queue_name+queue_name_len, " captured at %lld", chrono_current_time);
        
        if (mq_send (qd_server, client_queue_name, strlen (client_queue_name) + 1, 0) == -1) {
            perror ("Client: Not able to send message to server");
            continue;
        }

        // allocate shared memory and put image data there



        // sprintf(shm_base, "%s", data);
        memcpy(shm_base+n*SIZE, data, SIZE*sizeof(unsigned char));

        chrono_current_time = std::chrono::system_clock::now().time_since_epoch().count();
        sprintf(client_queue_name+queue_name_len, " saved to shared at %lld", chrono_current_time);
        
        if (mq_send (qd_server, client_queue_name, strlen (client_queue_name) + 1, 0) == -1) {
            perror ("Client: Not able to send message to server");
            continue;
        }


        std::cout<<"Saved to shared memory "<<n<<std::endl;
        (S->buff)[n] = n;
        sem_post(&S->mutex);
        sem_post(&S->full);
        // sleep(PRODUCER_SLEEP_SEC);
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