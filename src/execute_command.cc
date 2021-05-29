#include <iostream>		// Include all needed libraries here
#include <wiringPi.h>

// imports for tcp client
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

#include "semafor.h"

// imports for logger
#include <chrono>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>

int connect_to_server();


using namespace std;		// No need to keep using “std”

int main()
{
    wiringPiSetup();			// Setup the library
    pinMode(0, OUTPUT);		// Configure GPIO0 as an output
    pinMode(1, INPUT);		// Configure GPIO1 as an input
    
    digitalWrite(0, 0);//init output

    // init data for logger
    char client_queue_name [64];
    short queue_name_len;
    char og_publisher_name [64];
    mqd_t qd_server, qd_client;   // queue descriptors
    int64_t chrono_current_time;

    // create the client queue for receiving messages from server
    sprintf (client_queue_name, "/execute-command-%d-", getpid ());
    queue_name_len = strlen(client_queue_name);

    // open queues
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


    // establish server connection
    std::string reply("off");
    int expected_bytes = 3;
    int bytes_recv = 0;
    int bytes_sent;
    int sockFD = connect_to_server();
    if (sockFD < 0){
            printf("cons: Error while connecting to server %s\n", strerror(errno));

            exit(1);
    }

// Main program loop
    while(1)
    {
        do {
            bytes_recv = recv(sockFD, &reply.front(), reply.size(), 0);
        } while(bytes_recv != expected_bytes);

        // send info about capturing an image to logger
        chrono_current_time = std::chrono::system_clock::now().time_since_epoch().count();
        sprintf(client_queue_name+queue_name_len, " got command %s at %lld", reply.c_str(), chrono_current_time);
        
        if (mq_send (qd_server, client_queue_name, strlen (client_queue_name) + 1, 0) == -1) {
            perror ("Client: Not able to send message to server");
            continue;
        }

        if (strcmp(reply.c_str(), "on ")) {
            digitalWrite(0, 1);
        }
        else if (strcmp(reply.c_str(), "off")) {
            digitalWrite(0, 0);
        }


        bytes_sent = send(sockFD, "ok", sizeof("ok"), 0);
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

    // socket() call creates a new socket and returns it's descriptor
    int sockFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (sockFD == -1) {
        std::cerr << "Error while creating socket\n";
        return -4;
    }

    // connect() call tries to establish a TCP connection to the specified server
    int connectR = connect(sockFD, p->ai_addr, p->ai_addrlen);
    if (connectR == -1) {
        close(sockFD);
        std::cerr << "Error while connecting socket\n";
        return -5;
    }

    return sockFD;
}