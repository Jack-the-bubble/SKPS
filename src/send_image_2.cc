/* sources used to create this project
https://www.sarathlakshman.com/2010/10/15/producer-consumer-problem-using-posix-semaphores
https://riptutorial.com/cplusplus/example/24000/hello-tcp-client
http://www.cse.psu.edu/~deh25/cmpsc473/notes/OSC/Processes/shm.html

Authors:
Marcin Skrzypkowski
Bartosz Bok
*/

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

// imports for tcp client
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>


int send_image(int socket_fd, unsigned char *data, int chunk_size, int chunk_num);
int connect_to_server();

int main(int argc, char** argv) {
    // init data for semaphores
    int n;
    int max_iter = BUFFER_SIZE;
    MEM *S = memory();

    // open queues
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    // init data for logger
    char client_queue_name [MAX_MSG_SIZE];
    short queue_name_len;
    char og_publisher_name [64];
    mqd_t qd_server;   // queue descriptors
    int64_t chrono_current_time;

    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror ("Client: mq_open (server)");
        exit (1);
    }
    
    // create the client queue for updating logger process
    sprintf (client_queue_name, "/send-image-%d-", getpid ());
    queue_name_len = strlen(client_queue_name);

    // init data for shared memory
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
        exit(1);
    }

    // prepare data for sending to tcp server
    std::string reply;
    int chunk_size = 1024;
    int chunk_num = SIZE/chunk_size;

    // establish tcp connection with server
    int sockFD = connect_to_server();
    if (sockFD < 0){
        printf("cons: Error while connecting to server %s\n", strerror(errno));

        exit(1);
    }

    while(1)
    {
        // wait for capture process to save image in shared memory using mutex
        sem_wait(&S->full);
        sem_wait(&S->mutex);
        sem_getvalue(&S->full, &n);

        memcpy(data, shm_base+n*SIZE, SIZE*sizeof(unsigned char));

        // send info about capturing an image to logger
        chrono_current_time = std::chrono::system_clock::now().time_since_epoch().count();
        sprintf(client_queue_name+queue_name_len, " got from shared at %lld", chrono_current_time);
        send_log_message(qd_server, client_queue_name, queue_name_len);
        
        // send image in chunks
        auto nbytes_send = send_image(sockFD, data, chunk_size, chunk_num);
        if (nbytes_send < SIZE)
            break;
        
        // send info about capturing an image to logger
        chrono_current_time = std::chrono::system_clock::now().time_since_epoch().count();
        sprintf(client_queue_name+queue_name_len, " sent image at %lld", chrono_current_time);
        send_log_message(qd_server, client_queue_name, queue_name_len);

        // signal other processes about sent image
        sem_post(&S->mutex);
        sem_post(&S->empty);
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

int connect_to_server() {
    auto &ipAddress = "10.42.0.1";
    auto &portNum   = "9999";
    
    addrinfo hints, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    int gAddRes = getaddrinfo(ipAddress, portNum, &hints, &p);
    if (gAddRes != 0) {
        std::cerr << gai_strerror(gAddRes) << "\n";
        return -2;
    }

    if (p == NULL) {
        std::cerr << "No addresses found\n";
        return -3;
    }

    // create linux socket
    int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockFD == -1) {
        std::cerr << "Error while creating socket\n";
        return -4;
    }

    // establish connection with server
    int connectR = connect(sockFD, p->ai_addr, p->ai_addrlen);
    if (connectR == -1) {
        close(sockFD);
        std::cerr << "Error while connecting socket\n";
        return -5;
    }

    return sockFD;
}

int send_image(int socket_fd, unsigned char *data, int chunk_size, int chunk_num) {
    int bytes_sent = 0;
    int current_bytes;
    std::string reply("no");
    for (int i=0; i<chunk_num; i++){
        do {
            // send chunk of an image and check if correct number of bytes was actually sent
            current_bytes = send(socket_fd, (char *)data+i*chunk_size, chunk_size, 0);
            if (current_bytes < chunk_size){
                return -1;
            }
            auto bytes_recv = recv(socket_fd, &reply.front(), reply.size(), 0);

        }   while (strcmp(reply.c_str(), "ok"));

        bytes_sent += current_bytes;


    }
    std::cout<<"Sent image"<<std::endl;

    return bytes_sent;
}
