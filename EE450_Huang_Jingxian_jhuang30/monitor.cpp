#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <string>
#include <iostream>

using namespace std;

/**
 * monitor.cpp
 * A stream socket server, records results of every steps and print them out
 *
*/

/**
 * Named Constants
 */
#define LOCAL_HOST "127.0.0.1" // Host address
#define AWS_Monitor_TCP_PORT 26331 // AWS port number
#define MAXDATASIZE 1024 // max number of bytes we can get at once
#define FAIL -1

/**
 * Defined global variables
 */

string operation;

int sockfd_monitor_TCP; // Monitor stream socket
struct sockaddr_in aws_addr; // AWS server address

char check_buf[MAXDATASIZE]; // First data received from AWS server: write / compute
char check_result[MAXDATASIZE]; // transaction result from main

/**
 * Steps: Defined functions
 */

// 1. Create TCP socket
void create_monitor_socket_TCP();

// 2. Initialize TCP connection with AWS
void init_ServerM_connection(); 

// 3. Send connection request to AWS server
void request_ServerM_connection();

// 4. Receive the first response from AWS, need to determine it is for writing or computing

// 6. For compute,receive the data from AWS

// 7. For compute,receive the result from AWS

/**
 * Step 1: Create monitor socket (TCP stream socket)
 */
void create_monitor_socket_TCP()
 {
    sockfd_monitor_TCP = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if (sockfd_monitor_TCP == FAIL) {
        perror("[ERROR] monitor: can not open monitor socket ");
        exit(1);
    }

}

/**
 * Step 2: Initial TCP connection info
 */
void init_ServerM_connection() 
{

    // *** Beej’s guide to network programming - 9.24. struct sockaddr and pals ***
    // Initialize TCP connection between monitor and AWS server using specified IP address and port number
    memset(&aws_addr, 0, sizeof(aws_addr)); //  make sure the struct is empty
    aws_addr.sin_family = AF_INET; // Use IPv4 address family
    aws_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Source address
    aws_addr.sin_port = htons(AWS_Monitor_TCP_PORT); // AWS server port number
}

/**
 * Step 3: Send connection request to AWS server
 */
void request_ServerM_connection() 
{
    connect(sockfd_monitor_TCP, (struct sockaddr *) &aws_addr, sizeof(aws_addr));

    /*if (connect(sockfd_monitor_TCP, (struct sockaddr *) &aws_addr, sizeof(aws_addr)) == FAIL) {
        perror("[ERROR] monitor: fail to connect with AWS server");
        close(sockfd_monitor_TCP);
        exit(1); // If connection failed, we cannot continue
    }
    */

    // If connection succeed, display boot up message
    printf("The monitor is up and running \n");
}


int main(int argc, char *argv[]) 
{

    /******    Step 1: Create client socket (TCP)  ******/
    create_monitor_socket_TCP();
    /******    Step 2: Initialize connection with main server     *******/
    init_ServerM_connection();
    /******    Step 3: Send connection request to main server     *******/
    request_ServerM_connection();

    operation =argv[1];
    if(operation=="TXLIST")
    {
        strncpy(check_buf, operation.c_str(), MAXDATASIZE);
        //send to the main server
         if (send(sockfd_monitor_TCP, check_buf, sizeof(check_buf), 0) == FAIL)
          {
                perror("[ERROR] client: fail to send input data");
                close(sockfd_monitor_TCP);
                exit(1);
          }
        cout<<"Monitor sent a sorted list request to the main server."<<endl;

        //receive transactions from main server
        if (recv(sockfd_monitor_TCP, check_result, sizeof(check_result), 0) == FAIL)
            {
                perror("[ERROR] clientA: fail to receive write result from main server");
                close(sockfd_monitor_TCP);
                exit(1);
            }
            cout<<"Successfully received a sorted list of transactions from the main server"<<endl;
        
        memset(check_result,'\0',sizeof(check_result)); 

    }

    




    // Close the socket and tear down the connection after we are done using the socket
    close(sockfd_monitor_TCP);

    return 0;
}


