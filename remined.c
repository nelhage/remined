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

#include "SDL.h"
#include <stdlib.h>
#include <getopt.h>

#include "remined.h"

SDL_Surface    *screen;
SDL_Surface    *background;
SDL_Surface    *blank;
SDL_Surface    *pressed;
SDL_Surface    *opened[9];

SDL_Surface    *mine, *minedeath;
SDL_Surface    *flag;
SDL_Surface    *question;
SDL_Surface    *misflagged;

SDL_Surface    *digits[10];
SDL_Surface    *digitneg;

SDL_Surface    *smile;
SDL_Surface    *smilewin;
SDL_Surface    *smilelose;
SDL_Surface    *smileoh;
SDL_Surface    *smilepressed;

int boardWidth;
int boardHeight;
int numMines;
int useMark;			//Use the '?' mark, or just flag/no-flag if false.

SDL_Rect screenRect;
SDL_Rect boardRect;
SDL_Rect smileRect;

struct boardSquare ** board;

/*The square the mouse is being clicked on*/
Sint16 currentx, currenty;

int bothClick;
int ctrlDown;
int leftDown;
int rightDown;
int lastRedraw;
enum smilePressedMode smilePressed;

int squaresOpened;
int mineCount;
int startTime;
int gameTime;

enum gameState state = game_newGame;

int cheatEnabled;
int cheatCode[] = {'x', 'y', 'z', 'z', 'y', SDLK_RETURN, SDLK_RSHIFT, 0};
int * currChar;
SDL_Rect cheatRect;
Uint32 white, black;

int main(int argc, char ** argv)
{
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "Could not initialize SDL: %s\n",SDL_GetError());
		exit(-1);
	}

	atexit(SDL_Quit);

	parseArgs(argc, argv);

#ifdef REMINED_RESOURCES
	chdir(REMINED_RESOURCES);
#endif	
	
	initBoundaryRects();
	
	screen = SDL_SetVideoMode(screenRect.w,
							  screenRect.h,
							  32, SDL_SWSURFACE|SDL_DOUBLEBUF);
	if(!screen) {
		fprintf(stderr, "Couldn't open video: %s\n", SDL_GetError());
		exit(-1);
	}

	SDL_WM_SetCaption("ReMined", "ReMined");
	
	white = SDL_MapRGB(screen->format, 255, 255, 255);
	black = SDL_MapRGB(screen->format, 0, 0, 0);

	loadImages();
	initGame();
	newGame();

	runGame();

	shutDown();

	return 0;
}

void usage()
{
	printf("Usage: %s [OPTIONS]\n", "remined");
	printf("\n");
	printf("Mandatory arguments to long options are mandatory for short options too.\n");
	printf("  -w, --width=W               Set the board width\n");
	printf("  -h, --height=H              Set the board height\n");
	printf("  -m, --mines=M               Set the number of mines on the board\n");
	printf("  -a, --mark                  Enable the use of the '?' ``mark''\n");
	printf("  -b, --beginner              Equivalent to -h9 -w9 -m10\n");
	printf("  -i, --intermediate          Equivalent to -h16 -w16 -m40\n");
	printf("  -e, --expert                Equivalent to -h16 -w31 -m99\n");
	printf("      --help                  Display this help and exit\n");
	printf("\nWith no options, assume an expert game.\n");
	printf("\nReport bugs to <hanji@users.sourceforge.net>\n");
	exit(1);
}

void parseArgs(int argc, char ** argv)
{
    static struct option long_options[] =
        {
            {"width",           1, 0, 'w'},
            {"height",          1, 0, 'h'},
            {"mines",           1, 0, 'm'},
            {"mark",            0, 0, 'a'},
            {"beginner",        0, 0, 'b'},
            {"intermediate",    0, 0, 'i'},
            {"expert",          0, 0, 'e'},
            {"help",            0, 0,  0},
            {0,0,0,0}
        };
    int arg;
    
    //Set defaults
	boardWidth = 31;
	boardHeight = 16;
	numMines = 99;
	useMark = 0;

    while((arg = getopt_long(argc, argv, "w:h:m:abie",
                             long_options, NULL)) >= 0) {
        switch(arg) {
        case 'w':
            if(!(boardWidth = atoi(optarg))) {
                printf("Unable to understand width: %s\n", optarg);
                usage();
            }
            break;
        case 'h':
            if(!(boardHeight = atoi(optarg))) {
                printf("Unable to understand height: %s\n", optarg);
                usage();
            }
            break;
        case 'm':
            if(!(numMines = atoi(optarg))) {
                printf("Unable to understand number of mines: %s\n",
                       optarg);
                usage();
            }
            break;
        case 'a':
            useMark = 1;
            break;
        case 'b':
            boardWidth = 9;
			boardHeight = 9;
			numMines = 10;
            break;
        case 'i':
            boardWidth = 16;
			boardHeight = 16;
			numMines = 40;
            break;
        case 'e':
            boardWidth = 31;
			boardHeight = 16;
			numMines = 99;
            break;
        default:
            usage();
        }
    }
    if(optind < argc)
        usage();

	if(boardWidth < MIN_WIDTH) {
		printf("Width must be at least %d\n", MIN_WIDTH);
		usage();
	} else if(boardHeight < MIN_HEIGHT) {
		printf("Height must be at least %d\n", MIN_HEIGHT);
		usage();
	} else if(numMines * 2 >= boardWidth * boardHeight) {
		printf("Mines must take up less than 50%% of the board.\n");
		usage();
	}
}

void initBoundaryRects()
{
	
	screenRect.x = 0;
	screenRect.y = 0;
	screenRect.w = SQUARE_SIZE * boardWidth + LEFT_BORDER + RIGHT_BORDER;
	screenRect.h = SQUARE_SIZE * boardHeight + BOTTOM_BORDER + HEADER_SIZE;

	boardRect.x = LEFT_BORDER;
	boardRect.y = HEADER_SIZE;
	boardRect.w = SQUARE_SIZE * boardWidth;
	boardRect.h = SQUARE_SIZE * boardHeight;

	smileRect.x = boardRect.x + (boardRect.w/2 - SMILE_SIZE/2);
	smileRect.y = TOP_BORDER + 4;
	smileRect.w = SMILE_SIZE;
	smileRect.h = SMILE_SIZE;
	
	cheatRect.x = 0;
	cheatRect.y = 0;
	cheatRect.w = 1;
	cheatRect.h = 1;
}

SDL_Surface * loadImage(char * fn)
{
	char buff[256];
	snprintf(buff, 256, "images/%s.bmp", fn);
	SDL_Surface * img = SDL_LoadBMP(buff);
	if(!img) {
		fprintf(stderr, "Unable to load image %s: %s", fn, SDL_GetError());
		exit(-1);
	}
	return img;
}

void loadImages()
{
	int i;
	char filename[256];
		
	blank = loadImage("blank");
	for(i=0;i<9;i++) {
		snprintf(filename, 256, "open%d",i);
		opened[i] = loadImage(filename);
	}

	for(i=0;i<10;i++) {
		snprintf(filename, 256, "digit%d", i);
		digits[i] = loadImage(filename);
	}

	digitneg = loadImage("digit-");

	pressed = opened[0];

	mine = loadImage("mine");
	minedeath = loadImage("minedeath");
	flag = loadImage("flag");
	question = loadImage("question");
	misflagged = loadImage("misflagged");

	smile = loadImage("smile");
	smilewin = loadImage("smilewin");
	smilelose = loadImage("smilelose");
	smileoh = loadImage("smileoh");
	smilepressed = loadImage("smilepressed");

	/*
      Load background images and composite
      them onto a single background surface
    */
	generateBackground();
}

void generateBackground()
{
	int i;
	SDL_PixelFormat *fmt;
	SDL_Rect dst;
	SDL_Surface *topleft, *topright, *bottomleft,
        *bottomright, *left, *right, *top, *bottom;

	fmt = screen->format;
	background = SDL_CreateRGBSurface(SDL_HWSURFACE,
						 screenRect.w, screenRect.h,
						 32,
						 fmt->Rmask, fmt->Gmask,
						 fmt->Bmask, fmt->Amask);
	topleft = loadImage("topleft");
	topright = loadImage("topright");
	bottomleft = loadImage("bottomleft");
	bottomright = loadImage("bottomright");
	top = loadImage("top");
	bottom = loadImage("bottom");
	left = loadImage("left");
	right = loadImage("right");

	/*Corners*/
	dst.x = dst.y = 0;
	SDL_BlitSurface(topleft, NULL, background, &dst);

	dst.x = screenRect.w - RIGHT_BORDER;
	SDL_BlitSurface(topright, NULL, background, &dst);

	dst.y = screenRect.h - BOTTOM_BORDER;
	SDL_BlitSurface(bottomright, NULL, background, &dst);

	dst.x = 0;
	SDL_BlitSurface(bottomleft, NULL, background, &dst);

	/*Sides*/
	for(i=0;i<boardHeight;i++) {
		dst.y = HEADER_SIZE + i*SQUARE_SIZE;
		dst.x = 0;
		SDL_BlitSurface(left, NULL, background, &dst);
		dst.x = screenRect.w - RIGHT_BORDER;
		SDL_BlitSurface(right, NULL, background, &dst);
	}

	/*Top-Bottom*/
	for(i=0;i<boardWidth;i++) {
		dst.x = LEFT_BORDER + i*SQUARE_SIZE;
		dst.y = 0;
		SDL_BlitSurface(top, NULL, background, &dst);
		dst.y = screenRect.h - BOTTOM_BORDER;
		SDL_BlitSurface(bottom, NULL, background, &dst);
	}

	/*Free the individual background images*/
	SDL_FreeSurface(topleft);
	SDL_FreeSurface(topright);
	SDL_FreeSurface(bottomleft);
	SDL_FreeSurface(bottomright);
	SDL_FreeSurface(top);
	SDL_FreeSurface(bottom);
	SDL_FreeSurface(left);
	SDL_FreeSurface(right);
	
}

void initGame()
{
	int i;

	board = malloc(sizeof(struct boardSquare*) * boardHeight);
	for(i=0;i<boardHeight;i++)
		board[i] = malloc(sizeof(struct boardSquare) * boardWidth);

	srand(SDL_GetTicks());
	
	cheatEnabled = 0;
	currChar = cheatCode;
}

void newGame()
{
	Sint16 row, col;
	
	for(row=0;row<boardHeight;row++)
		for(col=0;col<boardWidth;col++) {
			board[row][col].click = click_Closed;
			board[row][col].type = type_Empty;
		}

	state = game_newGame;
	leftDown = rightDown = bothClick = 0;

	mineCount = numMines;
	startTime = SDL_GetTicks();
	gameTime = 0;
	squaresOpened = 0;
	smilePressed = smile_notPressed;
	lastRedraw = 0;
}

void runGame()
{
	SDL_Event e;
	int quit=0;
	Sint16 gridx, gridy;
	
	redrawBoard();
	while(!quit) {
		if(SDL_PollEvent(&e)) {
			switch(e.type) {
			case SDL_QUIT:
				quit = 1;
				break;
			case SDL_MOUSEBUTTONDOWN:
				if(pointInRect(e.button.x, e.button.y, &smileRect)) {
					if(e.button.button == SDL_BUTTON_LEFT) {
                        lastRedraw = 0;
						smilePressed = smile_isPressed;
                    }
				}
				if(state == game_newGame
                   || (state == game_playing
                       && smilePressed == smile_notPressed)) {
					screenToGrid(e.button.x, e.button.y, &gridx, &gridy);
					mouseDown(e.button.button, gridx, gridy);
				}
				break;
			case SDL_MOUSEBUTTONUP:
				if(smilePressed != smile_notPressed) {
					if(e.button.button == SDL_BUTTON_LEFT) {
						if(pointInRect(e.button.x,
                                       e.button.y,
                                       &smileRect)) {
							newGame();
						}
						smilePressed = smile_notPressed;
					}
				}
				else if(state == game_newGame || state == game_playing) {
					screenToGrid(e.button.x, e.button.y, &gridx, &gridy);
					mouseUp(e.button.button, gridx, gridy);
				}
				break;
			case SDL_MOUSEMOTION:
                if(smilePressed != smile_notPressed) {
					if(pointInRect(e.button.x, e.button.y, &smileRect))
						smilePressed = smile_isPressed;
					else
						smilePressed = smile_wasPressed;
                    lastRedraw = 0;
				} else {
                    screenToGrid(e.motion.x, e.motion.y, &gridx, &gridy);
                    mouseMove(e.motion.state, gridx, gridy);
                }

				break;
			case SDL_KEYDOWN:
				if(!cheatEnabled && e.key.keysym.sym == *currChar) {
					currChar++;
					cheatEnabled = !*currChar;
				} else currChar = cheatCode;
				if(e.key.keysym.sym == SDLK_F2) {
					newGame();
				}
				break;
			}
		} else {
			if(SDL_GetTicks() - lastRedraw > 900)
				redrawBoard();
			else
				SDL_Delay(50);
		}
	}
}

void shutDown()
{
	int i;
	SDL_FreeSurface(blank);
	for(i=0;i<9;i++)
		SDL_FreeSurface(opened[i]);
	for(i=0;i<10;i++)
		SDL_FreeSurface(digits[i]);

	SDL_FreeSurface(digitneg);

	SDL_FreeSurface(mine);
	SDL_FreeSurface(minedeath);
	SDL_FreeSurface(flag);
	SDL_FreeSurface(question);
	SDL_FreeSurface(misflagged);

	SDL_FreeSurface(smile);
	SDL_FreeSurface(smilelose);
	SDL_FreeSurface(smilewin);
	SDL_FreeSurface(smileoh);
	SDL_FreeSurface(smilepressed);

	SDL_FreeSurface(background);

	for(i=0;i<boardHeight
			;i++)
		free(board[i]);
	free(board);
}

int pointInRect(Uint16 x, Uint16 y, SDL_Rect * rect)
{
	return (x > rect->x &&
			x < rect->x + rect->w &&
			y > rect->y &&
			y < rect->y + rect->h);
}

void screenToGrid(Sint16 screenx, Sint16 screeny,
                  Sint16 *gridx, Sint16 *gridy)
{
	if(!pointInRect(screenx, screeny, &boardRect)) {
		*gridx = -1;
		*gridy = -1;
	} else {
		*gridx = (screenx - boardRect.x) / SQUARE_SIZE;
		*gridy = (screeny - boardRect.y) / SQUARE_SIZE;
	}
}

void gridToScreen(Sint16 gridx, Sint16 gridy,
                  Sint16 *screenx, Sint16 *screeny)
{
	*screenx = gridx * SQUARE_SIZE + boardRect.x;
	*screeny = gridy * SQUARE_SIZE + boardRect.y;
}

struct boardSquare * getSquare(Sint16 gridx, Sint16 gridy)
{
	if(gridx < 0 || gridx >= boardWidth ||
	   gridy < 0 || gridy >= boardHeight) {
		return NULL;
	}

	return &(board[gridy][gridx]);
}

void mouseDown(Uint8 button, Sint16 gridx, Sint16 gridy)
{
	if(button == SDL_BUTTON_LEFT)
		leftDown = 1;
	else if(button == SDL_BUTTON_RIGHT)
		rightDown = 1;
    else if(button == SDL_BUTTON_MIDDLE)
        leftDown = rightDown = 1;
	
	currentx = gridx;
	currenty = gridy;
	
	ctrlDown = SDL_GetModState()&KMOD_CTRL;
	
	if(leftDown && (rightDown || ctrlDown)) {
		bothClick = 1;
		bothDown(gridx, gridy);
	} else if(button == SDL_BUTTON_LEFT && !bothClick) {
		pressSquare(gridx, gridy);
	} else if(button == SDL_BUTTON_RIGHT && !bothClick) {
		markSquare(gridx, gridy);
	}
}

void mouseUp(Uint8 button, Sint16 gridx, Sint16 gridy)
{
	if(button == SDL_BUTTON_LEFT)
		leftDown = 0;
	else if(button == SDL_BUTTON_RIGHT)
		rightDown = 0;
    else if(button == SDL_BUTTON_MIDDLE) {
		int mouse = SDL_GetMouseState(NULL, NULL);
		leftDown = (mouse & SDL_BUTTON_LEFT) == SDL_BUTTON_LEFT;
		rightDown = (mouse & SDL_BUTTON_RIGHT) == SDL_BUTTON_RIGHT;
	}

    //Force a redraw, because the smile transitions from the `o' face
    lastRedraw = 0;     
    
    if(gridx < 0 || gridy < 0) return;
    
	if(button == SDL_BUTTON_LEFT && !bothClick) {
		openSquare(gridx, gridy);
	}

	if(bothClick && !(leftDown && rightDown)) {
		bothUp(gridx, gridy);
		doBothClick(gridx, gridy);
	}	   

	if(!leftDown && !rightDown)
		bothClick = 0;
}

void mouseMove(Uint8 buttons, Sint16 gridx, Sint16 gridy)
{
	if(gridx == currentx && gridy == currenty)
		return;

    if(cheatEnabled)
        lastRedraw = 0;

    if((state == game_newGame || state == game_playing) &&
       ((buttons & SDL_BUTTON_LEFT) || (buttons & SDL_BUTTON_MIDDLE))) {
        if(leftDown && (rightDown || ctrlDown)) {
			bothUp(currentx, currenty);
			bothDown(gridx, gridy);
		} else if(!bothClick) {
			unpressSquare(currentx, currenty);
			pressSquare(gridx, gridy);
		}
	}
	
	currentx = gridx;
	currenty = gridy;
}

void openSquare(Sint16 gridx, Sint16 gridy)
{
	struct boardSquare * square;

	if(state == game_newGame) {
		generateBoard(gridx, gridx);
		state = game_playing;
		startTime = SDL_GetTicks();
	}

    lastRedraw = 0;
	square = getSquare(gridx, gridy);
	
	if(!square ||
	   (square->click != click_Closed && square->click != click_Pressed)) {
		return;
	}
	
	square->click = click_Opened;
	squaresOpened++;
	
	if(square->type == type_Mine) {
		state = game_lost;
	} else if(squaresOpened == (boardWidth*boardHeight) - numMines) {
		state = game_won;
	}else if(square->type == type_Empty) {
		int dx, dy;
		for(dx=-1;dx<=1;dx++)
			for(dy=-1;dy<=1;dy++)
				if(!dx && !dy) continue;
				else openSquare(gridx + dx, gridy + dy);
	}
}

void bothDown(Sint16 gridx, Sint16 gridy)
{
	int dy,dx;
	
	if(!getSquare(gridx, gridy)) return;
	
	for(dx=-1;dx<=1;dx++)
		for(dy=-1;dy<=1;dy++) 
			pressSquare(gridx + dx, gridy + dy);
}

void bothUp(Sint16 gridx, Sint16 gridy)
{
	int dy,dx;
	
	if(!getSquare(gridx, gridy)) return;
	
	for(dx=-1;dx<=1;dx++)
		for(dy=-1;dy<=1;dy++)
			unpressSquare(gridx + dx, gridy + dy);
}

void pressSquare(Sint16 gridx, Sint16 gridy)
{
	struct boardSquare * square = getSquare(gridx, gridy);
	if(square && square->click == click_Closed)
		square->click = click_Pressed;
	lastRedraw = 0;
}

void unpressSquare(Sint16 gridx, Sint16 gridy)
{
	struct boardSquare * square = getSquare(gridx, gridy);
	if(square && square->click == click_Pressed)
		square->click = click_Closed;
	lastRedraw = 0;
}

void doBothClick(Sint16 gridx, Sint16 gridy)
{
	int dx, dy;
	int flags = 0;
	struct boardSquare * square;

	square = getSquare(gridx, gridy);
	if(!square || square->click != click_Opened)
		return;

	for(dx=-1;dx<=1;dx++)
		for(dy=-1;dy<=1;dy++) 
			if(!dy && !dx) continue;
			else if((square = getSquare(gridx + dx, gridy + dy)) &&
					square->click == click_Flagged)
				flags++;
	
	if(flags == getSquare(gridx, gridy)->type) {
		for(dx=-1;dx<=1;dx++)
			for(dy=-1;dy<=1;dy++)
				openSquare(gridx + dx, gridy + dy);
	lastRedraw = 0;
	}
}

void markSquare(Sint16 gridx, Sint16 gridy)
{
	struct boardSquare * square = getSquare(gridx, gridy);
	if(!square) return;

	switch(square->click) {
	case click_Closed:
		square->click = click_Flagged;
		mineCount--;
		break;
	case click_Flagged:
		square->click = useMark ? click_Question : click_Closed;
		mineCount++;
		break;
	case click_Question:
		square->click = click_Closed;
		break;
	default:
		break;
	}
	lastRedraw = 0;
}

void generateBoard(Sint16 initx, Sint16 inity)
{
	int minesPlaced = 0;
	Sint16 minex, miney;
	Sint16 x,y;
	Sint8 dx, dy;
	Uint8 mineCount;
	struct boardSquare * square;
	
	while(minesPlaced < numMines) {
		minex = rand() % boardWidth;
		miney = rand() % boardHeight;

		if(minex != initx && miney != inity &&
		   getSquare(minex, miney)->type != type_Mine) {
			getSquare(minex, miney)->type = type_Mine;
			minesPlaced++;
		}		   
	}

	for(x=0;x<boardWidth;x++)
		for(y=0;y<boardHeight;y++)
		{
			if(getSquare(x,y)->type == type_Mine)
				continue;
			mineCount = 0;
			for(dx=-1;dx<=1;dx++)
				for(dy=-1;dy<=1;dy++)
					if((square = getSquare(x+dx,y+dy)) &&
					   square->type == type_Mine)
						mineCount++;
			getSquare(x,y)->type = mineCount;
		}
}

void redrawBoard()
{
    struct boardSquare * square;
	SDL_BlitSurface(background, NULL, screen, &screenRect);

    drawMineCount();
    drawTimer();
    drawSmile();
    drawSquares();
    
	if(cheatEnabled) {
        square = getSquare(currentx, currenty);
        if(square && square->type == type_Mine)
            SDL_FillRect(screen, &cheatRect, black);
        else
            SDL_FillRect(screen, &cheatRect, white);
	}

	SDL_Flip(screen);
	lastRedraw = SDL_GetTicks();
}

void drawMineCount()
{
    drawNum(mineCount,
            LEFT_BORDER + NUM_HSPACE,
            TOP_BORDER + NUM_VSPACE);
}

void drawTimer()
{
    if(state == game_playing && gameTime < 999) {
		/*Start counting from 1, not 0, to emulate the original's behavior*/
		gameTime = (SDL_GetTicks() - startTime) / 1000 + 1;
		if(gameTime >= 999)
			gameTime = 999;
	}
	drawNum(gameTime, (screenRect.w - RIGHT_BORDER
                       - NUM_HSPACE - 3*DIGIT_WIDTH),
            TOP_BORDER + NUM_VSPACE);
}

void drawSmile()
{
    SDL_Surface * smileimg = NULL;
    
	if(smilePressed == smile_isPressed)
		smileimg = smilepressed;
	else if(state == game_won)
		smileimg = smilewin;
	else if(state == game_lost)
		smileimg = smilelose;
	else
		smileimg = leftDown ? smileoh : smile;

	SDL_BlitSurface(smileimg, NULL, screen, &smileRect);
}

void drawSquares()
{
    int row,col;

    for(row=0;row<boardHeight;row++)
		for(col=0;col<boardWidth;col++)
            drawSquare(row, col);
}

void drawSquare(int row, int col) {
    SDL_Rect dst;
	SDL_Surface * img;
    gridToScreen(col, row, &dst.x, &dst.y);
    if(state != game_lost) {
        if(board[row][col].click == click_Flagged ||
           (board[row][col].type == type_Mine && state == game_won))
            img = flag;
        else if(board[row][col].click == click_Closed)
            img = blank;
        else if(board[row][col].click == click_Pressed)
            img = pressed;
        else if(board[row][col].click == click_Question)
            img = question;
        else
            img = opened[board[row][col].type];
    } else {
        if(board[row][col].type == type_Mine)
            img = (board[row][col].click == click_Flagged) ? flag :
                (board[row][col].click == click_Opened) ? minedeath : mine;
        else if(board[row][col].click == click_Flagged)
            img = misflagged;
        else if(board[row][col].click == click_Opened)
            img = opened[board[row][col].type];
        else
            img = blank;
    }
    SDL_BlitSurface(img, NULL, screen, &dst);
}

void drawNum(int num, Sint16 x, Sint16 y)
{
	int sign = (num < 0);
	if(sign) num = -num;
	int ones = num % 10;
	int tens = (num / 10) % 10;
	int hundreds = (num / 100) % 10;

	SDL_Rect dst;

	dst.x = x;
	dst.y = y;
	if(!sign)
		SDL_BlitSurface(digits[hundreds], NULL, screen, &dst);
	else
		SDL_BlitSurface(digitneg, NULL, screen, &dst);
	dst.x += DIGIT_WIDTH;
	SDL_BlitSurface(digits[tens], NULL, screen, &dst);
	dst.x += DIGIT_WIDTH;
	SDL_BlitSurface(digits[ones], NULL, screen, &dst);
}
