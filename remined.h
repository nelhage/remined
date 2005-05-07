#ifndef __MINES_H__
#define __MINES_H__

#define SQUARE_SIZE 16
#define LEFT_BORDER 12
#define BOTTOM_BORDER 8
#define RIGHT_BORDER 8
#define HEADER_SIZE 55
#define DIGIT_WIDTH  13
#define NUM_HSPACE  6
#define NUM_VSPACE  5

#define SMILE_SIZE  26

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

SDL_Surface * loadImage(char * filename);
void loadImages();
void initGame();
void newGame();
void shutDown();

void runGame();

void screenToGrid(Sint16 screenx, Sint16 screeny, Sint16 *gridx, Sint16 *gridy);
void gridToScreen(Sint16 gridx, Sint16 gridy, Sint16 *screenx, Sint16 *screeny);
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
void drawNum(int num, Sint16 x, Sint16 y);

#endif
