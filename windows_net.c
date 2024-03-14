/**
 *   GO-FISH ONLINE!
 *  By: Will Garrison
 *  
 *  Networking and Chat Logic for Windows
 * 
 *  Sources of Code or Inspiration: 
 *      - Microsoft WinSock Documentation:
 *        https://learn.microsoft.com/en-us/windows/win32/winsock
 * 
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ws2tcpip.h>
#include <winsock2.h>

SOCKET ActiveSocket = INVALID_SOCKET;

int port = 0;
char recievedMsg[4096] = {0};

/**
 *  Returns: status code. 0 if yay, 1 if aww
*/
int requestSocket(int server_port){
    port = server_port;
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if(iResult != 0){
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    if(server_port){
        //Client
        char port_string[5];
        itoa(server_port, port_string, 10);

        struct addrinfo *result = NULL, *ptr = NULL, hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        //NULL to be replaced by address when LAN networking gets implemented
        iResult = getaddrinfo(NULL, port_string, &hints, &result);
        if(iResult != 0){
            printf("getaddrinfo failed: %d\n", iResult);
            WSACleanup();
            return 1;
        }

        //Request Socket
        ActiveSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
        if(ActiveSocket == INVALID_SOCKET){
            printf("Error at socket(): %ld\n", WSAGetLastError());
            freeaddrinfo(result);
            WSACleanup();
            return 1;
        }

        //Request Connection
        iResult = connect(ActiveSocket, result->ai_addr, (int)result->ai_addrlen);
        if(iResult == SOCKET_ERROR){
            closesocket(ActiveSocket);
            ActiveSocket= INVALID_SOCKET;
        }
        freeaddrinfo(result);
        if(ActiveSocket == INVALID_SOCKET){
            printf("Unable to connect to server!\n");
            WSACleanup();
            return 1;
        }
    }
    else{
        //Server
        struct addrinfo *result = NULL, *ptr = NULL, hints;
        ZeroMemory(&hints, sizeof(hints));
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_flags = AI_PASSIVE;

        srand(time(NULL));
        //Try different ports until it works!
        //hehehe
        int tryagain = 1;
        while(tryagain){
            port = (rand() % 4000)+2000;
            char port_string[5];
            itoa(port, port_string, 10);

            iResult = getaddrinfo(NULL, port_string, &hints, &result);
            if(iResult != 0){
                printf("getaddrinfo failed: %d\n", iResult);
                WSACleanup();
                return 1;
            }

            //Request Socket
            ActiveSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
            if(ActiveSocket == INVALID_SOCKET){
                printf("Error at socket(): %ld\n", WSAGetLastError());
                freeaddrinfo(result);
                WSACleanup();
                return 1;
            }

            //Bind to port
            iResult = bind(ActiveSocket, result->ai_addr, (int)result->ai_addrlen);
            int lastError = WSAGetLastError();
            if(iResult == SOCKET_ERROR && lastError != WSAEADDRINUSE){
                // Not "Address in use". More serious problem. Abort!
                printf("Error at bind(): %ld\n", lastError);
                freeaddrinfo(result);
                closesocket(ActiveSocket);
                WSACleanup();
                return 1;
            }
            else if(iResult != SOCKET_ERROR){
                tryagain = 0;
            }

            //Listen on port
            if(listen(ActiveSocket, SOMAXCONN) == SOCKET_ERROR){
                printf("Error on Listen(): %ld\n", WSAGetLastError());
                closesocket(ActiveSocket);
                WSACleanup();
                return 1;
            }
        }
    }
    return 0;
}

// Waits for a connection, then replaces the waiting socket with the direct
// connection to the connected person.
int serverListen(){
    SOCKET tempSocket = accept(ActiveSocket, NULL, NULL);
    if(tempSocket == INVALID_SOCKET){
        printf("Error on Accept(): %ld\n", WSAGetLastError());
        closesocket(ActiveSocket);
        WSACleanup();
        return 1;
    }
    ActiveSocket = tempSocket;
    return 0;
}

int sendMessage(char* msg, int msgLen, bool debug){
    int bytesSent = 0;
    while(bytesSent < msgLen){
        int iResult = send(ActiveSocket, msg+bytesSent, msgLen-bytesSent, 0);
        if(iResult == SOCKET_ERROR){
            printf("Error on Send(): %ld\n", WSAGetLastError());
            closesocket(ActiveSocket);
            WSACleanup();
            return 1;
        }
        bytesSent += iResult;
    }
    if(debug)
        printf("Sent %d bytes\n", bytesSent);
    send(ActiveSocket, "END", 3, 0);
    return 0;
}

int recvMessage(bool debug){
    memset(recievedMsg, 0, 4096);
    int bytesRecieved = 0;
    int iResult = -1;
    while(bytesRecieved < 3 || strcmp(recievedMsg+bytesRecieved-3, "END") != 0){
        iResult = recv(ActiveSocket, recievedMsg+bytesRecieved, 4096-bytesRecieved, 0);
        if(debug)
            printf("SNAPSHOT: %s\n", recievedMsg);
        if(iResult == SOCKET_ERROR){
            printf("Error on Recv(): %ld\n", WSAGetLastError());
            closesocket(ActiveSocket);
            WSACleanup();
            return 1;
        }
        bytesRecieved += iResult;
    }
    if(debug)
        printf("Recieved %d bytes\n", bytesRecieved);
    //Remove "END"
    memset(recievedMsg+bytesRecieved-3, 0, 3);
    return 0;
}

int closeSocket(){
    int iResult = shutdown(ActiveSocket, SD_SEND);
    if(iResult == SOCKET_ERROR){
        printf("Shutdown Failed: %d\n", WSAGetLastError());
        closesocket(ActiveSocket);
        WSACleanup();
        return 1;
    }
    closesocket(ActiveSocket);
    WSACleanup();
    return 0;
}