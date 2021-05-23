#include <iostream>		// Include all needed libraries here
#include <wiringPi.h>

// imports for tcp client
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>

int connect_to_server();


using namespace std;		// No need to keep using “std”

int main()
{
    wiringPiSetup();			// Setup the library
    pinMode(0, OUTPUT);		// Configure GPIO0 as an output
    pinMode(1, INPUT);		// Configure GPIO1 as an input
    
    digitalWrite(0, 0);//init output


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