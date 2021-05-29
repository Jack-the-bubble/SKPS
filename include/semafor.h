#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>

#include <memory>
#include <cstring>
#include <raspicam/raspicam.h>
#include <iostream>

// imports for logger
#include <chrono>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>


// namespace SEMAFOR{
    #define BUFFER_SIZE 5
    #define CONSUMER_SLEEP_SEC 2
    #define PRODUCER_SLEEP_SEC 1
    #define KEY 1111

    #define SERVER_QUEUE_NAME   "/skps-logger"
    #define QUEUE_PERMISSIONS 0660
    #define MAX_MESSAGES 10
    #define MAX_MSG_SIZE 256
    #define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

    // A structure to store BUFER and semaphores for synchronization
    typedef struct
    {
        int buff[BUFFER_SIZE];
        sem_t mutex, empty, full;

    } MEM;

    // Method for shared memory allocation
    MEM *memory()
    {
        key_t key = KEY;
        int shmid;
        shmid = shmget(key, sizeof(MEM), IPC_CREAT | 0666);
        return (MEM *) shmat(shmid, NULL, 0);
    }

    void init()
    {
        // Initialize structure pointer with shared memory
        MEM *M = memory();

        // Initialize semaphores
        sem_init(&M->mutex,1,1);
        sem_init(&M->empty,1,BUFFER_SIZE);
        sem_init(&M->full,1,0);
    }

    void send_log_message(mqd_t qd_server, char client_queue_name[MAX_MSG_SIZE], short queue_name_len) {
        // int64_t chrono_current_time = std::chrono::system_clock::now().time_since_epoch().count();

        
        if (mq_send (qd_server, client_queue_name, strlen (client_queue_name) + 1, 0) == -1) {
            perror ("Client: Not able to send message to server");
        }
    }

// }