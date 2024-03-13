/**
 *   GO-FISH ONLINE!
 *  By: Will Garrison
 * 
 *  Tie-it-all-Together Logic
 * 
 *  Note to graders: The following code is entirely my own. I became familiar
 *  with C's socket API in OS1 last year, so I opted to write this program in
 *  C instead of Python.
 * 
 *  Sources of Code or Inspiration: 
 * 
*/

//Game Logic
#include "./gofish.c"

//Platform-Specific Chat Logic
#if defined(_WIN32) || defined(__MSDOS__)
    #include "./windows_net.c"
#else
    #include "./linux_net.c"
#endif

char buf[4096];
bool noUnicode = false;
bool debug = false;
int server_port = 0;

/**
 *      Take Input From User
 * Returns 0 on successful input taking
 * Returns 1 if input is /q (user wants to quit the program)
*/
int takeInput(){
    printf(":>> ");
    fflush(stdout);
    memset(buf, 0, 4096);
    scanf("%4095[^\n]", buf);
    getchar();
    fflush(stdout);
    sendMessage(buf, debug);
    if(strcmp(buf, "/q") == 0){
        printf("|\n| The connection has been terminated.\n*\n");
        return 1;
    }
    return 0;
}


/**
 *      The Main Function!
 * 
 *  1. Parse arguments
 *      - Do we have --no-unicode?
 *      - Are we client or server?
 *  2. Set up network connection
 *      - If we are server, find open port
 *      - If we are client, attempt to connect
 *  3. Welcome user and initiate chat
 *  4. Parse messages and optionally start game
*/
int main(int argc, char *argv[]){
    // 1. Parse Arguments
    for(int i = 0; i < argc; i++){
        if(strcmp(argv[i], "--no-unicode") == 0)
            noUnicode = true;
        if(strcmp(argv[i], "--debug") == 0)
            debug = true;
        else if(atoi(argv[i]) != 0)
            server_port = atoi(argv[i]);
    }

    // 2. Setup the network stuff
    if(server_port != 0 && (server_port > 6000 || server_port < 2000)){
        printf("Error: That port was not generated by this program!\n");
        return 1;
    }
    int socketStatus = requestSocket(server_port);
    if(socketStatus == 1) //That's an error, pass it along.
        return 1;

    // 3. Welcome the user!
    printf(BOLD_RED  "\n============================\n" FORMAT_OFF);
    printf(BOLD_NORMAL " Welcome to Chat & Go Fish! \n" FORMAT_OFF);
    printf(BOLD_RED    "============================\n\n" FORMAT_OFF);
    if(!noUnicode){
        printf("If you are on Windows, please run 'chcp 65001' to enable ");
        printf("Unicode characters before playing Go Fish.\n");
        printf("You should be able to see the four suits: " HEART " " DIAMOND);
        printf(" " CLUB " " SPADE "\nIf you still cannot see them, run the ");
        printf("program with '--no-unicode'\n\n");
    }
    char port_string[5];
    itoa(port, port_string, 10);
    if(server_port){
        printf("Connected to server on port " BOLD_NORMAL "%s" FORMAT_OFF "\n\n", port_string);
        printf("Send them a message! Or type 'go fish' to start a game of go fish, or type '/q' to quit.\n\n");
        if(takeInput()) //Returns 1 on /q
            return closeSocket();
    }
    else{
        printf("Server listening on port " BOLD_NORMAL "%s" FORMAT_OFF ", waiting for connection...\n\n", port_string);
        if(serverListen()) //Returns 1 on error
            return 1;
        printf("A client has connected! \nOnce they have sent you a message, send a response! ");
        printf("Or type 'go fish' to start a game of go fish, or type '/q' to quit. \nBut wait for their message first!\n\n.\n");
        // We have a client!
    }
    
    // 4. Parse Messages
    while(true){
        if(recvMessage(debug))
            return 1;
        if(strcmp(recievedMsg, "/q") == 0){
            printf("|\n| The connection has been terminated.\n*\n");
            break;
        }
        printf("|\n| %s\n|\n", recievedMsg);

        if(takeInput()) // Returns 1 on /q
            break;
    }
    // 5. Cleanup!
    return closeSocket();
}