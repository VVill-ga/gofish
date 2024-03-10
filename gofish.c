/**
 *   GO-FISH ONLINE!
 *  By: Will Garrison
 * 
 *  Game Logic
 * 
 *  Sources of Code or Inspiration: 
 *      - "Go Fish Online" by cardgames.io, for complete ruleset
 *         https://cardgames.io/gofish
 * 
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>   //For seeding the random shuffle

#if defined(_WIN32) || defined(__MSDOS__)
    // Unicode + Windows Terminal == difficult :(
    #define SPADE   "\x06"
    #define CLUB    "\x05"
    #define HEART   "\x03"
    #define DIAMOND "\x04"
    #define BOLD_RED    "\033[;1;31m"
    #define BOLD_NORMAL "\033[;1m"
    #define FORMAT_OFF  "\033[m"
#else
    // Unicode + Linux Terminal == Possible!
    #define SPADE   "\xE2\x99\xA0"
    #define CLUB    "\xE2\x99\xA3"
    #define HEART   "\xE2\x99\xA5"
    #define DIAMOND "\xE2\x99\xA6"
    #define BOLD_RED    "\e[;1;31m"
    #define BOLD_NORMAL "\e[;1m"
    #define FORMAT_OFF  "\e[m"
#endif


// Spades, Clubs, Hearts, and Diamonds
const char SUITS[] = {'S', 'C', 'H', 'D'};
const int NUM_SUITS = 4;
// 1-9, Jack, Queen, King, Ace
const char FACES[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 'J', 'Q', 'K', 'A'};
const int NUM_FACES = 13;

typedef struct {
    char suit;
    char face;
} card;

/**
 *  Returns: A pointer to the first card in a complete deck of playing cards
 * 
 *  S1, S2, S3, ...
*/
card* generateDeck(){
    card* deck = malloc(NUM_SUITS * NUM_FACES * sizeof(card));
    for(int i = 0; i < NUM_SUITS; i++){
        for(int j = 0; j < NUM_FACES; j++){
            deck[(i * NUM_FACES) + j].suit = SUITS[i];
            deck[(i * NUM_FACES) + j].face = FACES[j];
        }
    }
    return deck;
}

/**
 *  Swaps each card with a random card in the deck. Not the greatest
 *  shuffle function you've ever seen, but pretty straightforward.
 * 
 *  Deck points to an array of cards, and size is the number of cards
 *  in the deck (ex. 52 for a full deck of playing cards)
*/
void shuffleDeck(card* deck, int size){
    // Used to swap two cards
    card inHand = {};
    srand(time(NULL));
    for(int i = 0; i < size; i++){
        // Save this card so we can put a random card here.
        inHand.suit = deck[i].suit;
        inHand.face = deck[i].face;

        int j = rand() % size;
        deck[i].suit = deck[j].suit;
        deck[i].face = deck[j].face;
        deck[j].suit = inHand.suit;
        deck[j].face = inHand.face;
    }
}

void printCard(card c){
    if(c.suit == 'H' || c.suit == 'D')
        printf(BOLD_RED);
    else
        printf(BOLD_NORMAL);
    switch(c.suit){
        case 'S':
            //printf("%c" SPADE, c.face);
            printf(SPADE "%c", c.face);
            break;
        case 'C':
            printf(CLUB "%c", c.face);
            break;
        case 'H':
            printf(HEART "%c", c.face);
            break;
        case 'D':
            printf(DIAMOND "%c", c.face);
            break;
    }
    printf(FORMAT_OFF);
}

int testDeck(){
    card* deck = generateDeck();
    shuffleDeck(deck, 52);

    for(int i = 0; i < 52; i++){
        printCard(deck[i]);
        if((i+1) % 13 == 0)
            printf("\n");
        else
            printf(", ");
    }
    printf("\n");

    return 0;
}