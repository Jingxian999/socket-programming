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
#include <iostream>
#include <string>
#include <vector>
#include <limits.h>

using namespace std;

/**
 * Client.cpp
 * A client, sends write and compute data to main server 
 *
*/


/**
 * Named Constants
 */
#define LOCAL_HOST "127.0.0.1" // Host address
#define AWS_Client_TCP_PORT 25331 // main server port number
#define MAXDATASIZE 1024 // max number of bytes we can get at once
#define FAIL -1 // socket fails if result = -1


/**
 * Defined global variables
 */
string username1; 
string username2;


int sockfd_client_TCP; // Client socket
struct sockaddr_in aws_addr; // main server address
char write_buf[MAXDATASIZE]; // Store input to write (send to main server)
char tx_buf[MAXDATASIZE]; // Store input to compute (send to main server)
char write_result[MAXDATASIZE]; // Write result from main server
char compute_result[MAXDATASIZE]; // Compute result from main server

/**
 * Steps (defined functions):
 */

// 1. Create TCP socket
void create_client_socket_TCP();

// 2. Initialize TCP connection with main server
void init_AWS_connection();

// 3. Send connection request to main server
void request_AWS_connection();

// 4. Send data to main server (write / compute)

// 5. Get result back from main server Server (write result / computed result)


/**
 * Step 1: Create client socket (TCP stream socket)
 */
void create_client_socket_TCP()
 {
    sockfd_client_TCP = socket(AF_INET, SOCK_STREAM, 0); // Create a TCP socket
    if (sockfd_client_TCP == FAIL) {
        perror("[ERROR] client: can not open client socket ");
        exit(1);
    }

}

/**
 * Step 2: Initial TCP connection info
 */
void init_AWS_connection()
 {

    // *** Beej’s guide to network programming - 9.24. struct sockaddr and pals ***
    // Initialize TCP connection between client and main server using specified IP address and port number
    memset(&aws_addr, 0, sizeof(aws_addr)); //  make sure the struct is empty
    aws_addr.sin_family = AF_INET; // Use IPv4 address family
    aws_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Source address
    aws_addr.sin_port = htons(AWS_Client_TCP_PORT); // main server port number
}

/**
 * Step 3: Send connection request to main server 
 */
void request_AWS_connection() 
{
    connect(sockfd_client_TCP, (struct sockaddr *) &aws_addr, sizeof(aws_addr));
    // If connection succeed, display boot up message
    printf("The client is up and running \n");
}


int main(int argc, char *argv[]) 
{

    create_client_socket_TCP();
    init_AWS_connection();
    request_AWS_connection();
    username1 = argv[1];

    // Check the validity of operation
    // check wallet 2 input,Txcoins 4 inputs


    /********************************************************************************************************/
    /*****************************************   Case 1: Check Wallet     *************************************/
    /********************************************************************************************************/
    if (argc == 2) 
    {
        string str1 = username1 ;
        strncpy(write_buf, str1.c_str(), MAXDATASIZE);

        /******    Step 4:  Send data to main server    *******/
        if (send(sockfd_client_TCP, write_buf, sizeof(write_buf), 0) == FAIL) {
            perror("[ERROR] client: fail to send input data");
            close(sockfd_client_TCP);
            exit(1);
        }
        
        cout<< username1<<" sent a balance enquiry request to the main server" <<endl;

        /******    Step 5:  Get check result back from main Server    *******/
        if (recv(sockfd_client_TCP, write_result, sizeof(write_result), 0) == FAIL) {
            perror("[ERROR] client: fail to receive write result from main server");
            close(sockfd_client_TCP);
            exit(1);
        }
        string result = write_result;
        //cout<<"the current balance is "<< result<<endl;
        
        if (result == "-2147483648") 
        {
           cout<<username1<<" is not part of the network"<<endl;
        }
        else
            cout<<"The current balance of "<<username1<<" is "<<result<<" txcoins"<<endl;

    }

        /********************************************************************************************************/
        /***************************************   Case 2: Transfer Coins     *************************************/
        /********************************************************************************************************/
    else if (argc == 4)
     {
         username2 = argv[2];
         string txcoins = argv[3];

         // Make sure the transfer amount is number
        for(int i = 0;i<strlen(argv[3]);i++)
        {
            if(!isdigit(txcoins[i]))
            {
                printf("Please enter a integer as the number to be transferred!\n ");
                exit(1);
            }      
        }

        string str2 =  username1 + " " + username2 + " " + txcoins;  
        strncpy(tx_buf, str2.c_str(), MAXDATASIZE);

        /******    Step 4:  Send transfer coins data to main server     *******/
        if (send(sockfd_client_TCP, tx_buf, sizeof(tx_buf), 0) == FAIL) {
            perror("[ERROR] client: fail to send input data");
            close(sockfd_client_TCP);
            exit(1);
        }
        cout<< username1<<" sent a balance enquiry request to the main server."<<endl;


        /******    Step 5:  Get result back from main server    *******/
        if (recv(sockfd_client_TCP, compute_result, sizeof(compute_result) + 1, 0) == FAIL) {
            perror("[ERROR] client: fail to receive result from main server");
            close(sockfd_client_TCP);
            exit(1);
        }

        string status = strtok(compute_result," ");
        if (status == "-1") //case -1: both sender and reciver are not in the list
        {
            cout<< "Unable to procedd with the transaction as "<<username1<<" and "<<username2 <<"are not part of the network."<<endl;

        }
        if (status == "-2")//case -2: sender is not in the list
        {
            cout<<"Unable to proceed with the transaction as "<<username1<<"  is not part of the network"<<endl;
        }
        if(status =="-3")//case -3: receiver is not in the list
        {
            cout<<"Unable to proceed with the transaction as "<<username2<<"  is not part of the network"<<endl;
        }
        if(status =="-4")//case -4:what the sender have is smaller than the transction money
        {
            string sender_balance = strtok(NULL," ");
            cout<<username1<<" was unable to tansfer "<< txcoins<<" txcoins to "<< username2
            <<" because of insufficient balance"<<endl;
            cout<<" The current balance of "<<username1<<" is "<<sender_balance<<" txcoins"<<endl;
        }
        if(status =="1")//case 1:successfully transfer money
        {
            string recv_balance = strtok(NULL," ");
            cout<<username1<<" successfully transferred "<< txcoins <<" txcoins to "<<username2<<endl;
            cout<<"The current balance of "<<username1<<" is "<<recv_balance<<" txcoins."<<endl;
        }
    // Close the socket and tear down the connection after we are done using the socket
    close(sockfd_client_TCP);
    return 0;
}

}




