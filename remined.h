/*
 * Copyright (c) 2005, Nelson Elhage
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __MINES_H__
#define __MINES_H__

#define SQUARE_SIZE 16
#define LEFT_BORDER 12
#define TOP_BORDER LEFT_BORDER
#define BOTTOM_BORDER 8
#define RIGHT_BORDER 8
#define HEADER_SIZE 55
#define DIGIT_WIDTH  13
#define NUM_HSPACE  6
#define NUM_VSPACE  5

#define SMILE_SIZE  26

#define MIN_WIDTH 8
#define MIN_HEIGHT 5

enum squareType
	{
		type_Mine = -1,
		type_Empty = 0,
		/*Any value above this means surrounded by that many mines*/
	};

enum clickState
	{
		click_Closed = 0,
		click_Pressed,
		click_Opened,
		click_Flagged,
		click_Question,
	};

struct boardSquare
{
	enum clickState click;
	enum squareType type;
};

enum gameState
	{
		game_newGame,
		game_playing,
		game_won,
		game_lost
	};

enum smilePressedMode
	{
		smile_notPressed,
		smile_wasPressed,
		smile_isPressed
	};

void usage();
void parseArgs(int argc, char ** argv);

void initBoundaryRects();
SDL_Surface * loadImage(char * filename);
void loadImages();
void initGame();
void newGame();
void shutDown();

void runGame();

int pointInRect(Uint16 x, Uint16 y, SDL_Rect * rect);
void screenToGrid(Sint16 screenx, Sint16 screeny,
                  Sint16 *gridx, Sint16 *gridy);
void gridToScreen(Sint16 gridx, Sint16 gridy,
                  Sint16 *screenx, Sint16 *screeny);
struct boardSquare* getSquare(Sint16 gridx, Sint16 gridy);

void mouseDown(Uint8 button, Sint16 x, Sint16 y);
void mouseUp(Uint8 button, Sint16 x, Sint16 y);
void mouseMove(Uint8 state, Sint16 x, Sint16 y);

void openSquare(Sint16 x, Sint16 y);
void bothDown(Sint16 gridx, Sint16 gridy);
void bothUp(Sint16 gridx, Sint16 gridy);
void doBothClick(Sint16 gridx, Sint16 gridy);
void pressSquare(Sint16 gridx, Sint16 gridy);
void unpressSquare(Sint16 gridx, Sint16 gridy);
void markSquare(Sint16 x, Sint16 y);

void generateBoard(Sint16 initx, Sint16 inity);

void redrawBoard();
void drawMineCount();
void drawTimer();
void drawSmile();
void drawSquares();
void drawSquare(int row, int col);
void drawNum(int num, Sint16 x, Sint16 y);

#endif
