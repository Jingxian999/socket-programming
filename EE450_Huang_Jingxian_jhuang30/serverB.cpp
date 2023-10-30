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
#include <stdlib.h>
#include <sstream>
#include <fstream>
#include <limits.h>

using namespace std;

/**
 * serverB.cpp
 * B storage server, possesses a database file database.txt
 * in which attribute values regarding information of links
 * are stored
 *
*/

/**
 * Named Constants
 */
#define LOCAL_HOST "127.0.0.1" // Host address
#define serverB_UDP_PORT 22331 // Server B port number
#define MAXDATASIZE 1024 // max number of bytes we can get at once
#define FAIL -1
#define MAX_FILE_SIZE 1000

/**
 * Defined global variables
 */

int sockfd_serverB_UDP; // Server B datagram socket
struct sockaddr_in serverB_addr, aws_addr; // Main server address as a server & as a client

fstream database;
int line_num;

string username1;
string username2;

char recv_buf[MAXDATASIZE]; // Data sent by client: write / compute
char check_resultB[MAXDATASIZE]; // Get from server B Send to Main server



/**
 * Defined functions
 */

// 1. Create UDP socket
void create_serverB_socket();

// 2. Initialize connection with main server
void init_AWS_connection();

// 3. Bind a socket
void bind_socket();

// 4. Receive data from Main server

// 5. Search request data and send result to Main server

// 6. decrypt value from block1,2,3
void decrypt(char recv[]);

// 7. calculate money from block1,2,3
int getBalance(const char username[]);

// 8. encrypt the calculated money
void encrypt(char recv[]);

// 9. get max serial number in blocks.
int get_max_senumb();

// 10. read txt
string readblocks();

/**
 * Step 1: Create server B UDP sockect
 */
void create_serverB_socket() 
{
    sockfd_serverB_UDP = socket(AF_INET, SOCK_DGRAM, 0); // Create a UDP socket
    if (sockfd_serverB_UDP == FAIL) {
        perror("[ERROR] server B: can not open socket");
        exit(1);
    }
}

/**
 * Step 2: Create sockaddr_in struct
 */

void init_AWS_connection()
 {

    // Server B side information
    // Initialize server B IP address, port number
    memset(&serverB_addr, 0, sizeof(serverB_addr)); //  make sure the struct is empty
    serverB_addr.sin_family = AF_INET; // Use IPv4 address family
    serverB_addr.sin_addr.s_addr = inet_addr(LOCAL_HOST); // Host IP address
    serverB_addr.sin_port = htons(serverB_UDP_PORT); // Server B port number
}


/**
 * Step 3: Bind socket with specified IP address and port number
 */
void bind_socket()
 {
    if (::bind(sockfd_serverB_UDP, (struct sockaddr *) &serverB_addr, sizeof(serverB_addr)) == FAIL) {
        perror("[ERROR] Server B: fail to bind Main server UDP socket");
        exit(1);
    }

    printf("The Server B is up and running using UDP on port <%d>. \n", serverB_UDP_PORT);
}

void encrypt(char recv[])
{
    for(int i=0;i<strlen(recv);i++)
    {
        if(recv[i] >= 'A' && recv[i] <= 'Z')
        {
            recv[i] = ((recv[i]-'A')+3)%26+'A';
        }
        else if(recv[i] >= 'a' && recv[i] <= 'z')
        {
            recv[i] = ((recv[i]-'a')+3)%26+'a';
        }
        else if(recv[i] >= '0' && recv[i] <= '9')
        {
            recv[i] = (recv[i]-'0'+3)%10+'0';
        }
    }
}

void decrypt(char recv[])
{
    for(int i=0;i<strlen(recv);i++)
    {
        if(recv[i] >= '3' && recv[i] <= '9')
        {
            recv[i] = ((recv[i]-'0')-3)+'0';
        }
        else if(recv[i] >= '0' && recv[i] <= '2')
        {
            recv[i] = ((recv[i]-'0')+7)+'0';
        }     
    }
}


int getBalance(const char username[])
{
    int balance = 0;
    int appearNum = 0;
    string s = username;
    fstream fin("block2.txt");
    if(!fin){
        cerr<< "cannot open file" << endl;
        exit(1);
    }
    while(!fin.eof()){
        char line[1024];
        fin.getline(line,500);
        string data = line;
        
        string::size_type idx = data.find(s);
        if(idx != string::npos){
            appearNum += 1;
            string num = strtok(line," ");
            string sender = strtok(NULL," ");
            string receiver = strtok(NULL," ");
            char* value = strtok(NULL, " ");
            decrypt(value);
            int amount = atoi(value);
            if(sender == s){
                balance -= amount;
            }else if(s == receiver){
                balance += amount;
            }
            
        }
    }
    if(appearNum == 0)
    {
        balance = INT_MIN;
    }
    
    fin.close();
    return balance;
}

int get_max_senumb()
{
    string s;
    
    int maxsenumb = 0;
    int senumb;
    fstream  fin("block2.txt");
    if(!fin){
        cerr<<"cannot open file"<< endl;
        exit(1);
    }
    
    while(getline(fin,s))
    {
        int length = s.length()+1;
        char line[length];
        strcpy(line,s.c_str());
        string num = strtok(line," ");
        sscanf(num.c_str(),"%d",&senumb);
        if(senumb>maxsenumb)
        {
            maxsenumb = senumb;           
        }
        
    }
    return maxsenumb;
}

string readblocks()
{
    int n = 0;
    fstream fin("block2.txt");
    if(!fin)
    {
        cerr<< "ERROR: Can't open file" << endl;
        exit(1);
    }
    stringstream read_info;
    string s;   
     while(getline(fin,s))
    {
        read_info<< s <<"\n";
        n++;
    }
    line_num = n;
    return read_info.str();
}

int main()
 {

    /******    Step 1: Create server B socket (UDP)  ******/
    create_serverB_socket();
    /******    Step 2: Create sockaddr_in struct  ******/
    init_AWS_connection();
    /******    Step 3: Bind socket with specified IP address and port number  ******/
    bind_socket();

    // Part of codes is from http://c.biancheng.net
    while (true)
     {  
        int count =0;
        memset(recv_buf,'\0',sizeof(recv_buf));
        /******    Step 4: Receive data from Main server  ******/
        socklen_t aws_addr_size = sizeof(aws_addr);
        if (::recvfrom(sockfd_serverB_UDP, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *) &aws_addr,
                       &aws_addr_size) == FAIL) {
            perror("[ERROR] Server B: fail to receive data from main server");
            exit(1);
        }
        printf("The serverB received a request from main server.\n");
         for(int i=0;i<strlen(recv_buf);i++)
         {    
            if(recv_buf[i] == ' ')
            {
                count++;
            }
            
         }
        
        /********************************************************************************************************/
        /*****************************************   Case 1: Check wallet +TXCOINS    *************************************/
        /********************************************************************************************************/
        if (count==0) 
        {
            username1 = recv_buf;
            if(username1 != "WAOLVW")
            {
                int balance = getBalance(username1.c_str());
                //print decrypt balance
    
                stringstream ssBalance;
                string feedack;
                ssBalance<< balance;
                ssBalance >> feedack;
                strncpy(check_resultB, feedack.c_str(), MAXDATASIZE);
                socklen_t aws_addr_size = sizeof(aws_addr);
                encrypt(check_resultB);
                if (sendto(sockfd_serverB_UDP, check_resultB, MAXDATASIZE, 0, (struct sockaddr *) &aws_addr,
                        aws_addr_size) < 0) {
                    perror("[ERROR] Server B: fail to send write result to main server");
                    exit(1);
                }
            
                cout<< "The serverB finished sending the response to the Main server"<<endl;
                memset(recv_buf,'\0',MAXDATASIZE);
                memset(check_resultB,'\0',MAXDATASIZE);   
            }
            else
            {
                string read_lines = readblocks();
                stringstream ListBlock;
                ListBlock<< line_num<<"\n"<< read_lines;
                char B_Record[MAXDATASIZE];
                strcpy(B_Record,ListBlock.str().c_str());
                if (sendto(sockfd_serverB_UDP, B_Record, MAXDATASIZE, 0, (struct sockaddr *) &aws_addr,
                        aws_addr_size) < 0) {
                    perror("[ERROR] Server B: fail to send write result to Main server");
                    exit(1);
                }
                printf("The serverB finished sending the response to the Main Server.\n");
                memset(recv_buf,'\0',MAXDATASIZE);
                memset(B_Record,'\0',MAXDATASIZE);



            }  
        }

            /********************************************************************************************************/
            /***************************************   Case 2: Transfer Coins     *************************************/
            /********************************************************************************************************/
        else if (count ==2)
        {
             stringstream checkResult;
             string check;    
             char txcoins_buf[MAXDATASIZE]; 
             char temp1_buf[MAXDATASIZE];    
             char temp2_buf[MAXDATASIZE];           
             username1 = strtok(recv_buf," ");//sender
             username2 = strtok(NULL," ");//receiver
           
            int sender_balance =getBalance(username1.c_str());
            int receiver_balance = getBalance(username2.c_str());
            stringstream ssend_balance;//for encryption
            stringstream rrecv_balance;
            string feedback1;
            string feedback2;
            ssend_balance<< sender_balance;
            ssend_balance>>feedback1;
            rrecv_balance << receiver_balance;
            rrecv_balance >>feedback2;
            stpncpy(temp1_buf,feedback1.c_str(),MAXDATASIZE);
            stpncpy(temp2_buf,feedback2.c_str(),MAXDATASIZE);
            encrypt(temp1_buf);//encrypt sender balance
            encrypt(temp2_buf);//encrypt receiver balance

            int maxnumb = get_max_senumb();
            checkResult<< temp1_buf<<" "<<temp2_buf<<" "<<maxnumb;
            strncpy(txcoins_buf,checkResult.str().c_str(),sizeof(checkResult));
            socklen_t aws_addr_size = sizeof(aws_addr);

            if (sendto(sockfd_serverB_UDP, txcoins_buf, MAXDATASIZE, 0, (struct sockaddr *) &aws_addr,
                       aws_addr_size) < 0) {
                perror("[ERROR] Server B: fail to send write result to main server");
                exit(1);
            }

            cout<< "The Server B finished sending the response to the Main server"<<endl;
            memset(recv_buf,'\0',MAXDATASIZE);
            memset(txcoins_buf,'\0',MAXDATASIZE); 
                      


        }
        else if(count == 3)
         {
            //successfully excute transactions, write into random server
            ofstream write;
            ifstream read;
        
            string serialNum = strtok(recv_buf, " ");
            string sender = strtok(NULL," ");
            string receiver = strtok(NULL," ");
            char* amount = strtok(NULL," ");
            encrypt(amount);
            write.open("block2.txt",ios::app);
            write<< serialNum<<" "<< sender<<" "<<receiver<< " " <<amount<<endl;
            write.close();
            read.close();
            strcpy(check_resultB,"1");
            socklen_t aws_addr_size = sizeof(aws_addr);
            if (sendto(sockfd_serverB_UDP, check_resultB, MAXDATASIZE, 0, (struct sockaddr *) &aws_addr,
                       aws_addr_size) < 0) {
                perror("[ERROR] Server B: fail to send write result to Main server");
                exit(1);
            }

         }
    }
    close(sockfd_serverB_UDP);
    return 0;
}
