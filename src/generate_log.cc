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
#include <string.h>
#include <sys/types.h>
#include <chrono>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>

#include <cstdio>

#define SERVER_QUEUE_NAME   "/skps-logger"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10


int main (int argc, char **argv)
{
    int64_t chrono_current_time;
    chrono_current_time = std::chrono::system_clock::now().time_since_epoch().count();
    // open file to write logs
    freopen("log.txt", "w", stdout);

    mqd_t qd_server;   // queue descriptor

    printf("Begin writing log at %lld\n", chrono_current_time);

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_server = mq_open (SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror ("Server: mq_open (server)");
        exit (1);
    }
    char in_buffer [MSG_BUFFER_SIZE];

    while (1) {
        // get the messages
        if (mq_receive (qd_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror ("Server: mq_receive");
            exit (1);
        }

        printf ("Server: message received: %s.\n", in_buffer);
    }
}
