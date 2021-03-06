
#include <assert.h>
#include <nusys.h> 
/*includes */
#include "graphic.h"
#include "main.h"
#include "globals.h"
#include "stage01.h"
//atan function
#include "math.c"
/* my models */
#include "pebble_black.h"
#include "pebble_white.h"
#include "cross_by_three.h"
// #include "boardcorners.h"
// #include "boardside.h"
#include "boardsquarealt.h"
#include "boardedge.h"
#include "selectionRing.h"
// #include "boardsquare.h"
#include "table.h"
#include "floor.h"
#include "decal.h"

// borrowed bits
#include "font.h"

#define MAX_DROPS 100
#define MAX_PLAYERS 2
#define MAX_MENU_ITEMS 2

extern NUContData contdata[];
extern u32 nuScRetraceCounter;

Lights1 sun_light = gdSPDefLights1(	200,200,200,           /* ambient color red = purple * yellow */
        														255,0,0,
																		-80,-80,0); 

Lights0 my_ambient_only_light=gdSPDefLights0(200,200,200);

/* camera values ----------------------*/
Vec3d cameraPos = {0.0f, 0.0f, 0.0f};   /*|*/
Vec3d cameraTarget = {0.0f, 0.0f, 0.0f};/*|*/
Vec3d cameraUp = {0.0f, 1.0f, 0.0f};    /*|*/
                                        /*|*/
float cameraDistance;                   /*|*/
Vec3d cameraRotation;                   /*|*/
/*-------------------------------------*/

/* ---board structure--- */
typedef struct Pebble{
  Vec3d     position;
  float     rotation;
  int 			colour;
} Pebble;

typedef struct boardSquare{
  int isEmpty;
  Pebble *content; /* set to pointer to pebble in that square*/
} boardSquare;

Pebble pebbles[MAX_DROPS];
int pebbleCount;
int squareCount;
boardSquare board[9][9];
float squareSize;
float boardSize;
/*----------------------*/

/* game values --------------*/
int turnCount;
int2d cursorPos[MAX_PLAYERS];
float xLocation[MAX_PLAYERS];
float zLocation[MAX_PLAYERS];
float xGoal[MAX_PLAYERS];
float zGoal[MAX_PLAYERS];
int blackCaptureScore;
int whiteCaptureScore;
int blackTerritoryScore;
int whiteTerritoryScore;
int bScoreLoc;
int wScoreLoc;
const Vec2d whitePotLocation = {425.0f,0.0f};
const Vec2d blackPotLocation = {-425.0f,0.0f};
int maxTurns;
int gameOver; // BOOL
int gamePause; // BOOL
int lastTurnWasSkipped; // BOOL
int thisTurnWasSkipped; // BOOL
char menuArrow[MAX_MENU_ITEMS];
unsigned int menuSelection; // unsigned because the menu item array is only positive numbers
/*---------------------------*/

/* capture variables --------*/
int currentCaptureGroupColour;
int currentCaptureGroupReachedEmpty;
// if the array size is changed, change the for loop limit in the resetCaptureGroup function too
Vec2d pebblesInCurrentCaptureGroup[100];
int currentCaptureGroupIndex;
/*---------------------------*/

/* territory variables --------*/
int currentTerritoryGroupColour;
int currentTerritoryGroupReachedPebble;
int currentTerritoryGroupReachedWrongPebble;
// if the array size is changed, change the for loop limit in the resetCaptureGroup function too
Vec2d pebblesInCurrentTerritoryGroup[100];
int currentTerritoryGroupIndex;
Vec2d spacesAlreadyCheckedForTerritory[100];
int spacesCheckedForTerritoryIndex;
int sizeOfTerritory;
/*---------------------------*/

/*----- functions -----------*/
int CURRENT_GFX;
// loop values, unfortunately we have a 3 layer deep for loop :(
int i;
int o;
int k;
int timer;
int padNo;
int btn_down;
int padNoThatPaused;
int bannerHeight = (int)(SCREEN_HT/12)-1;
u32     screenWhite = GPACK_RGBA5551(255, 255, 255, 1);
u32     screenBlack = GPACK_RGBA5551(0, 0, 0, 1);
u32     screenRed =   GPACK_RGBA5551(255,0,0, 1);

//controller variables
float stickMoveX[MAX_PLAYERS];
float stickMoveY[MAX_PLAYERS];
float moveSpeedSteps[MAX_PLAYERS];
int returnedToCenter[MAX_PLAYERS];
int stickResetFromTimer[MAX_PLAYERS];
/*-----------------------*/

/*--- Debug ----*/
int debugMode;
float debugY;
float debugX;
/*--------------*/

// the 'setup' function
void initStage01() {  
  // the advantage of initializing these values here, rather than statically, is
  // that if you switch stages/levels, and later return to this stage, you can
  // call this function to reset these values.
  
  for (i=0; i<MAX_DROPS;i++){
    pebbles[i].position.x = NULL;
    pebbles[i].position.y = NULL;
    pebbles[i].position.z = NULL;
    pebbles[i].rotation = 0.0f;
    pebbles[i].colour = NULL;
  }

  pebbleCount = 0;

  /*initialise board size and scale*/
  squareCount = 9;
  for (i=0;i<squareCount;i++){
    for (o=0;o<squareCount;o++){
      board[i][o].isEmpty = TRUE;
    }
  }
  squareSize = 75.0f; //50.0
  boardSize = squareSize*(squareCount-1);

  for(padNo=0;padNo<MAX_PLAYERS; padNo++){
    //moving values
    cursorPos[padNo].x = padNo*(squareCount-1);
    cursorPos[padNo].y = (int)squareCount/2;

    stickMoveX[padNo] = 0;
    stickMoveY[padNo] = 0;
    xLocation[padNo] = cursorPos[padNo].x*squareSize - (boardSize/2); 
    zLocation[padNo] = cursorPos[padNo].y*squareSize - (boardSize/2); 
    xGoal[padNo] = xLocation[padNo];
    zGoal[padNo] = zLocation[padNo];
    returnedToCenter[padNo] = TRUE;
    moveSpeedSteps[padNo] = 1;
    stickResetFromTimer[padNo] = 0;
    moveCursor(padNo,0,0);
  }

  menuArrow[0] = '-';
  for(i=1;i<MAX_MENU_ITEMS;i++){
    menuArrow[i]= ' ';
  }
  menuSelection = 0;

  setDefaultCamera();

  timer = 0;
	turnCount = 0;
  blackCaptureScore = 0;
  whiteCaptureScore = 0;
  blackTerritoryScore = 0;
  whiteTerritoryScore = 0;
  maxTurns = 200;
  gameOver = FALSE;  
  gamePause = FALSE;
  btn_down = FALSE;

  thisTurnWasSkipped = FALSE;
  lastTurnWasSkipped = FALSE;

  // debug
  debugMode = FALSE;
  debugX = 0.0f;
  debugY = 0.0f;
  //functions
  resetCaptureGroup();

  nuAuSeqPlayerStop(0);
  nuAuSeqPlayerSetNo(0,0);
  nuAuSeqPlayerPlay(0);
  nuAuSeqPlayerSetVol(0,0x3fff);
}

// the 'update' function
void updateGame01() {
  if(gamePause == FALSE){
    timer++;
  }
  // nuContDataGetExAll(contdata);
  for(padNo=0;padNo<MAX_PLAYERS;padNo++){
    readController(padNo);
    if(gamePause == FALSE){
      movePebble(padNo);
    }
  }
  moveCamera();
  
  

  //moving the pebbles
  // for(i=0; i<pebbleCount;i++){
  //   pebbles[i].position.x = pebbles[i].position.x + pebbles[i].velocity*cos(pebbles[i].rotation);
  //   pebbles[i].position.z = pebbles[i].position.z + pebbles[i].velocity*sin(pebbles[i].rotation);
  // }
 
  


  /* check contact with n64 */
  // for( i =0; i< pebbleCount;i++){
  //   if ( pebbles[i].position.x < 550 + range && pebbles[i].position.x > 550 - range){
  //     if( pebbles[i].position.z < 550 + range && pebbles[i].position.z > 550 - range){
  //       if ( isHit == FALSE ) {
  //         nuAuSndPlayerPlay(5);
  //       }
  //       isHit = TRUE;
  //     }
  //   }
  // }
  
}
void takeTurn(int padNo){
    // osSyncPrintf("Turn taken from player %d", padNo);
    placeCounter(padNo);
    checkForCaptures(padNo);
    calculateTerritories();
    checkGameOver();
}
void readController(int padNo){
  // read controller input from controllers
  // nuContDataGetEx(contdata, padNo);
  // We check if the 'A' Button was pressed using a bitwise AND with
  // contdata[0].trigger and the A_BUTTON constant.
  // The contdata[0].trigger property is set only for the frame that the button is
  // initially pressed. The contdata[0].button property is similar, but stays on
  // for the duration of the button press.
  if( gameOver == FALSE){
    if (contdata[padNo].trigger & START_BUTTON){
      //start button pressed and not game over'd
      if(gamePause == FALSE){
        gamePause = TRUE;
        padNoThatPaused = padNo;
        setDefaultCamera();
        nuAuSeqPlayerSetVol(0,0x1fff);
      }else if(padNoThatPaused == padNo){
        gamePause = FALSE;
        nuAuSeqPlayerSetVol(0,0x3fff);
      }
    } else {
      if( gamePause == FALSE){
        //game is not paused, and not game over'd
        if( turnCount % 2 == padNo){
          // stops players placing a piece out of turn
          if (contdata[padNo].trigger & A_BUTTON){
            takeTurn(padNo);
          }else if( contdata[padNo].trigger & B_BUTTON){
            passTurn();
          }

        }
      }else{
        // game is paused, and not game over
        if(padNo == padNoThatPaused){
          if (contdata[padNo].trigger & U_JPAD){
            menuSelection--;
            menuSelection = menuSelection%MAX_MENU_ITEMS;
            updateMenuArrow();
          }else if (contdata[padNo].trigger & D_JPAD){
            menuSelection++;
            menuSelection = menuSelection%MAX_MENU_ITEMS;
            updateMenuArrow();
          }
          if(contdata[padNo].trigger & A_BUTTON){
            btn_down = TRUE;
          }
          //selects option when button is released
          if(!(contdata[padNo].button & A_BUTTON) && btn_down == TRUE){
            //start button pressed then return to title screen
            switch (menuSelection){
              case 0 : // continue
                //this is the same as pressing start
                gamePause = FALSE;
                btn_down = FALSE;
                nuAuSeqPlayerSetVol(0,0x3fff);
                break;
              case 1 : // quit
                quit();
                btn_down= FALSE;
                break;
            }
          }
        }
      }
    }
    //c buttons to move the camera round please
    if (contdata[padNo].button & U_CBUTTONS)
      cameraRotation.y -= 0.05;
      if(cameraRotation.y <= -(M_PI/2)+0.01f){
        cameraRotation.y = -(M_PI/2)+0.01f;
      }
    if (contdata[padNo].button & D_CBUTTONS)
      cameraRotation.y += 0.05;
      if(cameraRotation.y >= (M_PI/2)-0.01f){
        cameraRotation.y = (M_PI/2)-0.01f;
      }
    if (contdata[padNo].button & L_CBUTTONS)
      cameraRotation.x -= 0.05;
    if (contdata[padNo].button & R_CBUTTONS)
      cameraRotation.x += 0.05;
    
    if(contdata[padNo].button & Z_TRIG ){
      cameraDistance = cameraDistance+2.0f;
    }
    if(contdata[padNo].button & R_TRIG ){
      cameraDistance = cameraDistance-2.0f;
    }
  }else{
    //game is over
    if(contdata[padNo].trigger & START_BUTTON){
      //start button pressed then return to title screen
      quit();
    }
  }
  // debug
  if(contdata[padNo].trigger & L_TRIG){
    debugMode ^= 1;
    nuDebConClear(NU_DEB_CON_WINDOW0);
  }
  if(debugMode == TRUE){
    if(contdata[padNo].trigger & U_JPAD){
      debugY += 1.0f;
    }else if( contdata[padNo].trigger & D_JPAD){
      debugY -= 1.0f;
    }
    if(contdata[padNo].trigger & R_JPAD){
      debugX += 1.0f;
    }else if( contdata[padNo].trigger & L_JPAD){
      debugX -= 1.0f;
    }
  }
}
void quit(){
  nuGfxFuncRemove();
  nuAuSeqPlayerStop(0);
  stage = 0;
}
void moveCamera(){
	//moving the camera
  cameraTarget.x = 0.0f;
  cameraTarget.y = 0;
  cameraTarget.z = -111.1f*cameraRotation.y + 173.0f;

  cameraPos.x = cameraTarget.x + ( cameraDistance * sin(cameraRotation.x) * cos(cameraRotation.y) );
  cameraPos.y = cameraTarget.y + ( cameraDistance * sin(cameraRotation.y)                         );
  cameraPos.z = cameraTarget.z + ( cameraDistance * cos(cameraRotation.x) * cos(cameraRotation.y) );
}
void setDefaultCamera(){
  cameraPos.x = 0.0f;
  cameraPos.y = 0.0f;
  cameraPos.z = 0.0f;
  cameraDistance = 7.0f*(squareCount*squareCount) + 300.0f;
  cameraRotation.x = 0.0f;
  cameraRotation.y = 1.0f;
  cameraRotation.z = 0.0f;
}
void setTopDownCamera(){
  cameraPos.x = 0.0f;
  cameraPos.y = 0.0f;
  cameraPos.z = 0.0f;
  cameraDistance = 7.0f*(squareCount*squareCount) + 200.0f;
  cameraRotation.x = 0.0f;
  cameraRotation.y = M_PI/2;
  cameraRotation.z = 0.0f;
}
void updateMenuArrow(){
  for(i=0;i<MAX_MENU_ITEMS;i++){
    if(menuSelection == i){
      menuArrow[i] = '-';
    }else{
      menuArrow[i] = ' ';
    }
  }
}
void movePebble(int padNo){
	//setting values for the controller sticks, divided by 80 to reduce the range to be from 0 to 1
  stickMoveX[padNo] = contdata[padNo].stick_x/80.0f;
  stickMoveY[padNo] = contdata[padNo].stick_y/80.0f;


  //moving the object
  if((stickMoveX[padNo] != 0 || stickMoveY[padNo] != 0 )&& returnedToCenter[padNo] == TRUE){
    returnedToCenter[padNo] = FALSE;
    if ( abs(stickMoveX[padNo]*100) > abs(stickMoveY[padNo]*100) ){
      //moving left right
      if ( stickMoveX[padNo] < 0){
				moveCursor(padNo,-1,0);
      } else {
				moveCursor(padNo,1,0);
      }
    } else {
      //moving up down
      if ( stickMoveY[padNo] < 0){
				moveCursor(padNo,0,1);
      } else {
				moveCursor(padNo,0,-1);
      }
    }
  }
  if(
    returnedToCenter[padNo] == FALSE && 
    (stickMoveX[padNo] == 0 && stickMoveY[padNo] == 0 || (timer - stickResetFromTimer[padNo]) % 15 == 0)
  ){
    returnedToCenter[padNo] = TRUE;
  }
  //moving the cursor
  xLocation[padNo] += (xGoal[padNo]-xLocation[padNo])/moveSpeedSteps[padNo];
  zLocation[padNo] += (zGoal[padNo]-zLocation[padNo])/moveSpeedSteps[padNo];
  // adds a ease-out effect to the movement of the spinning pebble above the cursor
  // movespeed steps reaches 1 at which point the calculation above will divide by 1, and reach the x and z goal values
  if(moveSpeedSteps[padNo]<1.1){
    moveSpeedSteps[padNo]=1;
  }else{
    moveSpeedSteps[padNo] *= 0.95;
  }
}
void moveCursor(int padNo, int xDelta, int yDelta){
		//setting position of cursor & setting when the cursor can move again

		//stopping border breaks
		if(	0 <= (cursorPos[padNo].x + xDelta) && (cursorPos[padNo].x + xDelta) < squareCount &&
				0 <= (cursorPos[padNo].y + yDelta) && (cursorPos[padNo].y + yDelta) < squareCount){
			
			// if( isEmpty(cursorPos[padNo].x+xDelta,cursorPos[padNo].y+yDelta) == TRUE){
				cursorPos[padNo].x += xDelta;
				cursorPos[padNo].y += yDelta;
			// }		
		}
		
    xGoal[padNo] = cursorPos[padNo].x*squareSize - (boardSize/2);
    zGoal[padNo] = cursorPos[padNo].y*squareSize - (boardSize/2); 
    // lower this is the faster the pebble cursor will move
    moveSpeedSteps[padNo] = 5;
    stickResetFromTimer[padNo] = timer - 1; // -1 to stop it resetting the count immediately
  }
void removePebble(int x, int y){
  if(board[x][y].isEmpty == FALSE){
    board[x][y].isEmpty = TRUE;
    movePebbleToDiscardPot(board[x][y].content);
    if((*board[x][y].content).colour == WHITE){
      blackCaptureScore++;
    }else{
      whiteCaptureScore++;
    }
    board[x][y].content = NULL;
  }
}
void movePebbleToDiscardPot(Pebble *pebble){
  if((*pebble).colour == WHITE){
    // move captured white pebbles to the pile
    (*pebble).position.x = whitePotLocation.x;
    // makes the pebbles rise
    (*pebble).position.y = blackCaptureScore * 10;
    (*pebble).position.z = whitePotLocation.y;
  }else{
    // move captured black pebbles to the pile
    (*pebble).position.x = blackPotLocation.x;
    (*pebble).position.y = whiteCaptureScore * 10;
    (*pebble).position.z = blackPotLocation.y;
  }
}
void undoPebblePlace(padNo){
  board[cursorPos[padNo].x][cursorPos[padNo].y].content = NULL;
  board[cursorPos[padNo].x][cursorPos[padNo].y].isEmpty = TRUE;
  pebbleCount--;
  turnCount--;
  thisTurnWasSkipped = FALSE;
}
void passTurn(){
  // cant skip on first two turns
  if(turnCount > 1){
    lastTurnWasSkipped = thisTurnWasSkipped;
    thisTurnWasSkipped = TRUE;
    turnCount++;
    checkGameOver();
  }
}
void placeCounter(int padNo) {
	if(board[cursorPos[padNo].x][cursorPos[padNo].y].isEmpty == TRUE){
		if( pebbleCount <= MAX_DROPS ){
			for (i = 0; i < MAX_DROPS; i++) {
				//check which number is next to store
				if( i == pebbleCount ){
					pebbleCount++;
					if( pebbleCount >= MAX_DROPS ){
						nuAuSndPlayerPlay(1);
					}else{
						nuAuSndPlayerPlay(4);
					}
					pebbleCount = pebbleCount % MAX_DROPS;
					// pebbles[i].x = ((int)(xLocation[padNo]/150))*150;
					pebbles[i].position.x = xGoal[padNo];
					pebbles[i].position.y = 10.0f;
					// pebbles[i].z = ((int)(zLocation/150))*150;
					pebbles[i].position.z = zGoal[padNo];
					pebbles[i].rotation = 0.0f;
					if(turnCount%2==0){
						pebbles[i].colour = BLACK;
					}else{
						pebbles[i].colour = WHITE;
					}
          board[cursorPos[padNo].x][cursorPos[padNo].y].content = &pebbles[i];
          board[cursorPos[padNo].x][cursorPos[padNo].y].isEmpty = FALSE;
          turnCount++;
          lastTurnWasSkipped = FALSE;
          thisTurnWasSkipped = FALSE;
					break;
				}
			}
		}
	}else{
    // cant place piece because board place is occupied
    // BONG noise
		nuAuSndPlayerPlay((int)debugX);
	}
}
int isOffBoard(int x, int y){
  if( 0 <= x && x < squareCount && 0 <= y && y < squareCount ){
    return FALSE;
  }else{
    return TRUE;
  }
}
void resetCaptureGroup(){
  currentCaptureGroupIndex = 0;
  // for (i=0;i<100;i++){
  //   pebblesInCurrentCaptureGroup[i] = NULL;
  // }
}

// returning 0 means self capture avoided
// returning 1 means capture occured
// returning 2 means nothing happened
int checkForCaptures(int padNo){
  for(i=0;i<squareCount; ++i){   
    for(o=0;o<squareCount; ++o){
      if(board[i][o].isEmpty == FALSE){
        // osSyncPrintf("\n-----A capture chain has begun-----\n");
        resetCaptureGroup();
        // set values for current check
        currentCaptureGroupReachedEmpty = FALSE;
        currentCaptureGroupColour = (*board[i][o].content).colour;
        // osSyncPrintf("Starting from %d,%d, colour is: %d\n",i,o,currentCaptureGroupColour);
        // add current to list of pebbles checked
        pebblesInCurrentCaptureGroup[currentCaptureGroupIndex].x = i;
        pebblesInCurrentCaptureGroup[currentCaptureGroupIndex].y = o;
        currentCaptureGroupIndex++;

        startCheckingFromHere(i,o);
        // after the checks occured the spaces put into the array should be the current territory to capture,
        // if the checks never reached an empty space then capture the pieces
        if(currentCaptureGroupReachedEmpty == FALSE){
          // osSyncPrintf("-------The capture group found an enclosed section!-----\n");
          // before capturing the pieces we need to check if its a self capture
          if(currentCaptureGroupColour == (*board[cursorPos[padNo].x][cursorPos[padNo].y].content).colour){
            // is a self capture, so capture will not take place, and pebble will be 'un placed'
            undoPebblePlace(padNo);
            // BONG noise
            nuAuSndPlayerPlay(1);
            return 0;
          }else{
            // capture pieces
            for(k=0;k<currentCaptureGroupIndex;++k){
              removePebble(pebblesInCurrentCaptureGroup[k].x,pebblesInCurrentCaptureGroup[k].y);
            } 
            //pling noise
            nuAuSndPlayerPlay(6);
            return 1;
          }
        }
      }
    }
  }
  return 2;
}
void startCheckingFromHere(int x, int y){
  // osSyncPrintf("expanding checking from %d,%d\n",x,y);
  checkCapturesContinues(x+1,y);
  checkCapturesContinues(x  ,y+1);
  checkCapturesContinues(x-1,y  );
  checkCapturesContinues(x  ,y-1  );
}
void checkCapturesContinues(int x, int y){
  // osSyncPrintf("continue checking %d,%d\n",x,y);
  if(isOffBoard(x,y) == FALSE){
    if(board[x][y].isEmpty == TRUE){
      // Found an empty space in the capture, so stop altogether
      // osSyncPrintf("%d,%d reached empty\n",x,y);
      currentCaptureGroupReachedEmpty = TRUE;

    //  only continue if the space is on the board, same colour as the current group, and other checks haven't found empty space.
    }else if(currentCaptureGroupReachedEmpty == FALSE && isInCurrentCaptureGroup(x,y) == FALSE){
      if(currentCaptureGroupColour == (*board[x][y].content).colour){
        pebblesInCurrentCaptureGroup[currentCaptureGroupIndex].x = x;
        pebblesInCurrentCaptureGroup[currentCaptureGroupIndex].y = y;
        currentCaptureGroupIndex++;
        // osSyncPrintf("%d,%d was correct and the search continues\n",x,y);
        startCheckingFromHere(x,y);
      }
    }
  }
}
int isInCurrentCaptureGroup(int x, int y){
  // osSyncPrintf("/\\/\\ welcome to the isInCUrrentCaptureGroup function! /\\/\\\n");
  for(k=0;k<currentCaptureGroupIndex;k++){
    // osSyncPrintf("comparing at index: %d",k);
    // osSyncPrintf("x: %d, y: %d",pebblesInCurrentCaptureGroup[k].x,pebblesInCurrentCaptureGroup[k].y);
    if(pebblesInCurrentCaptureGroup[k].x == x && pebblesInCurrentCaptureGroup[k].y == y){
      return TRUE;
    }
  }
  return FALSE;
}
void calculateTerritories(){
  whiteTerritoryScore = 0;
  blackTerritoryScore = 0;
  // spacesCheckedForTerritoryIndex = 0;
          resetTerritoryGroup();
  sizeOfTerritory = 0;

  if( turnCount > 1 ){
    for(i=0;i<squareCount; ++i){   
      for(o=0;o<squareCount; ++o){
        //calculate the territories, very similar to capture checking except no capturing takes place
        if(board[i][o].isEmpty == TRUE && isInCurrentTerritoryGroup(i,o) == FALSE){
          // osSyncPrintf("\n-----A capture chain has begun-----\n");
          // set values for current check
          sizeOfTerritory = 0;

          currentTerritoryGroupReachedWrongPebble = FALSE;
          currentTerritoryGroupReachedPebble = FALSE;
          // currentTerritoryGroupColour = (*board[i][o].content).colour;
          // osSyncPrintf("Starting from %d,%d, colour is: %d\n",i,o,currentCaptureGroupColour);
          // add current to list of pebbles checked
          // addToTerritoryPlacesChecked(i,o);
          pebblesInCurrentTerritoryGroup[currentTerritoryGroupIndex].x = i;
          pebblesInCurrentTerritoryGroup[currentTerritoryGroupIndex].y = o;
          currentTerritoryGroupIndex++;
          sizeOfTerritory++;
          // spacesAlreadyCheckedForTerritory[spacesCheckedForTerritoryIndex].x = i;
          // spacesAlreadyCheckedForTerritory[spacesCheckedForTerritoryIndex].y = o;
          // spacesCheckedForTerritoryIndex++;

          startCheckingTerritoriesFromHere(i,o);
          // after the checks occured the spaces put into the array should be the current territory to capture,
          // if the checks never reached an empty space then capture the pieces
          if(currentTerritoryGroupReachedWrongPebble == FALSE){
            // osSyncPrintf("-------The capture group found an enclosed section!-----\n");
            if(currentTerritoryGroupColour == WHITE){
              // whiteTerritoryScore += currentTerritoryGroupIndex;
              whiteTerritoryScore += sizeOfTerritory;
              sizeOfTerritory = 0;
            }else{
              // blackTerritoryScore += currentTerritoryGroupIndex;
              blackTerritoryScore += sizeOfTerritory;
              sizeOfTerritory = 0;
            }
          }
        }
      }
    }
  }
}
void resetTerritoryGroup(){
  currentTerritoryGroupIndex = 0;
  // for (i=0;i<100;i++){
  //   pebblesInCurrentCaptureGroup[i] = NULL;
  // }
}
void startCheckingTerritoriesFromHere(int x, int y){
  // osSyncPrintf("expanding checking from %d,%d\n",x,y);
  checkTerritoryContinues(x+1,y  );
  checkTerritoryContinues(x  ,y+1);
  checkTerritoryContinues(x-1,y  );
  checkTerritoryContinues(x  ,y-1);
}
void checkTerritoryContinues(int x, int y){
  // osSyncPrintf("continue checking %d,%d\n",x,y);
  if(isOffBoard(x,y) == FALSE ){
    if(board[x][y].isEmpty == TRUE /*&& isAlreadyCheckedForTerritory(x,y) == FALSE*/){
      if(isInCurrentTerritoryGroup(x,y) == FALSE){
        // Found an empty space in the capture, so stop altogether
        // osSyncPrintf("%d,%d reached empty\n",x,y);
        pebblesInCurrentTerritoryGroup[currentTerritoryGroupIndex].x = x;
        pebblesInCurrentTerritoryGroup[currentTerritoryGroupIndex].y = y;
        currentTerritoryGroupIndex++;
        sizeOfTerritory++;
        // spacesAlreadyCheckedForTerritory[spacesCheckedForTerritoryIndex].x = x;
        // spacesAlreadyCheckedForTerritory[spacesCheckedForTerritoryIndex].y = y;
        // spacesCheckedForTerritoryIndex++;
        // osSyncPrintf("%d,%d was correct and the search continues\n",x,y);
        startCheckingTerritoriesFromHere(x,y);
      }
    //  only continue if the space is on the board, same colour as the current group, and other checks haven't found empty space.
    }else if(currentTerritoryGroupReachedPebble == FALSE){
      currentTerritoryGroupColour = (*board[x][y].content).colour;
      currentTerritoryGroupReachedPebble = TRUE;
    }else if(currentTerritoryGroupReachedPebble == TRUE){
      //second time reaching a pebble, best be the same colour as before or we are stopping
      if(currentTerritoryGroupColour != (*board[x][y].content).colour){
        currentTerritoryGroupReachedWrongPebble = TRUE;
      }
    }
  }
}
int isInCurrentTerritoryGroup(int x, int y){
  // osSyncPrintf("/\\/\\ welcome to the isInCUrrentCaptureGroup function! /\\/\\\n");
  for(k=0;k<currentTerritoryGroupIndex;k++){
    // osSyncPrintf("comparing at index: %d",k);
    // osSyncPrintf("x: %d, y: %d",pebblesInCurrentCaptureGroup[k].x,pebblesInCurrentCaptureGroup[k].y);
    if(pebblesInCurrentTerritoryGroup[k].x == x && pebblesInCurrentTerritoryGroup[k].y == y){
      return TRUE;
    }
  }
  return FALSE;
}
int isAlreadyCheckedForTerritory(int x, int y){
  // osSyncPrintf("/\\/\\ welcome to the isInCUrrentCaptureGroup function! /\\/\\\n");
  for(k=0;k<spacesCheckedForTerritoryIndex;k++){
    // osSyncPrintf("comparing at index: %d",k);
    // osSyncPrintf("x: %d, y: %d",pebblesInCurrentCaptureGroup[k].x,pebblesInCurrentCaptureGroup[k].y);
    if(spacesAlreadyCheckedForTerritory[k].x == x && spacesAlreadyCheckedForTerritory[k].y == y){
      return TRUE;
    }
  }
  return FALSE;
}
void checkGameOver(){
  if(turnCount >= maxTurns || (lastTurnWasSkipped == TRUE && thisTurnWasSkipped == TRUE)){
    gameOver = TRUE;
    setTopDownCamera();
    // nuAuSeqPlayerStop(0);
    // nuAuSeqPlayerSetNo(0,0);
    // nuAuSeqPlayerPlay(0);
  }
}
// the 'draw' function
void makeDL01() {
  unsigned short perspNorm;
  GraphicsTask * gfxTask;
  
  char conbuf[30];
  
  
  // switch the current graphics task
  // also updates the displayListPtr global variable
  gfxTask = gfxSwitchTask(); 
  

  // prepare the RCP for rendering a graphics task
  gfxRCPInit();

  // clear the color framebuffer and Z-buffer, similar to glClear()
  gfxClearCfb();
 
  // initialize the projection matrix, similar to glPerspective() or glm::perspective()
  guPerspective(&gfxTask->projection, &perspNorm, FOVY, ASPECT, NEAR_PLANE,
                FAR_PLANE, 1.0);
  
  // Our first actual displaylist command. This writes the command as a value at
  // the tail of the current display list, and we increment the display list
  // tail pointer, ready for the next command to be written.
  // As for what this command does... it's just required when using a perspective
  // projection. Try pasting 'gSPPerspNormalize' into google if you want more
  // explanation, as all the SDK documentation has been placed online by
  // hobbyists and is well indexed.
  gSPPerspNormalize(displayListPtr++, perspNorm);

  // initialize the modelview matrix, similar to gluLookAt() or glm::lookAt()
  guLookAt(&gfxTask->modelview, cameraPos.x, cameraPos.y,
           cameraPos.z, cameraTarget.x, cameraTarget.y,
           cameraTarget.z, cameraUp.x, cameraUp.y, cameraUp.z);
           
  // guLookAt(&gfxTask->modelview, -xLocation - 400.0f, zLocation,
  //          cameraPos.z, -xLocation, zLocation,
  //          cameraTarget.z, cameraUp.x, cameraUp.y, cameraUp.z);

  // load the projection matrix into the matrix stack.
  // given the combination of G_MTX_flags we provide, effectively this means
  // "replace the projection matrix with this new matrix"

  gSPMatrix(
    displayListPtr++,
    // we use the OS_K0_TO_PHYSICAL macro to convert the pointer to this matrix
    // into a 'physical' address as required by the RCP 
    OS_K0_TO_PHYSICAL(&(gfxTask->projection)),
    // these flags tell the graphics microcode what to do with this matrix
    // documented here: http://n64devkit.square7.ch/tutorial/graphics/1/1_3.htm
    G_MTX_PROJECTION | // using the projection matrix stack...
    G_MTX_LOAD | // don't multiply matrix by previously-top matrix in stack
    G_MTX_NOPUSH // don't push another matrix onto the stack before operation
  );

  



  gSPMatrix(displayListPtr++,
    OS_K0_TO_PHYSICAL(&(gfxTask->modelview)),
    // similarly this combination means "replace the modelview matrix with this new matrix"
    G_MTX_MODELVIEW | G_MTX_NOPUSH | G_MTX_LOAD
  );
	gSPSetLights0(displayListPtr++, my_ambient_only_light);
	gSPSetLights1(displayListPtr++, sun_light);

  //sets the background to white
  gDPSetCycleType(displayListPtr++, G_CYC_FILL);
  gDPSetRenderMode(displayListPtr++, G_RM_OPA_SURF, G_RM_OPA_SURF);
  if(gameOver == FALSE){
    gDPSetFillColor(displayListPtr++, screenWhite);
  }else{
    gDPSetFillColor(displayListPtr++, (blackCaptureScore+blackTerritoryScore > whiteCaptureScore+whiteTerritoryScore) ? screenBlack:screenWhite);
  }
  gDPFillRectangle(displayListPtr++, 0, 0, SCREEN_WD-1, SCREEN_HT-1);
  gDPNoOp(displayListPtr++);
  gDPPipeSync(displayListPtr++);

  {
    if(gameOver== FALSE){
      CURRENT_GFX = -1;
      
      CURRENT_GFX++;
      guPosition(&gfxTask->objectTransforms[CURRENT_GFX],0.0f,0.0f,0.0f,2.5f, debugX, 50.0f, debugY);
      gSPMatrix(displayListPtr++,
          OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
          G_MTX_MODELVIEW | // operating on the modelview matrix stack...
          G_MTX_PUSH | // ...push another matrix onto the stack...
          G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
        );
      drawTransModel(Wtx_selectionring);
      gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);

    
    // for(i=0;i<(squareCount-1)/2; ++i){   
    //   for(o=0;o<(squareCount-1)/2; ++o){ 
    //     CURRENT_GFX++;
    //     guPosition(&gfxTask->objectTransforms[CURRENT_GFX],
    //     0.0f, //angle it to be flat
    //     0.0f, 0.0f, 
    //     (2.5f), //scale based on square size
    //     i * squareSize*2.0f - boardSize/2.0f + squareSize,  //x, the *2 is the same as the /2 in the for loop
    //     0.0f,//y move down
    //     o * squareSize*2.0f - boardSize/2.0f + squareSize); //z

    //     gSPMatrix(displayListPtr++,
    //       OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
    //       G_MTX_MODELVIEW | // operating on the modelview matrix stack...
    //       G_MTX_PUSH | // ...push another matrix onto the stack...
    //       G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
    //     );
    //     drawModel(Wtx_boardsquare);
    //     gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
    //   }
    // }

    // for(i=0;i<2; ++i){   
    //   for(o=0;o<2; ++o){ 
        CURRENT_GFX++;
        guPosition(&gfxTask->objectTransforms[CURRENT_GFX],
        0.0f, //angle it to be flat
        0.0f, 0.0f, 
        (2.5f), //scale based on square size
        0.0f,  //x, the *2 is the same as the /2 in the for loop
        0.0f,//y move down
        0.0f); //z

        gSPMatrix(displayListPtr++,
          OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
          G_MTX_MODELVIEW | // operating on the modelview matrix stack...
          G_MTX_PUSH | // ...push another matrix onto the stack...
          G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
        );
        drawModel(Wtx_boardsquarealt);
        gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
    //   }
    // }
      

      CURRENT_GFX++;
      guPosition(&gfxTask->objectTransforms[CURRENT_GFX],
      0.0f, //angle it to be flat
      0.0f, 0.0f, 
      2.5f, //scale based on square size
      0.0f,  //x
      0.0f,//y move down
      0.0f); //z

      gSPMatrix(displayListPtr++,
        OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
        G_MTX_MODELVIEW | // operating on the modelview matrix stack...
        G_MTX_PUSH | // ...push another matrix onto the stack...
        G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
      );
      drawModel(Wtx_boardedge);
      gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);

    // for(i=0;i<4;i++){
    //   CURRENT_GFX++;
    //   guPosition(&gfxTask->objectTransforms[CURRENT_GFX],
    //   0.0f, //angle it to be flat
    //   i*90.0f, 0.0f, 
    //   2.5f, //scale based on square size
    //   0.0f,  //x
    //   0.0f,//y move down
    //   0.0f); //z

    //   gSPMatrix(displayListPtr++,
    //     OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
    //     G_MTX_MODELVIEW | // operating on the modelview matrix stack...
    //     G_MTX_PUSH | // ...push another matrix onto the stack...
    //     G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
    //   );
    //   if(i < 2){
    //     drawModel(Wtx_boardside);
    //   }else{
    //     drawModel(Wtx_boardside_mirror);
    //   }
    //   gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
    // }

    // CURRENT_GFX++;
    // guPosition(&gfxTask->objectTransforms[CURRENT_GFX],
    // 0.0f, //angle it to be flat
    // 0.0f, 0.0f, 
    // 2.5f, //scale based on square size
    // 0.0f,  //x
    // 0.0f,//y move down
    // 0.0f); //z

    // gSPMatrix(displayListPtr++,
    //   OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
    //   G_MTX_MODELVIEW | // operating on the modelview matrix stack...
    //   G_MTX_PUSH | // ...push another matrix onto the stack...
    //   G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
    // );
    // drawModel(Wtx_floor);
    // gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
// 
    // 
// 
    // CURRENT_GFX++;
    // guPosition(&gfxTask->objectTransforms[CURRENT_GFX],
    // 0.0f, //angle it to be flat
    // 0.0f, 0.0f, 
    // 2.5f, //scale based on square size
    // 0.0f,  //x
    // 0.0f,//y move down
    // 0.0f); //z

    // gSPMatrix(displayListPtr++,
    //   OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
    //   G_MTX_MODELVIEW | // operating on the modelview matrix stack...
    //   G_MTX_PUSH | // ...push another matrix onto the stack...
    //   G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
    // );
    // drawModel(Wtx_table);
    // gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
   
      for(i=0;i<pebbleCount; ++i){

        CURRENT_GFX++;

        // guPosition(&gfxTask->objectTransforms[CURRENT_GFX], 0.0f, timer + -(pebbles[i].rotation*(180/M_PI)) ,0.0f,1.0f, pebbles[i].position.x, 50 + 30*sin(pebbles[i].position.y +(float)timer/30.0f), pebbles[i].position.z);
        guPosition(&gfxTask->objectTransforms[CURRENT_GFX], 0.0f, pebbles[i].rotation ,0.0f,0.8f, pebbles[i].position.x, pebbles[i].position.y, pebbles[i].position.z);

        gSPMatrix(displayListPtr++,
          OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
          G_MTX_MODELVIEW | // operating on the modelview matrix stack...
          G_MTX_PUSH | // ...push another matrix onto the stack...
          G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
        );
        if(pebbles[i].colour == WHITE){
          drawModel(Wtx_pebble_white);
        }else{
          drawModel(Wtx_pebble_black);
        }
        gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);

        
      }

      /* the pebble that hovers above the cursor */
      for(padNo=0;padNo<MAX_PLAYERS;padNo++){
        if(nuContStatus[padNo].errno != 0){
        continue;
        }
        
        if(turnCount%2 == padNo){
          //dont display the square if not that players turn
          CURRENT_GFX++;
          guPosition(&gfxTask->objectTransforms[CURRENT_GFX],0.0f,0.0f,0.0f,2.5f, xGoal[padNo], 0.0f, zGoal[padNo]);
          gSPMatrix(displayListPtr++,
              OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
              G_MTX_MODELVIEW | // operating on the modelview matrix stack...
              G_MTX_PUSH | // ...push another matrix onto the stack...
              G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
            );
          drawTransModel(Wtx_selectionring);
          gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
        }

        CURRENT_GFX++;
        guPosition(&gfxTask->objectTransforms[CURRENT_GFX], 0.0f, timer*2%360, 0.0f, 0.6f, xLocation[padNo], (turnCount % 2 == padNo) ? 40 + 20*sin((float)timer/30.0f+padNo*2) : 30, zLocation[padNo]);
        gSPMatrix(displayListPtr++,
          OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
          G_MTX_MODELVIEW | // operating on the modelview matrix stack...
          G_MTX_PUSH | // ...push another matrix onto the stack...
          G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
        );
        
        if(padNo%2==0){
          drawSmoothModel(Wtx_pebble_black);
        }else{
          drawSmoothModel(Wtx_pebble_white);
        }
        gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
      }	
      drawBanner();
    }
  }

  if( gameOver == FALSE){
    if (gamePause == TRUE){
      // sprintf(outstring,"- Game Paused -");
      // Draw8Font(114,100,TEX_COL_RED,0);
      sprintf(outstring,"%c Resume", menuArrow[0]);
      if(btn_down==TRUE && menuSelection == 0){
      Draw8Font(114,100,TEX_COL_BLACK,0);
      }else{
      Draw8Font(114,100,TEX_COL_WHITE,0);
      }
      sprintf(outstring,"%c Quit", menuArrow[1]);
      if(btn_down==TRUE && menuSelection == 1){
      Draw8Font(114,120,TEX_COL_BLACK,0);
      }else{
      Draw8Font(114,120,TEX_COL_WHITE,0);
      }
    }else{
      sprintf(outstring,"White - %d",whiteCaptureScore + whiteTerritoryScore);
      wScoreLoc = 7;
      Draw8Font(wScoreLoc,5, TEX_COL_BLACK, 0);

      sprintf(outstring,"%d - Black",blackCaptureScore + blackTerritoryScore);
      bScoreLoc = 252 - 8 * numDigits(blackCaptureScore + blackTerritoryScore); 
      Draw8Font(bScoreLoc,5, TEX_COL_WHITE, 0);
    }
  }else{
    sprintf(outstring,"Game Over");
    Draw8Font(130,90, TEX_COL_RED, 0);

    // if(0x30 > (nuScRetraceCounter & 0x30)){
    //   sprintf(outstring,"PR");
    //   Draw8Font(110,110, TEX_COL_BLACK, 0);
    // }  
    sprintf(outstring,"%s WINS!", (blackCaptureScore+blackTerritoryScore > whiteCaptureScore+whiteTerritoryScore) ? "BLACK":"WHITE");
    Draw8Font(120, 130, (blackCaptureScore+blackTerritoryScore > whiteCaptureScore+whiteTerritoryScore) ? TEX_COL_WHITE:TEX_COL_BLACK, 0);
    // sprintf(outstring,"White - %d vs %d - Black",whiteCaptureScore, blackCaptureScore);
    // Draw8Font(8*(int)numDigits(whiteCaptureScore+blackCaptureScore)/2,110, TEX_COL_WHITE, 0);
  }
  // drawHudGraphic(turnCount);
  

  // mark the end of the display list
  gDPFullSync(displayListPtr++);
  gSPEndDisplayList(displayListPtr++);

  // assert that the display list isn't longer than the memory allocated for it,
  // otherwise we would have corrupted memory when writing it.
  // isn't unsafe memory access fun?
  // this could be made safer by instead asserting on the displaylist length
  // every time the pointer is advanced, but that would add some overhead.
  assert(displayListPtr - gfxTask->displayList < MAX_DISPLAY_LIST_COMMANDS);

  // create a graphics task to render this displaylist and send it to the RCP
  nuGfxTaskStart(
    gfxTask->displayList,
    (int)(displayListPtr - gfxTask->displayList) * sizeof (Gfx),
    NU_GFX_UCODE_F3DEX, // load the 'F3DEX' version graphics microcode, which runs on the RCP to process this display list
    NU_SC_SWAPBUFFER // tells NuSystem to immediately display the frame on screen after the RCP finishes rendering it
  );

  /*
  ---------------------------start debug on screen-------------------------------------
  */
  if(debugMode == TRUE){
    // nuDebConTextPos(0,0,0);
    // sprintf(conbuf,"the answer is: %f",atan2bodyf(M_PI,2));
    // nuDebConCPuts(0, conbuf);

    // nuDebConTextPos(0,0,0);
    // sprintf(conbuf,"White: %d",whiteCaptureScore);
    // nuDebConCPuts(0, conbuf);
    // nuDebConTextPos(0,0,1);
    // sprintf(conbuf,"Black: %d",blackCaptureScore);
    // nuDebConCPuts(0, conbuf);
    // nuDebConTextPos(0,0,3);
    // sprintf(conbuf,"Turns: %d",turnCount);
    // nuDebConCPuts(0, conbuf);
    // nuDebConTextPos(0,0,4); 
    // sprintf(conbuf,"Stage no: %d",stage);
    // nuDebConCPuts(0, conbuf);


    nuDebConTextPos(0,0,3);
    sprintf(conbuf,"debug:%f, %f",debugX, debugY);
    nuDebConCPuts(0, conbuf);
    // nuDebConTextPos(0,0,3);
    // sprintf(conbuf,"tempo:%f",boardSize);
    // nuDebConCPuts(0, conbuf);
    // nuDebConTextPos(0,0,4);
    // sprintf(conbuf,"debug:%d, %d",numDigits((int)debugY), bScoreLoc);
    // nuDebConCPuts(0, conbuf);
    // if(board[0][0].isEmpty == FALSE){
    //   nuDebConTextPos(0,0,1);
    //   sprintf(conbuf,"0,0: %d",(*board[0][0].content).colour);
    //   nuDebConCPuts(0, conbuf);
    // }
  }
 
  /*
  ---------------------------end debug on screen---------------------------------------
  */
  /*  character written to frame buffer */
  nuDebConDisp(NU_SC_SWAPBUFFER);
}


// A static array of model vertex data.
// This include the position (x,y,z), texture U,V coords (called S,T in the SDK)
// and vertex color values in r,g,b,a form.
// As this data will be read by the RCP via direct memory access, which is
// required to be 16-byte aligned, it's a good idea to annotate it with the GCC
// attribute `__attribute__((aligned (16)))`, to force it to be 16-byte aligned.
Vtx squareVerts[] __attribute__((aligned (16))) = {
  //  x,   y,  z, flag, S, T,    r,    g,    b,    a
  { -10,   0,  10,    0, 0, 0, 0x00, 0xff, 0x00, 0xff  },
  {  10,   0,  10,    0, 0, 0, 0x00, 0x00, 0x00, 0xff  },
  {  10,   0, -10,    0, 0, 0, 0x00, 0x00, 0xff, 0xff  },
  { -10,   0, -10,    0, 0, 0, 0xff, 0x00, 0x00, 0xff  },
};

void drawSquare() {
  
  // load vertex data for the triangles
  gSPVertex(displayListPtr++, &(squareVerts[0]), 4, 0);


  // depending on which graphical features, the RDP might need to spend 1 or 2
  // cycles to render a primitive, and we need to tell it which to do
  gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
  // use antialiasing, rendering an opaque surface
  gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
  // reset any rendering flags set when drawing the previous primitive
  gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
  // enable smooth (gourad) shading and z-buffering
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);

  // actually draw the triangles, using the specified vertices
  gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0);

  // Mark that we've finished sending commands for this particular primitive.
  // This is needed to prevent race conditions inside the rendering hardware in 
  // the case that subsequent commands change rendering settings.
  gDPPipeSync(displayListPtr++);
}

// void drawPebble() {
//   gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
//   gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
//   gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
//   gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
  
//   // The gSPDisplayList command causes the RCP to render a static display list,
//   // then return to this display list afterwards. These 4 display lists are
//   // defined in n64logo.h, and were generated from a 3D model using a conversion
//   // script.
//   // gSPDisplayList(displayListPtr++, N64Red_PolyList);
//   // gSPDisplayList(displayListPtr++, N64Green_PolyList);
//   // gSPDisplayList(displayListPtr++, N64Blue_PolyList);
//   // gSPDisplayList(displayListPtr++, N64Yellow_PolyList);
//   // gSPDisplayList(displayListPtr++, Wtx_triangle);
//   gSPDisplayList(displayListPtr++, Wtx_pebble);
//   gDPPipeSync(displayListPtr++);
// }
// void drawCross(){
//   gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
//   gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
//   gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
//   gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
//   gSPDisplayList(displayListPtr++, Wtx_cross);
//   gDPPipeSync(displayListPtr++);
// }
void drawModel(modelName){
  gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
  gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
  gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
  gSPDisplayList(displayListPtr++, modelName);
  gDPPipeSync(displayListPtr++);
}
void drawTransModel(modelName){
  gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
  gDPSetTextureFilter(displayListPtr++, G_TF_AVERAGE);
  gDPSetRenderMode(displayListPtr++,G_RM_AA_ZB_XLU_DECAL, G_RM_AA_ZB_XLU_DECAL2);
  gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
  gSPDisplayList(displayListPtr++, modelName);
  gDPPipeSync(displayListPtr++);
}
void drawSmoothModel(modelName){
  gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
  gDPSetTextureFilter(displayListPtr++, G_TF_BILERP);
  gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
  gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
  gSPDisplayList(displayListPtr++, modelName);
  gDPPipeSync(displayListPtr++);
}

// void drawModelInPosition(GraphicsTask *gft, void (*modelName), float roll, float pitch, float yaw, float scale, float transx, float transy, float transz){
//     CURRENT_GFX++;
//     guPosition(&gft->objectTransforms[CURRENT_GFX],
//     roll, //angle it to be flat
//     pitch, yaw, 
//     scale, //scale based on square size
//     transx,  //x
//     transy,//y move down
//     transz); //z

//     gSPMatrix(displayListPtr++,
//       OS_K0_TO_PHYSICAL(&(gft->objectTransforms[CURRENT_GFX])),
//       G_MTX_MODELVIEW | // operating on the modelview matrix stack...
//       G_MTX_PUSH | // ...push another matrix onto the stack...
//       G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
//     );
//     drawModel(*modelName);
//     gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
// }
//sets the background to white
void drawBanner(){
  //displays banner where the scores go

  //white behind left 3rd
  gDPSetCycleType(displayListPtr++, G_CYC_FILL);
  gDPSetRenderMode(displayListPtr++, G_RM_OPA_SURF, G_RM_OPA_SURF);
  gDPSetFillColor(displayListPtr++, screenWhite);
  gDPFillRectangle(displayListPtr++, 0, 0, (int)(SCREEN_WD/3)-1, bannerHeight);
  gDPNoOp(displayListPtr++);
  gDPPipeSync(displayListPtr++);

  // middle 3rd
  gDPSetCycleType(displayListPtr++, G_CYC_FILL);
  gDPSetRenderMode(displayListPtr++, G_RM_OPA_SURF, G_RM_OPA_SURF);
  if(thisTurnWasSkipped == TRUE){
    gDPSetFillColor(displayListPtr++, screenRed);
  }else{
    gDPSetFillColor(displayListPtr++, (turnCount%2==0) ? screenBlack : screenWhite);
  }
  gDPFillRectangle(displayListPtr++, (int)(SCREEN_WD/3)-1, 0, (int)(SCREEN_WD/3)*2-1, bannerHeight);
  gDPNoOp(displayListPtr++);
  gDPPipeSync(displayListPtr++);

  // black behind 3rd, 3rd
  gDPSetCycleType(displayListPtr++, G_CYC_FILL);
  gDPSetRenderMode(displayListPtr++, G_RM_OPA_SURF, G_RM_OPA_SURF);
  gDPSetFillColor(displayListPtr++, screenBlack);
  gDPFillRectangle(displayListPtr++, (int)(SCREEN_WD/3)*2-1, 0, SCREEN_WD-1, bannerHeight);
  gDPNoOp(displayListPtr++);
  gDPPipeSync(displayListPtr++);

  //line underneath the header
  gDPSetCycleType(displayListPtr++, G_CYC_FILL);
  gDPSetRenderMode(displayListPtr++, G_RM_OPA_SURF, G_RM_OPA_SURF);
  gDPSetFillColor(displayListPtr++, screenBlack);
  gDPFillRectangle(displayListPtr++, 0, bannerHeight, SCREEN_WD-1, 20);
  gDPNoOp(displayListPtr++);
  gDPPipeSync(displayListPtr++);
}
int numDigits(int n){
  if( n < 10 ) return 1;
  if( n < 100 ) return 2;
  if( n < 1000 ) return 3;
  if( n < 10000 ) return 4;
  if( n < 100000 ) return 5;
  if( n < 1000000 ) return 6;
  if( n < 10000000 ) return 7;
  return 8;
}
/* Provide playback and control of audio by the button of the controller */
// void soundCheck(void)
// {
//   static int snd_no = 0;
//   static int seq_no = 0;

//   /* Change music of sequence playback depending on the top and bottom of 
//   the cross key */
//   if((contdata[0].trigger & U_JPAD) || (contdata[0].trigger & D_JPAD))
//     {
//       if(contdata[0].trigger & U_JPAD)
// 	{
// 	  seq_no--;
// 	  if(seq_no < 0) seq_no = 2;
// 	}
//       else
// 	{
// 	  seq_no++;
// 	  if(seq_no > 2) seq_no = 0;
// 	}	  

//       nuAuSeqPlayerStop(0);
//       nuAuSeqPlayerSetNo(0,seq_no);
//       nuAuSeqPlayerPlay(0);
//     }

//   /* Possible to play audio in order by right and left of the cross key */
//   if((contdata[0].trigger & L_JPAD) || (contdata[0].trigger & R_JPAD))
//     {
//       if(contdata[0].trigger & L_JPAD)
// 	{
// 	  snd_no--;
// 	  if(snd_no < 0) snd_no = 10;
// 	}
//       else
// 	{
// 	  snd_no++;
// 	  if(snd_no > 10) snd_no = 0;
// 	}	  

//       /* Eleven sounds (sound data items) are provided.  Of these, the first 10 are sampled at 44 KHz and the 11th at 24 KHz. */
//       nuAuSndPlayerPlay(snd_no); 
//       if(snd_no < 10)
// 	nuAuSndPlayerSetPitch(44100.0/32000);
//       else
// 	nuAuSndPlayerSetPitch(24000.0/32000);
//     }

//   /* Change tempo of sequence playback by the L and R buttons */
//   if((contdata[0].trigger & L_TRIG) || (contdata[0].trigger & R_TRIG))
//     {
//       s32 tmp;
//       tmp = nuAuSeqPlayerGetTempo(0);

//       if(contdata[0].trigger & L_TRIG)
// 	{
// 	  tmp /= 10;
// 	  tmp *= 8;
// 	}
//       else
// 	{
// 	  tmp /= 10;
// 	  tmp *= 12;
// 	}
//       nuAuSeqPlayerSetTempo(0,tmp);
//     }

//   /* Fade out sound by pushing the Z button */
//   if(contdata[0].trigger & Z_TRIG)
//     {
//       nuAuSeqPlayerFadeOut(0,200);
//     }
// }

// the nusystem callback for the stage, called once per frame
void stage01(int pendingGfx)
{
  // produce a new displaylist (unless we're running behind, meaning we already
  // have the maximum queued up)
  if(pendingGfx < 1)
    makeDL01();

  nuContDataGetExAll(contdata);

  // update the state of the world for the next frame
  updateGame01();
}

