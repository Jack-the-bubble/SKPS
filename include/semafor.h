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

// }