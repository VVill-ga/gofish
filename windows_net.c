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
#include <string.h>
#include <time.h>
#include <ws2tcpip.h>
#include <winsock2.h>

SOCKET ActiveSocket = INVALID_SOCKET;

int port = 0;

/**
 *  Returns: status code. 0 if yay, 1 if aww
*/
int requestSocket(int server_port){
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
        }
    }
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