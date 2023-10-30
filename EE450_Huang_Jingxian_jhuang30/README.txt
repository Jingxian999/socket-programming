a. JINGXIAN HUANG

b. STU ID:7057619331

c. I have completed check wallet , txcoins and txlist in this project.

d. serverM.cpp 
   Main server
   Create TCP socket with Main server, connect with main server, receive from main server
  
   serverA/B/C.cpp ——Backend server, each work with block1,2,3, receive from main server

 client.cpp- excute function with input,check wallet and transfer

monitor.cpp-list all the transactions.

e. The format of all the messages exchanged.

Phase 1: CHECK WALLET
The main server is up and running.
The main server received input Martin from the client using TCP over port 25331
The main server sent a request to server A 
The main server sent a request to server B 
The main server sent a request to server C 
The main server received transactions from Server A using UDP over port <21331> 
The main server received transactions from Server B using UDP over port <22331> 
The main server received transactions from Server C using UDP over port <23331> 
The main server sent the current balance to the client

Phase2 : TXCOINS
The main server is up and running.
The main server received from Pduwlq to transfer 10 to Fklqpdb using TCP over port 25331
The main server sent a request to server A 
The main server sent a request to server B 
The main server sent a request to server C 
The main server received the feedback from serverA using UDP over port 21331
The main server received the feedback from server B using UDP over port 22331
The main server received the feedback from server C using UDP over port 23331


Phase3: TXLIST
The main server received the feedback from server A using UDP over port 21331
The main server received the feedback from server B using UDP over port 22331
The main server received the feedback from server C using UDP over port 23331
The main server received a sorted list request from the monitor using TCP over port  25331

h.## Reused Code:
1. For TCP connections and UDP connections
Beej's Code: http://www.beej.us/guide/bgnet/

2.For read files into string, I used codes from DelftStack.
https://www.delftstack.com/howto/cpp/read-file-into-string-cpp/
   





