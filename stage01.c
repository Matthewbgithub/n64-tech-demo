
#include <assert.h>
#include <nusys.h> 
/*includes */
#include "graphic.h"
#include "main.h"
#include "stage01.h"
#include "math.c"
/* my models */
#include "pebble_black.h"
#include "pebble_white.h"
#include "triangle.h"
#include "cross_by_three.h"
/*borrowed models for testing*/
#include "n64logo.h"
#include "kabe.h"
#include "texture.h"

#define MAX_DROPS 100

#ifdef N_AUDIO
#include <nualsgi_n.h>
#else
#include <nualsgi.h>
#endif

#ifndef WHITE
#define WHITE    1
#endif  /* WHITE */

#ifndef BLACK
#define BLACK   2
#endif  /* BLACK */


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
int objectCount;
int squareCount;
boardSquare board[9][9];
float sqaureSize;
float boardSize;
/*----------------------*/

/* game values --------------*/
int2d cursorPos = {0,0};
int turnCount;
float xLocation;
float zLocation;
float xGoal;
float zGoal;
int blackScore;
int whiteScore;
const Vec2d whitePotLocation = {300.0f,0.0f};
const Vec2d blackPotLocation = {-300.0f,0.0f};
int maxTurns;
int gameOver;
/*---------------------------*/

/* capture variables --------*/
int currentCaptureGroupColour;
int currentCaptureGroupReachedEmpty;
// if the array size is changed, change the for loop limit in the resetCaptureGroup function too
Vec2d pebblesInCurrentCaptureGroup[100];
int currentCaptureGroupIndex;
/*---------------------------*/

/*----- functions -----------*/
int CURRENT_GFX;
// loop values, unfortunately we have a 3 layer deep for loop :(
int i;
int o;
int k;
int timer;

float stickMoveX;
float stickMoveY;
float moveSpeedSteps;
int returnedToCenter;
int stickResetFromTimer;
/*-----------------------*/


// the 'setup' function
void initStage01() {  
  // the advantage of initializing these values here, rather than statically, is
  // that if you switch stages/levels, and later return to this stage, you can
  // call this function to reset these values.
  
  //moving values
  stickMoveX = 0;
  stickMoveY = 0;
  xLocation = 0;
  zLocation = 0;
  xGoal = xLocation;
  zGoal = zLocation;
  moveSpeedSteps = 20;
  returnedToCenter = TRUE;

  for (i=0; i<MAX_DROPS;i++){
    pebbles[i].position.x = NULL;
    pebbles[i].position.y = NULL;
    pebbles[i].position.z = NULL;
    pebbles[i].rotation = 0.0f;
    pebbles[i].colour = NULL;
  }
  objectCount = 0;

  /*initialise board size and scale*/
  squareCount = 9;
  for (i=0;i<squareCount;i++){
    for (o=0;o<squareCount;o++){
      board[i][o].isEmpty = TRUE;
    }
  }
  sqaureSize = 50;
  boardSize = sqaureSize*squareCount;
  moveCursor(0,0);

  cameraPos.x = 0.0f;
  cameraPos.y = 0.0f;
  cameraPos.z = 0.0f;
  cameraDistance = 5.0f*(squareCount*squareCount) + 195.0f;
  cameraRotation.x = 0.0f;
  cameraRotation.y = 1.2f;
  cameraRotation.z = 0.0f;

  timer = 0;
	turnCount = 0;
  blackScore = 0;
  whiteScore = 0;
  stickResetFromTimer = 0;
  maxTurns = 30;
  gameOver = FALSE;  
  resetCaptureGroup();
}

// the 'update' function
void updateGame01() {
  timer++;
  // read controller input from controller 1 (index 0)
  nuContDataGetEx(contdata, 0);
  // We check if the 'A' Button was pressed using a bitwise AND with
  // contdata[0].trigger and the A_BUTTON constant.
  // The contdata[0].trigger property is set only for the frame that the button is
  // initially pressed. The contdata[0].button property is similar, but stays on
  // for the duration of the button press.
  if( gameOver == FALSE){
    if (contdata[0].trigger & START_BUTTON){
      //when a button is pressed, reset the scene
      initStage01();
    } else {
      if (contdata[0].trigger & A_BUTTON){
        //when a button is pressed, reset the scene
        takeTurn();
      }
      if (contdata[0].trigger & B_BUTTON){
        //nothing lol, maybe could skip turn?
        // removePebble(cursorPos.x,cursorPos.y);

      }
    }
    moveCamera();
    movePebble();
  }else{
    
  }
  

  //moving the pebbles
  // for(i=0; i<objectCount;i++){
  //   pebbles[i].position.x = pebbles[i].position.x + pebbles[i].velocity*cos(pebbles[i].rotation);
  //   pebbles[i].position.z = pebbles[i].position.z + pebbles[i].velocity*sin(pebbles[i].rotation);
  // }
 
  


  /* check contact with n64 */
  // for( i =0; i< objectCount;i++){
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
void takeTurn(){
    placeCounter();
    checkForCaptures();
    checkGameOver();
}
void moveCamera(){
  //c buttons to move the camera round please
  if (contdata[0].button & U_CBUTTONS)
    cameraRotation.y -= 0.05;
    if(cameraRotation.y <= -(M_PI/2)+0.01f){
      cameraRotation.y = -(M_PI/2)+0.01f;
    }
  if (contdata[0].button & D_CBUTTONS)
    cameraRotation.y += 0.05;
    if(cameraRotation.y >= (M_PI/2)-0.01f){
      cameraRotation.y = (M_PI/2)-0.01f;
    }
  if (contdata[0].button & L_CBUTTONS)
    cameraRotation.x -= 0.05;
  if (contdata[0].button & R_CBUTTONS)
    cameraRotation.x += 0.05;
  
  if(contdata[0].button & Z_TRIG ){
    cameraDistance = cameraDistance+2.0f;
  }
  if(contdata[0].button & R_TRIG ){
    cameraDistance = cameraDistance-2.0f;
  }
	//moving the camera
  cameraTarget.x = 0.0f;
  cameraTarget.y = 0;
  cameraTarget.z = -111.1f*cameraRotation.y + 173.0f;

  cameraPos.x = cameraTarget.x + ( cameraDistance * sin(cameraRotation.x) * cos(cameraRotation.y) );
  cameraPos.y = cameraTarget.y + ( cameraDistance * sin(cameraRotation.y)                         );
  cameraPos.z = cameraTarget.z + ( cameraDistance * cos(cameraRotation.x) * cos(cameraRotation.y) );
}
void movePebble(){
	//setting values for the controller sticks, divided by 80 to reduce the range to be from 0 to 1
  stickMoveX = contdata[0].stick_x/80.0f;
  stickMoveY = contdata[0].stick_y/80.0f;


  //moving the object
  if((stickMoveX != 0 || stickMoveY != 0 )&& returnedToCenter == TRUE){
    returnedToCenter = FALSE;
    if ( abs(stickMoveX*100) > abs(stickMoveY*100) ){
      //moving left right
      if ( stickMoveX < 0){
				moveCursor(-1,0);
      } else {
				moveCursor(1,0);
      }
    } else {
      //moving up down
      if ( stickMoveY < 0){
				moveCursor(0,1);
      } else {
				moveCursor(0,-1);
      }
    }
  }
  if(
    returnedToCenter == FALSE && 
    (stickMoveX == 0 && stickMoveY == 0 || (timer - stickResetFromTimer) % 15 == 0)
  ){
    returnedToCenter = TRUE;
  }
  //moving the cursor
  xLocation += (xGoal-xLocation)/moveSpeedSteps;
  zLocation += (zGoal-zLocation)/moveSpeedSteps;
  // adds a ease-out effect to the movement of the spinning pebble above the cursor
  // movespeed steps reaches 1 at which point the calculation above will divide by 1, and reach the x and z goal values
  if(moveSpeedSteps<1.1){
    moveSpeedSteps=1;
  }else{
    moveSpeedSteps *= 0.95;
  }
}
void moveCursor(int xDelta, int yDelta){
		//setting position of cursor & setting when the cursor can move again

		//stopping border breaks
		if(	0 <= (cursorPos.x + xDelta) && (cursorPos.x + xDelta) < squareCount &&
				0 <= (cursorPos.y + yDelta) && (cursorPos.y + yDelta) < squareCount){
			
			// if( isEmpty(cursorPos.x+xDelta,cursorPos.y+yDelta) == TRUE){
				cursorPos.x += xDelta;
				cursorPos.y += yDelta;
			// }		
		}
		//stopping collisions
		
    xGoal = cursorPos.x*sqaureSize - (boardSize/2 - sqaureSize/2);
    zGoal = cursorPos.y*sqaureSize - (boardSize/2 - sqaureSize/2);
    // lower this is the faster the pebble cursor will move
    moveSpeedSteps = 5;
    stickResetFromTimer = timer - 1; // -1 to stop it resetting the count immediately
  }
void removePebble(int x, int y){
  if(board[x][y].isEmpty == FALSE){
    board[x][y].isEmpty = TRUE;
    startPebbleRemove(board[x][y].content);
    if((*board[x][y].content).colour == WHITE){
      blackScore++;
    }else{
      whiteScore++;
    }
    board[x][y].content = NULL;
  }
}
void startPebbleRemove(Pebble *pebble){
  if((*pebble).colour == WHITE){
    // move captured white pebbles to the pile
    (*pebble).position.x = whitePotLocation.x;
    // makes the pebbles rise
    (*pebble).position.y = blackScore * 10;
    (*pebble).position.z = whitePotLocation.y;
  }else{
    // move captured black pebbles to the pile
    (*pebble).position.x = blackPotLocation.x;
    (*pebble).position.y = whiteScore * 10;
    (*pebble).position.z = blackPotLocation.y;
  }
}
void placeCounter() {
	if(board[cursorPos.x][cursorPos.y].isEmpty == TRUE){
		if( objectCount <= MAX_DROPS ){
			for (i = 0; i < MAX_DROPS; i++) {
				//check which number is next to store
				if( i == objectCount ){
					objectCount++;
					if( objectCount >= MAX_DROPS ){
						nuAuSndPlayerPlay(1);
					}else{
						nuAuSndPlayerPlay(4);
					}
					objectCount = objectCount % MAX_DROPS;
					// pebbles[i].x = ((int)(xLocation/150))*150;
					pebbles[i].position.x = xGoal;
					pebbles[i].position.y = 10.0f;
					// pebbles[i].z = ((int)(zLocation/150))*150;
					pebbles[i].position.z = zGoal;
					pebbles[i].rotation = 0.0f;
					if(turnCount%2==0){
						pebbles[i].colour = BLACK;
					}else{
						pebbles[i].colour = WHITE;
					}
          board[cursorPos.x][cursorPos.y].content = &pebbles[i];
          board[cursorPos.x][cursorPos.y].isEmpty = FALSE;
          turnCount++;
					break;
				}
			}
		}
	}else{
		nuAuSndPlayerPlay(1);
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
void checkForCaptures(){
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
          // capture pieces
          for(k=0;k<currentCaptureGroupIndex;++k){
            removePebble(pebblesInCurrentCaptureGroup[k].x,pebblesInCurrentCaptureGroup[k].y);
          } 
        }
      }
    }
  }
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
void checkGameOver(){
  if(turnCount >= maxTurns){
    gameOver = TRUE;
    nuAuSeqPlayerStop(0);
    nuAuSeqPlayerSetNo(0,0);
    nuAuSeqPlayerPlay(0);
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
  {
		CURRENT_GFX = 0;
    /* the pebble that hovers above the cursor */
			
		guPosition(&gfxTask->objectTransforms[CURRENT_GFX], 0.0f, timer*2%360, 0.0f, 0.6f, xLocation, 40 + 20*sin((float)timer/30.0f), zLocation);
		gSPMatrix(displayListPtr++,
			OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
			G_MTX_MODELVIEW | // operating on the modelview matrix stack...
			G_MTX_PUSH | // ...push another matrix onto the stack...
			G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
		);
		if(objectCount%2==0){
			drawModel(Wtx_pebble_black);
		}else{
			drawModel(Wtx_pebble_white);
		}
		// pop the matrix that we added back off the stack, to move the drawing position 
		// back to where it was before we rendered this object
		gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);

    
    for(i=0;i<(int)squareCount/3; ++i){   
      for(o=0;o<(int)squareCount/3; ++o){ 
        CURRENT_GFX++;
        guPosition(&gfxTask->objectTransforms[CURRENT_GFX],
        0.0f, //angle it to be flat
        0.0f, 0.0f, 
        (2.5f), //scale based on square size
        i * sqaureSize*3 - boardSize/2.0f + sqaureSize*3/2.0f,  //x
        0.0f,//y move down
        o * sqaureSize*3 - boardSize/2.0f + sqaureSize*3/2.0f); //z

        gSPMatrix(displayListPtr++,
          OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
          G_MTX_MODELVIEW | // operating on the modelview matrix stack...
          G_MTX_PUSH | // ...push another matrix onto the stack...
          G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
        );
        drawModel(Wtx_cross_by_three);
        gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
      }
    }

    for(i=0;i<objectCount; ++i){

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

    CURRENT_GFX++;
    guPosition(&gfxTask->objectTransforms[CURRENT_GFX],0.0f,0.0f,0.0f,2.5f, xGoal, 0.1f, zGoal);
    gSPMatrix(displayListPtr++,
        OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
        G_MTX_MODELVIEW | // operating on the modelview matrix stack...
        G_MTX_PUSH | // ...push another matrix onto the stack...
        G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
      );
    drawSquare();
    gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);

  }
  
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
  
  // nuDebConTextPos(0,0,0);
  // sprintf(conbuf,"the answer is: %f",atan2bodyf(M_PI,2));
  // nuDebConCPuts(0, conbuf);

  nuDebConTextPos(0,0,0);
  sprintf(conbuf,"White: %d",whiteScore);
  nuDebConCPuts(0, conbuf);
  nuDebConTextPos(0,0,1);
  sprintf(conbuf,"Black: %d",blackScore);
  nuDebConCPuts(0, conbuf);
  nuDebConTextPos(0,0,3);
  sprintf(conbuf,"Turns: %d",turnCount);
  nuDebConCPuts(0, conbuf);

  // nuDebConTextPos(0,0,8);
  // sprintf(conbuf,"White is 1 and black is 2: %d ",debugNumber);
  // nuDebConCPuts(0, conbuf);
  // nuDebConTextPos(0,0,9);
  // sprintf(conbuf,"True is 1 and false is 0: %d ",debugNumbertwo);
  // nuDebConCPuts(0, conbuf);
  // if(board[0][0].isEmpty == FALSE){
  //   nuDebConTextPos(0,0,1);
  //   sprintf(conbuf,"0,0: %d",(*board[0][0].content).colour);
  //   nuDebConCPuts(0, conbuf);
  // }
 
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
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER);

  // actually draw the triangles, using the specified vertices
  gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0);

  // Mark that we've finished sending commands for this particular primitive.
  // This is needed to prevent race conditions inside the rendering hardware in 
  // the case that subsequent commands change rendering settings.
  gDPPipeSync(displayListPtr++);
}

// this is an example of rendering a model defined as a set of static display lists
// void drawHead() {
//   gDPSetRenderMode(displayListPtr++,G_RM_ZB_OPA_SURF, G_RM_ZB_OPA_SURF2);
    
// 	gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
// 	gSPSetGeometryMode(displayListPtr++, G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH |
// 			   G_LIGHTING | G_CULL_BACK);

// 	gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
// 	gDPSetCombineMode(displayListPtr++,G_CC_DECALRGB, G_CC_DECALRGB);

// 	gSPDisplayList(displayListPtr++,kabe_mdl_model0);
//   gDPPipeSync(displayListPtr++);
// }
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

  // update the state of the world for the next frame
  updateGame01();
}

