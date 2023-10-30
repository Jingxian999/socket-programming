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
#include <string.h>
#include <iostream>
#include <vector>
#include <limits.h>
#include <sstream>
#include <fstream>

using namespace std;

#define LOCAL_HOST "127.0.0.1"     // Host address
#define AWS_UDP_PORT 24331         // AWS port number
#define AWS_Client_TCP_PORT 25331  // AWS port number
#define AWS_Monitor_TCP_PORT 26331 // AWS port number
#define ServerA_PORT 21331
#define ServerB_PORT 22331
#define ServerC_PORT 23331
#define MAXDATASIZE 1024 // max number of bytes we can get at once
#define BACKLOG 10       // max number of connections allowed on the incoming queue
#define FAIL -1

int link_id; // Integer input data

double size, length;

int Line_sum;                        // define the sum of line numbes in block1,2,3
int MaxLine_A, MaxLine_B, MaxLine_C; // define the number of line index in block1,2,3

int sockfd_client_TCP, sockfd_monitor_TCP, sockfd_UDP; // Parent socket for client & for monitor & UDP socket
int child_sockfd_client, child_sockfd_monitor;         // Child socket for connection with client and monitor
struct sockaddr_in aws_client_addr, aws_monitor_addr, aws_UDP_addr;
struct sockaddr_in dest_client_addr, dest_monitor_addr, dest_serverA_addr, dest_serverB_addr, dest_serverC_addr; // When AWS works as a client

char input_buf[MAXDATASIZE];     // Input data from client
char check_resultA[MAXDATASIZE]; // Check result of balance of users in server A
char check_resultB[MAXDATASIZE]; // Check result of balance of users in server B
char check_resultC[MAXDATASIZE]; // Check result of balance of users in server C
char check_reuslt[MAXDATASIZE];  // Check result
char tx_result[MAXDATASIZE];     // The check result of whether the operation of transfer coins can be executed
char save_buf[MAXDATASIZE];      // use for successfully excuted txcoins buffer
int maxsenum;                    // the max serial numb in all blocks.
bool sender_exist, recver_exist;

// 1. Create TCP socket w/ client & bind socket
void create_TCP_client_socket();

// 2. Create TCP socket w/ monitor & bind socket
void create_TCP_monitor_socket();

// 3. Create UDP socket
void create_UDP_socket();

// 4. Listen for client
void listen_client();

// 5. Listen for monitor
void listen_monitor();

void init_connection_serverA();

void init_connection_serverB();

void init_connection_serverC();

// 6. Encrpyt the message
void encrypt(char recv[]);

// 7. Decrpyt the message
void decrypt(char recv[]);

void mysort(int n, string input[], string output[]);

void action(void);

/**
 * Step 1: Create TCP socket for client & bind socket
 */
void create_TCP_client_socket()
{
    sockfd_client_TCP = socket(AF_INET, SOCK_STREAM, 0); // Create TCP socket
    if (sockfd_client_TCP == FAIL)
    {
        perror("[ERROR] Main Server: fail to create socket for client");
        exit(1);
    }

    // Initialize IP address, port number
    memset(&aws_client_addr, 0, sizeof(aws_client_addr));    //  make sure the struct is empty
    aws_client_addr.sin_family = AF_INET;                    // Use IPv4 address family
    aws_client_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    aws_client_addr.sin_port = htons(AWS_Client_TCP_PORT);   // Port number for client

    // Bind socket for client with IP address and port number for client
    if (::bind(sockfd_client_TCP, (struct sockaddr *)&aws_client_addr, sizeof(aws_client_addr)) == FAIL)
    {
        perror("[ERROR] Main Server: fail to bind client socket");
        exit(1);
    }
}

/**
 * Step 2: Create TCP socket for monitor & bind socket
 */

void create_TCP_monitor_socket()
{

    sockfd_monitor_TCP = socket(AF_INET, SOCK_STREAM, 0); // TCP socket
    if (sockfd_monitor_TCP == FAIL)
    {
        perror("[ERROR] Main server: fail to create socket for monitor");
        exit(1);
    }

    // Initialize IP address, port number
    memset(&aws_monitor_addr, 0, sizeof(aws_monitor_addr));   //  make sure the struct is empty
    aws_monitor_addr.sin_family = AF_INET;                    // Use IPv4 address family
    aws_monitor_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    aws_monitor_addr.sin_port = htons(AWS_Monitor_TCP_PORT);  // Port number for monitor

    // Bind socket
    if (::bind(sockfd_monitor_TCP, (struct sockaddr *)&aws_monitor_addr, sizeof(aws_monitor_addr)) == FAIL)
    {
        perror("[ERROR] Main Server: fail to bind monitor socket");
        exit(1);
    }
}

/**
 * Step 3: Create UDP socket and bind socket
 */
void create_UDP_socket()
{
    sockfd_UDP = socket(AF_INET, SOCK_DGRAM, 0); // UDP datagram socket
    if (sockfd_UDP == FAIL)
    {
        perror("[ERROR] Main Server: fail to create UDP socket");
        exit(1);
    }

    // Initialize IP address, port number
    memset(&aws_UDP_addr, 0, sizeof(aws_UDP_addr));       //  make sure the struct is empty
    aws_UDP_addr.sin_family = AF_INET;                    // Use IPv4 address family
    aws_UDP_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    aws_UDP_addr.sin_port = htons(AWS_UDP_PORT);          // Port number for client

    // Bind socket
    if (::bind(sockfd_UDP, (struct sockaddr *)&aws_UDP_addr, sizeof(aws_UDP_addr)) == FAIL)
    {
        perror("[ERROR] Main Server: fail to bind UDP socket");
        exit(1);
    }
}

/**
 * Step 4: Listen for incoming connection from client
 */
void listen_client()
{
    if (listen(sockfd_client_TCP, BACKLOG) == FAIL)
    {
        perror("[ERROR] Main server: fail to listen for client socket");
        exit(1);
    }
}

/**
 * Step 5:  Listen for incoming connection from monitor
 */

void listen_monitor()
{
    if (listen(sockfd_monitor_TCP, BACKLOG) == FAIL)
    {
        perror("[ERROR] Main server: fail to listen for monitor socket");
        exit(1);
    }
}

void init_connection_serverA()
{
    // Info about server A
    memset(&dest_serverA_addr, 0, sizeof(dest_serverA_addr));
    dest_serverA_addr.sin_family = AF_INET;
    dest_serverA_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    dest_serverA_addr.sin_port = htons(ServerA_PORT);
}

void init_connection_serverB()
{
    // Info about server B
    memset(&dest_serverB_addr, 0, sizeof(dest_serverB_addr));
    dest_serverB_addr.sin_family = AF_INET;
    dest_serverB_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    dest_serverB_addr.sin_port = htons(ServerB_PORT);
}

void init_connection_serverC()
{
    // Info about server C
    memset(&dest_serverC_addr, 0, sizeof(dest_serverC_addr));
    dest_serverC_addr.sin_family = AF_INET;
    dest_serverC_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST);
    dest_serverC_addr.sin_port = htons(ServerC_PORT);
}

/**
 * Step 6: Encrypt the message from client
 */

void encrypt(char recv[])
{
    for (int i = 0; i < strlen(recv); i++)
    {
        if (recv[i] >= 'A' && recv[i] <= 'Z')
        {
            recv[i] = ((recv[i] - 'A') + 3) % 26 + 'A';
        }
        else if (recv[i] >= 'a' && recv[i] <= 'z')
        {
            recv[i] = ((recv[i] - 'a') + 3) % 26 + 'a';
        }
        else if (recv[i] >= '0' && recv[i] <= '9')
        {
            (recv[i] - '0' + 3) % 10 + '0';
        }
    }
}

/**
 * Step 7: Decrypt the message from client
 */
void decrypt(char recv[])
{
    for (int i = 0; i < strlen(recv); i++)
    {
        if (recv[i] >= '3' && recv[i] <= '9')
        {
            recv[i] = ((recv[i] - '0') - 3) + '0';
        }
        else if (recv[i] >= '0' && recv[i] <= '2')
        {
            recv[i] = ((recv[i] - '0') + 7) + '0';
        }else if(recv[i] >='A' && recv[i] <= 'Z'){
            recv[i] = ((recv[i] - 'A') +23) % 26 + 'A';
        }else if(recv[i] >= 'a' && recv[i] <= 'z'){
            recv[i] = ((recv[i] - 'a') +23) % 26 + 'a';
        }
    }
}

void mysort(int n, string input[], string output[])
{
    char temp_record[1024];
    int temp_num;
    for (int i = 0; i < n; i++)
    {
        stringstream tempSS;
        strcpy(temp_record, input[i].c_str());
        
        sscanf(strtok(temp_record, " "), "%d", &temp_num);
        char* sender = strtok(NULL, " ");
        char* receiver = strtok(NULL, " ");
        char* amount = strtok(NULL," ");
        decrypt(sender);
        decrypt(receiver);
        decrypt(amount);
        tempSS<<temp_num<<" "<<sender<<" "<<receiver<<" "<<amount;
        output[temp_num] = tempSS.str();
    }
}

void action(void)
{
    int count = 0;
    /******    Step 1: Receive input from client: checkwallet / txcoins   ******/
    /*
    int recv_client = recv(child_sockfd_client, input_buf, MAXDATASIZE, 0);
    if (recv_client == FAIL)
    {
        perror("[ERROR] Main server: fail to receive input data from client");
        exit(1);
    }
    */

    for (int i = 0; i < strlen(input_buf); i++)
    {
        if (input_buf[i] == ' ')
        {
            count++;
        }
    }
   // cout << count << endl;
    // encrypt the input;
    encrypt(input_buf);

    // Same data from client, but send to monitor and server A
    char data_buf[MAXDATASIZE];
    strncpy(data_buf, input_buf, strlen(input_buf));

    if (count == 0)
    {
        char operation[] ="TXLIST";
        encrypt(operation);
        string username1 = input_buf;
        //cout << "username1: " << username1 << endl;
        if (username1 != operation)
        {
            cout << "The main server received input"
                 << " " << username1 << "from the client using TCP over port"
                 << " " << AWS_Client_TCP_PORT << endl;
            // Send to server A
            init_connection_serverA();
            if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverA_addr,
                       sizeof(dest_serverA_addr)) == FAIL)
            {
                perror("[ERROR] Main Server: fail to send input data to server A");
                exit(1);
            }
            printf("The main server sent a request to server A \n");
            // Send to server B
            init_connection_serverB();
            if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverB_addr,
                       sizeof(dest_serverB_addr)) == FAIL)
            {
                perror("[ERROR] Main server: fail to send input data to server B");
                exit(1);
            }
            printf("The main server sent a request to server B \n");
            // Send to server C
            init_connection_serverC();
            if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverC_addr,
                       sizeof(dest_serverC_addr)) == FAIL)
            {
                perror("[ERROR] Main server: fail to send input data to server C");
                exit(1);
            }
            printf("The main server sent a request to server C \n");
            // Receive Check result from Server A
            socklen_t dest_serverA_size = sizeof(dest_serverA_addr);
            if (::recvfrom(sockfd_UDP, check_resultA, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverA_addr,
                           &dest_serverA_size) == FAIL)
            {
                perror("[ERROR] Main Server: fail to receive computed result from Server A");
                exit(1);
            }
            printf("The main server received transactions from Server A using UDP over port <%d> \n", ServerA_PORT);
            decrypt(check_resultA);

            // Receive Check result from Server B
            socklen_t dest_serverB_size = sizeof(dest_serverB_addr);
            if (::recvfrom(sockfd_UDP, check_resultB, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverB_addr,
                           &dest_serverB_size) == FAIL)
            {
                perror("[ERROR] Main Server: fail to receive computed result from Server B");
                exit(1);
            }
            printf("The main server received transactions from Server B using UDP over port <%d> \n", ServerB_PORT);
            decrypt(check_resultB);

            // Receive Check result from Server C
            socklen_t dest_serverC_size = sizeof(dest_serverC_addr);
            if (::recvfrom(sockfd_UDP, check_resultC, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverC_addr,
                           &dest_serverC_size) == FAIL)
            {
                perror("[ERROR] Main Server: fail to receive computed result from Server C");
                exit(1);
            }
            printf("The main server received transactions from Server C using UDP over port <%d> \n", ServerC_PORT);
           
            decrypt(check_resultC);

            string resultA = check_resultA;
            int balanceA = atoi(check_resultA);
            int balanceB = atoi(check_resultB);
            int balanceC = atoi(check_resultC);
            int balance;
            string balance2client;

            if (balanceA == INT_MIN && balanceB && balanceB == INT_MIN && balanceC == INT_MIN)
            {
                cout << "There is  no such person." << endl;
                balance2client = "-2147483648";
            }
            else
            {
                if (balanceA == INT_MIN)
                {
                    balanceA = 0;
                }
                if (balanceB == INT_MIN)
                {
                    balanceB = 0;
                }
                if (balanceC == INT_MIN)
                {
                    balanceC = 0;
                }
                balance = 1000 + balanceA + balanceB + balanceC;
                stringstream ss;
                ss << balance;
                balance2client = ss.str();
                /*cout << "A:  " << balanceA << endl; 
                cout << "B:  " << balanceB << endl;
                cout << "C:  " << balanceC << endl;
                cout << "D:  " << balance2client << endl;*/
            }
            // Send response to client
            //cout << "the  balance transfer to client is " << balance2client << endl;
            if (sendto(child_sockfd_client, balance2client.c_str(), sizeof(balance2client), 0, (struct sockaddr *)&dest_client_addr,
                       sizeof(dest_client_addr)) == FAIL)
            {
                perror("[ERROR] Main Server: fail to send current balance to client");
                exit(1);
            }
            printf("The main server sent the current balance to the client\n ");

            memset(data_buf, '\0', MAXDATASIZE);
            memset(check_resultA, '\0', MAXDATASIZE);
            memset(check_resultB, '\0', MAXDATASIZE);
            memset(check_resultC, '\0', MAXDATASIZE);
        }
        else
        { // TXLIST
            /********************************************************************************************************/
            /*******************************************       TXLIST        ****************************************/
            /********************************************************************************************************/

            // Send to server A
            init_connection_serverA();
            if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverA_addr,
                       sizeof(dest_serverA_addr)) == FAIL)
            {
                perror("[ERROR] Main Server: fail to send input data to server A");
                exit(1);
            }
            printf("The main server sent a request to server A \n");
            // Send to server B
            init_connection_serverB();
            if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverB_addr,
                       sizeof(dest_serverB_addr)) == FAIL)
            {
                perror("[ERROR] Main server: fail to send input data to server B");
                exit(1);
            }
            printf("The main server sent a request to server B \n");
            // Send to server C
            init_connection_serverC();
            if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverC_addr,
                       sizeof(dest_serverC_addr)) == FAIL)
            {
                perror("[ERROR] Main server: fail to send input data to server C");
                exit(1);
            }
            printf("The main server sent a request to server C \n");
            // Receive Check result from Server A
            socklen_t dest_serverA_size = sizeof(dest_serverA_addr);
            if (::recvfrom(sockfd_UDP, check_resultA, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverA_addr,
                           &dest_serverA_size) == FAIL)
            {
                perror("[ERROR] Main Server: fail to receive computed result from Server A");
                exit(1);
            }
            cout << "The main server received the feedback from server A using UDP over port " << ServerA_PORT << endl;

            sscanf(strtok(check_resultA, "\n"), "%d", &MaxLine_A);
            string records_A[MaxLine_A];
            for (int i = 0; i < MaxLine_A; i++)
            {
                records_A[i] = strtok(NULL, "\n"); // extract every line
            }

            // Receive Check result from Server B
            socklen_t dest_serverB_size = sizeof(dest_serverB_addr);
            if (::recvfrom(sockfd_UDP, check_resultB, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverB_addr,
                           &dest_serverB_size) == FAIL)
            {
                perror("[ERROR] Main Server: fail to receive computed result from Server B");
                exit(1);
            }
            cout << "The main server received the feedback from server B using UDP over port " << ServerB_PORT << endl;

            sscanf(strtok(check_resultB, "\n"), "%d", &MaxLine_B);
            string records_B[MaxLine_B];
            for (int i = 0; i < MaxLine_B; i++)
            {
                records_B[i] = strtok(NULL, "\n"); // extract every line
            }

            // Receive Check result from Server C
            socklen_t dest_serverC_size = sizeof(dest_serverC_addr);
            if (::recvfrom(sockfd_UDP, check_resultC, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverC_addr,
                           &dest_serverC_size) == FAIL)
            {
                perror("[ERROR] Main Server: fail to receive computed result from Server C");
                exit(1);
            }
            cout << "The main server received the feedback from server C using UDP over port " << ServerC_PORT << endl;
            sscanf(strtok(check_resultC, "\n"), "%d", &MaxLine_C);
            string records_C[MaxLine_C];
            for (int i = 0; i < MaxLine_C; i++)
            {
                records_C[i] = strtok(NULL, "\n"); // extract every line
            }

            Line_sum = MaxLine_A + MaxLine_B + MaxLine_C;
           // cout<<"Line num  is  "<<Line_sum<<endl;
            string Listall[Line_sum + 1];
            mysort(MaxLine_A, records_A, Listall);
            mysort(MaxLine_B, records_B, Listall);
            mysort(MaxLine_C, records_C, Listall);

           // cout << Listall << endl; // for testing only

            ofstream writeFile("txchain.txt");
            for (int i = 1; i < Line_sum + 1; i++)
            {
                writeFile << Listall[i] << endl;
            }
            writeFile.close();

            // Send response to monitor
            if (send(child_sockfd_monitor, "1", 1, 0) == FAIL)
            {
                perror("[ERROR] Main Server: fail to send current balance to client");
                exit(1);
            }
            cout << "The main server received a sorted list request from the monitor using TCP over port  " << AWS_Client_TCP_PORT << endl;
            memset(data_buf, '\0', sizeof(data_buf));
            memset(check_resultA, '\0', MAXDATASIZE);
            memset(check_resultB, '\0', MAXDATASIZE);
            memset(check_resultC, '\0', MAXDATASIZE);
        }
    }
    else if (count == 2)//txcoins
    {
                string username1 = strtok(input_buf, " ");
                string username2 = strtok(NULL, " ");
                string txcoins = strtok(NULL, "");
                int txvalue;
                sscanf(txcoins.c_str(), "%d", &txvalue);//decrypt

                cout << "The main server received from " << username1 << " to transfer " << txcoins << " to "
                     << username2
                     << " using TCP over port"
                     << " " << AWS_Client_TCP_PORT << endl;

                // Send to monitor
                /*init_connection_serverA();
                if (sendto(child_sockfd_monitor, data_buf, sizeof(data_buf), 0, (struct sockaddr *)&dest_monitor_addr,
                           sizeof(dest_monitor_addr)) == FAIL)
                {
                    perror("[ERROR] Main Server: fail to send input data to monitor");
                    exit(1);
                }*/

                // Send to server A
                init_connection_serverA();
                if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverA_addr,
                           sizeof(dest_serverA_addr)) == FAIL)
                {
                    perror("[ERROR] Main Server: fail to send input data to server A");
                    exit(1);
                }
                printf("The main server sent a request to server A \n");
                // Send to server B
                init_connection_serverB();
                if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverB_addr,
                           sizeof(dest_serverB_addr)) == FAIL)
                {
                    perror("[ERROR] Main server: fail to send input data to server B");
                    exit(1);
                }
                printf("The main server sent a request to server B \n");
                // Send to server C
                init_connection_serverC();
                if (sendto(sockfd_UDP, data_buf, sizeof(data_buf), 0, (const struct sockaddr *)&dest_serverC_addr,
                           sizeof(dest_serverC_addr)) == FAIL)
                {
                    perror("[ERROR] Main server: fail to send input data to server C");
                    exit(1);
                }
                printf("The main server sent a request to server C \n");
                // Receive Check result from Server A
                socklen_t dest_serverA_size = sizeof(dest_serverA_addr);
                if (::recvfrom(sockfd_UDP, check_resultA, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverA_addr,
                               &dest_serverA_size) == FAIL)
                {
                    perror("[ERROR] Main Server: fail to receive computed result from Server A");
                    exit(1);
                }

                char *sender_ba_inA = strtok(check_resultA, " "); // sender's balance in A
                //cout << sender_ba_inA << endl;
                char *receiver_ba_inA = strtok(NULL, " "); // receiver's balance in A
                char *maxsenuminA = strtok(NULL, " ");     // get max serial numb in  A

                decrypt(sender_ba_inA);
                decrypt(receiver_ba_inA);
                // decrypt(maxsenuminA);

                cout << "The main server received the feedback from serverA using UDP over port " << ServerA_PORT << endl;

                // Receive Check result from Server B
                socklen_t dest_serverB_size = sizeof(dest_serverB_addr);
                if (::recvfrom(sockfd_UDP, check_resultB, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverB_addr,
                               &dest_serverB_size) == FAIL)
                {
                    perror("[ERROR] Main Server: fail to receive computed result from Server B");
                    exit(1);
                }
                cout << "The main server received the feedback from server B using UDP over port " << ServerB_PORT << endl;
                char *sender_ba_inB = strtok(check_resultB, " "); // sender's balance in B
                char *receiver_ba_inB = strtok(NULL, " ");        // receiver's balance in B
                char *maxsenuminB = strtok(NULL, " ");            // get max serial numb in B

                decrypt(sender_ba_inB);
                decrypt(receiver_ba_inB);
                // decrypt(maxsenuminB);;

                // Receive Check result from Server C
                socklen_t dest_serverC_size = sizeof(dest_serverC_addr);
                if (::recvfrom(sockfd_UDP, check_resultC, MAXDATASIZE, 0, (struct sockaddr *)&dest_serverC_addr,
                               &dest_serverC_size) == FAIL)
                {
                    perror("[ERROR] Main Server: fail to receive computed result from Server C");
                    exit(1);
                }
                cout << "The main server received the feedback from server C using UDP over port " << ServerC_PORT << endl;
                char *sender_ba_inC = strtok(check_resultC, " "); // sender's balance in B
                char *receiver_ba_inC = strtok(NULL, " ");        // receiver's balance in B
                char *maxsenuminC = strtok(NULL, " ");            // get max serial numb in B

                decrypt(sender_ba_inC);
                decrypt(receiver_ba_inC);
                // decrypt(maxsenuminC);

                int sender_balanceA = atoi(sender_ba_inA);
                int sender_balanceB = atoi(sender_ba_inB);
                int sender_balanceC = atoi(sender_ba_inC);
                int sender_balance; // current balance
                int recver_balanceA = atoi(receiver_ba_inA);
                int recver_balanceB = atoi(receiver_ba_inB);
                int recver_balanceC = atoi(receiver_ba_inC);
                int recver_balance;
                // Check whether sender exists and get the balance of sender.
                if (sender_balanceA == INT_MIN && sender_balanceB == INT_MIN && sender_balanceC == INT_MIN)
                {
                    sender_balance = INT_MIN;
                    sender_exist = false;
                }
                else
                {
                    if (sender_balanceA == INT_MIN)
                        sender_balanceA = 0;
                    if (sender_balanceB == INT_MIN)
                        sender_balanceB = 0;
                    if (sender_balanceC == INT_MIN)
                        sender_balanceC = 0;
                    sender_balance = 1000 + sender_balanceA + sender_balanceB + sender_balanceC;
                    sender_exist = true;
                }
                // Check whether receiver exists
                if (recver_balanceA == INT_MIN && recver_balanceB == INT_MIN && recver_balanceC == INT_MIN)
                {
                    recver_exist = false;
                }
                else
                {
                    recver_exist = true;
                }

                int maxA = atoi(maxsenuminA);
                int maxB = atoi(maxsenuminB);
                int maxC = atoi(maxsenuminC);

                maxsenum = max(maxA, max(maxB, maxC));

                if (!sender_exist && !recver_exist)
                {
                    // case 1: both sender and reciver are not in the list
                    sprintf(tx_result, "-1");
                }
                else if (!sender_exist)
                {
                    // case 2: sender is not in the list
                    sprintf(tx_result, "-2");
                }
                else if (!recver_exist)
                {
                    // case 3: receiver is not in the list
                    sprintf(tx_result, "-3");
                }
                else if (sender_balance < txvalue)
                { // case 4:what the sender have is smaller than the transction money

                    stringstream status;
                    status << "-4 " << sender_balance;       // print current balance if unable tosend
                    strcpy(tx_result, status.str().c_str()); // status :-4 balance
                }
                else
                {
                    // case 5: both sender and reciver are in the list
                    sender_balance -= txvalue;
                    stringstream status;
                    stringstream success_content;
                    // count in serverA/B/C =3
                    success_content << maxsenum + 1 << " " << username1 << " " << username2 << " " << txcoins;
                    strcpy(save_buf, success_content.str().c_str());
                    //cout << save_buf << endl;//for testing only
                    // Randomly choose a block.txt and save the transaction
                    srand((unsigned)time(NULL));
                    int server = rand() % 3;
                    if (server == 0) // server =0 ,send to serverA
                    {
                        if (sendto(sockfd_UDP, save_buf, sizeof(save_buf), 0, (const struct sockaddr *)&dest_serverA_addr,
                                   sizeof(dest_serverA_addr)) == FAIL)
                        {
                            perror("[ERROR] Main_server: fail to send transaction data to server A");
                            exit(1);
                        }
                        if (::recvfrom(sockfd_UDP, check_resultA, sizeof(check_resultA), 0, (struct sockaddr *)&dest_serverA_addr,
                                       &dest_serverA_size) == FAIL)
                        {
                            perror("[ERROR] Main_server: fail to receive transaction data from Server A");
                            exit(1);
                        }
                        status << "1 " << sender_balance;
                        strcpy(tx_result, status.str().c_str());
                        memset(check_resultA, '\0', MAXDATASIZE);
                    }
                    if (server == 1) // server =1 ,send to serverB
                    {
                        if (sendto(sockfd_UDP, save_buf, sizeof(save_buf), 0, (const struct sockaddr *)&dest_serverB_addr,
                                   sizeof(dest_serverB_addr)) == FAIL)
                        {
                            perror("[ERROR] Main_server: fail to send transaction data to server B");
                            exit(1);
                        }
                        if (::recvfrom(sockfd_UDP, check_resultB, sizeof(check_resultB), 0, (struct sockaddr *)&dest_serverB_addr,
                                       &dest_serverB_size) == FAIL)
                        {
                            perror("[ERROR] Main_server: fail to receive transaction data from Server B");
                            exit(1);
                        }
                        status << "1 " << sender_balance;
                        strcpy(tx_result, status.str().c_str());
                        memset(check_resultB, '\0', MAXDATASIZE);
                    }
                    if (server == 2) // server =2 ,send to serverC
                    {
                        if (sendto(sockfd_UDP, save_buf, sizeof(save_buf), 0, (const struct sockaddr *)&dest_serverC_addr,
                                   sizeof(dest_serverC_addr)) == FAIL)
                        {
                            perror("[ERROR] Main_server: fail to send transaction data to server C");
                            exit(1);
                        }
                        if (::recvfrom(sockfd_UDP, check_resultC, sizeof(check_resultC), 0, (struct sockaddr *)&dest_serverC_addr,
                                       &dest_serverC_size) == FAIL)
                        {
                            perror("[ERROR] Main_server: fail to receive transaction data from Server C");
                            exit(1);
                        }
                        status << "1 " << sender_balance;
                        strcpy(tx_result, status.str().c_str());
                        memset(check_resultC, '\0', MAXDATASIZE);
                    }
                }
                // Send write response to client

                if (send(child_sockfd_client, tx_result, sizeof(tx_result), 0) == FAIL)
                {
                    perror("[ERROR] Main Server: fail to send transfer balance to client");
                    exit(1);
                }

               
                printf("The main server sent the result of the transaction to the client ");

                memset(tx_result, '\0', sizeof(tx_result));
                memset(data_buf, '\0', MAXDATASIZE);
                memset(check_resultA, '\0', MAXDATASIZE);
                memset(check_resultB, '\0', MAXDATASIZE);
                memset(check_resultC, '\0', MAXDATASIZE);
    }

}

int main()
{
    signal(SIGPIPE, SIG_IGN); // ignore the SIGPIPE signal first
    fd_set read_fds;
    fd_set temp_fds;
    int fdmax;
    struct timeval tv;

    create_TCP_client_socket();
    create_TCP_monitor_socket();
    listen_client();
    listen_monitor();
    create_UDP_socket();
    printf("The main server is up and running.\n");

    FD_ZERO(&read_fds);
    FD_SET(sockfd_client_TCP, &read_fds);
    FD_SET(sockfd_monitor_TCP, &read_fds);

    fdmax = max(sockfd_client_TCP, sockfd_monitor_TCP);
    child_sockfd_client = -1;
    child_sockfd_monitor = -1;

    while (true)
    {
        temp_fds = read_fds;
        socklen_t client_addr_size = sizeof(dest_client_addr);
        socklen_t monitor_addr_size = sizeof(dest_monitor_addr);
        tv.tv_sec = 20; // select timeout: 20s
        tv.tv_usec = 0;

        // Accept listening socket (parent)

        int ret = select(fdmax + 1, &temp_fds, NULL, NULL, &tv);

        if (ret < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("select error!\n");
            printf("select fatal error, exit!\n");
            exit(1);
        }
        else if (ret == 0)
        {
            continue;
        }

        if (FD_ISSET(sockfd_client_TCP, &temp_fds))
        {
            if (child_sockfd_client == -1)
            {

                child_sockfd_client = ::accept(sockfd_client_TCP, (struct sockaddr *)&dest_client_addr, &client_addr_size);
                
                if (child_sockfd_client < 0)
                {
                    //cout << "here3" << endl;
                    perror("ERROR: Client cannot be accepted.\n");
                    exit(1);
                }
                // add child_sockfd_client to read_fds
                FD_SET(child_sockfd_client, &read_fds);
                
                fdmax = max(child_sockfd_client, fdmax);
                
            }
            else
            {
                cout << "Can not run client and monitor at the same time" << endl;
                struct sockaddr_in tmpaddr;
                socklen_t tmpaddrsize = sizeof(tmpaddr);
                ;
                close(accept(sockfd_client_TCP, (struct sockaddr *)&tmpaddr, &tmpaddrsize));
            }
        }
        else if (FD_ISSET(sockfd_monitor_TCP, &temp_fds))
        {
            if (child_sockfd_monitor == -1)
            {
              
                child_sockfd_monitor = ::accept(sockfd_monitor_TCP, (struct sockaddr *)&dest_monitor_addr, &monitor_addr_size);
                if (child_sockfd_monitor < 0)
                {
                    perror("ERROR: Monitor cannot be accepted.\n");
                    exit(1);
                }
                // add child_sockfd_clientB to read_fds
                FD_SET(child_sockfd_monitor, &read_fds);
                fdmax = max(child_sockfd_monitor, fdmax);
            }
            else
            {
                cout << "Can not run client and monitor at the same time" << endl;
                struct sockaddr_in tmpaddr;
                socklen_t tmpaddrsize = sizeof(tmpaddr);
                ;
                close(accept(sockfd_monitor_TCP, (struct sockaddr *)&tmpaddr, &tmpaddrsize));
            }
        }
        else if (FD_ISSET(child_sockfd_client, &temp_fds))
        {

            int ret = recv(child_sockfd_client, input_buf, MAXDATASIZE, 0);

            if (ret < 0)
            {
                if (errno == EINTR)
                    continue;
                perror("recv error!\n");
                cout << "recv child_sockfd_client fatal error, exit!" << endl;
                exit(1);
            }
            else if (ret == 0)
            {
                FD_CLR(child_sockfd_client, &read_fds);
                close(child_sockfd_client);
                child_sockfd_client = -1;
                
                continue;
            }
            action();
        }
        else if (FD_ISSET(child_sockfd_monitor, &temp_fds))
        {
            int ret = recv(child_sockfd_monitor, input_buf, MAXDATASIZE, 0);
            if (ret < 0)
            {
                if (errno == EINTR)
                    continue;
                perror("recv error!\n");
                exit(1);
            }
            else if (ret == 0)
            {
                FD_CLR(child_sockfd_monitor, &read_fds);
                close(child_sockfd_monitor);
                child_sockfd_monitor = -1;
                continue;
                cout << "monitor is up and running." << endl;
            }
            action();
        }
        else
        {
            cout << "select error" << endl;
            exit(1);
        }
    }
    // Close parent socket
    close(sockfd_UDP);
    return 0;
}