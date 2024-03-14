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
 *  Returns 0 on successful input taking
 *  Returns 1 if input is /q (user wants to quit the program)
*/
void takeInput(){
    printf(":>> ");
    fflush(stdout);
    memset(buf, 0, 4096);
    scanf("%4095[^\n]", buf);
    getchar();
    fflush(stdout);
}

void printStatus(GoFish* game){
    printf("|\n| Your hand: ");
    for(int i = 0; i < game->hand_size; i++){
        printCard(game->hand[i], noUnicode);
        printf(" ");
    }
    printf("\n| Your books: %d", game->books);
    printf("\n| Opponent's books: %d\n", game->opponent_books);
}

int handleMine(GoFish* game){
    while(processAsk(game, buf)){
        printf("|\n| You got what you asked for!\n|\n");
        printStatus(game);
        printf("|\n| What would you like to ask for?\n| Enter the face value, like 'K' or '9'\n|\n");
        takeInput();
        while(!verifyFaceSelection(game, buf)){
            printf("|\n| That's not a valid face value. Enter one of your face vales.\n|\n");
            takeInput();
        }
        sendMessage(buf, 1, debug);
        if(recvMessage(debug))
            return 1;
    }
    printf("|\n| You went fishing and got a ");
    printCard(game->hand[game->hand_size-1], noUnicode);
    printf("!\n|\n");
    sendMessage("DONE", 1, debug);
}

/**
 *      Takes control of the conversation loop
 *  isFirst: true if this process sent the first "go fish"
*/
int handleGame(bool isFirst){
    GoFish* game = initGame();
    if(isFirst){
        //You sent `go fish`, they send pond:
        if(recvMessage(debug))
            return 1;
        game->pond = (card*)recievedMsg;
        dealCards(isFirst, game);
        printStatus(game);
        printf("|\n| What would you like to ask for?\n| Enter the face value, like 'K' or '9'\n|\n");
        takeInput();
        while(!verifyFaceSelection(game, buf)){
            printf("|\n| That's not a valid face value. Enter one of your face vales.\n|\n");
            takeInput();
        }
        sendMessage(buf, 1, debug);
    }
    else{
        //They sent go fish, you send pond, they send first move:
        if(debug)
            printf("Sending initial pond\n");
        sendMessage((char*)game->pond, 104, debug);
        dealCards(isFirst, game);
        printf("|\n| A game of Go Fish has begun! Awaiting your opponent's first move.\n|\n");
    }

    while(game->pond_size > 0 || game->hand_size > 0 || game->opponent_size > 0){
        if(recvMessage(debug))
            return 1;
        //Message
        while(strcmp(recievedMsg, "DONE") != 0){
            printStatus(game);
            printf("|\n| Opponent asked for %ss. Do you have any? Type 'yes' or 'go fish'.\n|\n", recievedMsg);
            takeInput();
            while(strcmp(buf, "yes") != 0 && strcmp(buf, "go fish") != 0){
                printf("|\n| That's not 'yes' or 'go fish'.\n|\n");
                takeInput();
            }
            while(isLying(game, recievedMsg, strcmp(buf, "yes") != 0)){
                printf("|\n| You're lying. Is it 'yes' or 'go fish'?\n|\n");
                takeInput();
            }
            sendMessage("OK", 2, debug);
            //Process what just happened !!!!!!
        }

        printStatus(game);
        printf("|\n| What would you like to ask for?\n| Enter the face value, like 'K' or '9'\n|\n");
        takeInput();
        while(!verifyFaceSelection(game, buf)){
            printf("|\n| That's not a valid face value. Enter one of your face vales.\n|\n");
            takeInput();
        }
        sendMessage(buf, 1, debug);
        if(recvMessage(debug))
            return 1;
        handleMine(game);
    }

    //Set recievedMsg for the person that did not start the game. 
    //They will be the next to send a message.
    //That who started the game will next wipe recievedMsg and start listening.
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
        takeInput();
        sendMessage(buf, strlen(buf), debug);
        if(strcmp(buf, "/q") == 0){
            printf("|\n| The connection has been terminated.\n*\n");
            return closeSocket();
        }
        else if(strcmp(buf, "go fish") == 0)
            handleGame(true);
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
        else if(strcmp(recievedMsg, "go fish") == 0)
            handleGame(false);
        printf("|\n| %s\n|\n", recievedMsg);

        takeInput();
        sendMessage(buf, strlen(buf), debug);
        if(strcmp(buf, "/q") == 0){
            printf("|\n| The connection has been terminated.\n*\n");
            break;
        }
        else if(strcmp(buf, "go fish") == 0)
            handleGame(true);
    }
    // 5. Cleanup!
    return closeSocket();
}