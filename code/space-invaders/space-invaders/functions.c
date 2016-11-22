//
//  functions.c
//  space-invaders
//
//  Created by Illya Starikov on 11/10/16.
//  Copyright © 2016 Illya Starikov. All rights reserved.
//

#include "functions.h"
#include "constants.h"
#include "game.h"
#include "point.h"

#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>

void initGame(Game* game) {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    
    game -> height = w.ws_row;
    game -> width = w.ws_col;
    
    game -> level = 2; // THIS IS ONLY A TEST
    game -> lives = 3;
    game -> score = 0;
}

void createHeader(const Game *game, char** headerLine) {
    unsigned char workingSize = game -> width; // working size is to keep track how much room is left for the header line
    char tempString[2];
    
    *headerLine = malloc(game -> width * sizeof(char));
    
    if (workingSize > strlen(NAME)) {
        strcpy(*headerLine, NAME);
        workingSize -= strlen(NAME);
    }
    
    
    if (workingSize > strlen(LIVES) + sizeof(tempString)) {
        strcat(*headerLine, LIVES);
        strcat(*headerLine, numberToString(game -> lives, tempString));
        workingSize -= strlen(NAME);
    }
    
    if (workingSize > strlen(SCORE) + sizeof(tempString)) {
        strcat(*headerLine, SCORE);
        strcat(*headerLine, numberToString(game -> score, tempString));
        workingSize -= strlen(NAME);
    }
}

// Note: the center actually ignores height. ¯\_(ツ)_/¯
void drawShooter(const CartesianPoint center, const Game *game, char*** shooterAscii) {
    const unsigned char width = sizeof(*gunner);
    const unsigned char height = sizeof(gunner) / width; //works like stringHeight, but can't use it because incompatible pointers
    
    int i, j, shooterCounter;

    *shooterAscii = malloc(height * sizeof(char*));
    for (i = 0; i < height; i++) {
        (*shooterAscii)[i] = malloc(game -> width * sizeof(char));
    }
    
    for (j = 0; j < height; j++) {
        shooterCounter = 0;
        for (i = 0; i < game -> width - 1; i++) {
            
            // the if determines if the shooter should be drawn, and it works like so:
            // if it's in the range of 1/2width on either side of the center, then it's a match
            // -1 because used sizeof for width calculation and that includes terminating character
            if (i + strlen(*gunner)/2.0 >= center.x && i - strlen(*gunner)/2.0 <= center.x ) {
                (*shooterAscii)[j][i] = gunner[j][shooterCounter];
                shooterCounter++;
            } else {
                (*shooterAscii)[j][i] = ' ';
            }
        }
        (*shooterAscii)[j][i] = '\0';
    }
}

void drawGame(Game *game, char*** aliensAndShields) {
    unsigned char numberOfAliens;
    const unsigned char shelterHeight = sizeof(shelter)/sizeof(*shelter);
    const unsigned char height = game -> height - 1 - sizeof(gunner)/sizeof(*gunner); // read string height to figure out how this works
    // we subtract 1 for the header string and the gunner height becuase this is this the "working canvas"
    static const unsigned char heightOfAverageAlien = sizeof(smallInvaderOne)/sizeof(smallInvaderOne[0]); // static because we assume height to never change
    unsigned char i, j;
    
    if (game -> currentNumberOfAliens == 0) {
        game -> currentNumberOfAliens = game -> level;
        numberOfAliens = game -> level;
    } else {
        numberOfAliens = game -> currentNumberOfAliens;
    }
    
    *aliensAndShields = malloc(height * sizeof(char*));
    for (i = 0; i < height; i++) {
        (*aliensAndShields)[i] = malloc(game -> width * sizeof(char));
    }

    // Draw Sheilds First
    // i = row, j = column
    
    // We start at the lowermost part of the gameboard to start drawing the shields (i.e. height -1)
    // And continue until we get to the top of where the shelter be, i.e. the previous height - shetler height
    for (i = height - 1; i > height - shelterHeight - 1; i--) {
        // We subtracted game -> width % shelter because we don't want to draw half a shelter
        for (j = 0; j < game -> width - (game -> width % strlen(*shelter)); j++) {
            // The second accessor (i.e. shelter[i][x]) just mods the shelter length so we can repitively
            // draw shelters
            (*aliensAndShields)[i][j] = shelter[(i - height + shelterHeight)][j % strlen(*shelter)];
        }
        (*aliensAndShields)[i][j] = '\0';
    }
    
    // Now the aliens
    // We start at the height and draw the same row of aliens as there are rows
    // However, the aliens are typically 2 rows, so have to multiply that by the alien height
    // We assume all alien heights to be the same
    for (i = 0; i < game -> level * heightOfAverageAlien; i++) {
        for (j = 0; j < game -> width; j++) {
            (*aliensAndShields)[i][j] = smallInvaderOne[i % heightOfAverageAlien][j % strlen(*smallInvaderOne)];
        }
        (*aliensAndShields)[i][j] = '\0';
    }
    
    
    // This is just to draw fill in the rest of the pace
    for (i = game -> level * heightOfAverageAlien; i < height - shelterHeight - 1; i++) {
        for (j = 0; j < game -> width; j++) {
            (*aliensAndShields)[i][j] = ' ';
        }
        (*aliensAndShields)[i][j] = '\0';
    }
}

void dealloc(const Game* game, char* header, char** gameboard, char** footer) {
    unsigned char i;
    
    free(header);
    
    for (i = 0; i < sizeof(gunner) / sizeof(*gunner); i++) {
        free(footer[i]);
    }
    free(footer);
    
    for (i = 0; i < game -> height - 1 - sizeof(gunner)/sizeof(*gunner); i++) {
        free(gameboard[i]);
    }
    
    free(gameboard);
}

int power(int base, int exponent) {
    if (exponent == 0) {
        return 1;
    }
    
    return base * power(base, exponent - 1);
}

char* numberToString(int number, char *string) {
    if (string == NULL) {
        return NULL;
    }
    
    sprintf(string, "%d", number);
    return string;
}


unsigned char stringHeight(char** string) {
    // so sizeof returns effectively the area
    // and sizeof returns the width
    // area/width = height. Woo!
    if (string == NULL) {
        return 0; // To prevent division by zero
    }
    return sizeof(string)/sizeof(string[0]);
}
