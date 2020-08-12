
#include <assert.h>
#include <nusys.h> 

#include "graphic.h"
#include "main.h"
#include "stage00.h"
// #include "n64logo.h"
#include "speaker.h"


#define MAX_DROPS 11

#ifdef N_AUDIO
#include <nualsgi_n.h>
#else
#include <nualsgi.h>
#endif

// the positions of the squares we're gonna draw
// 


// this is a boolean but the older version of C used by the N64 compiler
// (roughly C89), doesn't have a bool type, so we just use integers

// int showN64Logo;
int zoomOut;
float height;
float stickMoveX;
float stickMoveY;
float stickXWithDecel;
float stickYWithDecel;
float xLocation;
float zLocation;
float xVelocity;
float zVelocity;
float maxXVelocity;
float maxZVelocity;
float accelerationSpeed;
float decelerationSpeed;

// float theta;

// float lastx;
// float lasty;
// float lastz;
// float currentx;
// float currenty;
// float currentz;

// double num;
// float result;
// double result2;
float objectAngle;
float forwardVelocity;

Vec3d cameraPos = {0.0f, 0.0f, 0.0f};
Vec3d cameraTarget = {0.0f, 0.0f, 0.0f};
Vec3d cameraUp = {0.0f, 1.0f, 0.0f};

float cameraDistance;
Vec3d cameraRotation;


typedef struct {
  Vec3d     position;
  float     rotation;
  float     velocity;
} Disc;

Disc discs[MAX_DROPS];
int objectCount;

int CURRENT_GFX;
int i;
int timer;


// the 'setup' function
void initStage00() {  
  // the advantage of initializing these values here, rather than statically, is
  // that if you switch stages/levels, and later return to this stage, you can
  // call this function to reset these values.
  // showN64Logo = FALSE;

  //moving values
  zoomOut = FALSE;
  height = 50;
  stickMoveX = 0;
  stickMoveY = 0;
  stickXWithDecel = 0;
  stickYWithDecel = 0;
  xLocation = 0;
  zLocation = 0;
  xVelocity = 0;
  zVelocity = 0;
  // maxXVelocity = 20.0f;
  // maxZVelocity = 20.0f;
  // accelerationSpeed = 0.6f; 
  // decelerationSpeed = 0.8f; //multiplied by every frame

  // theta = 0.0f;
  // currentx = xLocation;
  // currenty = zLocation;
  // currentz = xMod;
  // lastx = currentx;
  // lasty = currenty;
  // lastz = currentz;

  objectAngle = 0.0f;
  forwardVelocity = 0.0f;

  cameraPos.x = xLocation;
  cameraPos.y = height;
  cameraPos.z = zLocation;
  cameraDistance = 600.0f;
  cameraRotation.x = 0.785f;
  cameraRotation.y = 0.349f;
  cameraRotation.z = 0.0f;
  timer = 0;

  for (i=0; i<MAX_DROPS;i++){
    discs[i].position.x = NULL;
    discs[i].position.y = NULL;
    discs[i].position.z = NULL;
    discs[i].rotation = 0.0f;
    discs[i].velocity = 0.0f;
  }
  objectCount = 0;

}

// the 'update' function
void updateGame00() {
  timer++;
  // read controller input from controller 1 (index 0)
  nuContDataGetEx(contdata, 0);
  // We check if the 'A' Button was pressed using a bitwise AND with
  // contdata[0].trigger and the A_BUTTON constant.
  // The contdata[0].trigger property is set only for the frame that the button is
  // initially pressed. The contdata[0].button property is similar, but stays on
  // for the duration of the button press.
  if (contdata[0].trigger & START_BUTTON){
    //when a button is pressed, reset the scene
    initStage00();
  } else {
     if (contdata[0].trigger & A_BUTTON){
      //when a button is pressed, reset the scene
      placeObj();
    }
    if (contdata[0].button & B_BUTTON){
      // when B button is held, change squares into n64 logos
      // showN64Logo = TRUE;
      zoomOut = TRUE;
    } else {
      zoomOut = FALSE;
      // showN64Logo = FALSE;
    }
  }

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
  
  //setting values for the controller sticks, divided by 80 to reduce the range to be from 0 to 1
  stickMoveX = contdata[0].stick_x/80.0f;
  stickMoveY = contdata[0].stick_y/80.0f;


  //moving the object
  forwardVelocity = stickMoveY*5;
  objectAngle += stickMoveX/15;

  xLocation = xLocation + forwardVelocity*cos(objectAngle);
  zLocation = zLocation + forwardVelocity*sin(objectAngle);

  //moving the discs
  for(i=0; i<objectCount;i++){
    discs[i].position.x = discs[i].position.x + discs[i].velocity*cos(discs[i].rotation);
    discs[i].position.z = discs[i].position.z + discs[i].velocity*sin(discs[i].rotation);
  }
  // if(   (xVelocity > ( stickMoveX *  maxXVelocity ) && stickMoveX >= 0.0f )
  //       ||
  //       (xVelocity < ( stickMoveX *  maxXVelocity ) && stickMoveX <= 0.0f) ){
  //   //over the max velocity so decrease
  //   xVelocity = xVelocity * decelerationSpeed;
  //   // if(xVelocity <= 0.0f){
  //   //   xVelocity = 0.0f;
  //   // }
  // }else{
  //   //increase it
  //   xVelocity = xVelocity + (stickMoveX * accelerationSpeed);
  // }

  // if( (zVelocity > ( stickMoveY * maxZVelocity ) && stickMoveY >= 0.0f)
  //     ||
  //     (zVelocity < ( stickMoveY *  maxZVelocity ) && stickMoveY <= 0.0f) ){
  //   //lower it
  //   zVelocity = zVelocity * decelerationSpeed;
  //   // if(yVelocity <= 0.0f){
  //   //   yVelocity = 0.0f;
  //   // }
  // }else{
  //   //increase it
  //   zVelocity = zVelocity + (stickMoveY * accelerationSpeed);
  // }
  /*object moving calculation
                20      +    0-1    *      1   */
  // xLocation = xLocation + xVelocity;// * accelerationSpeed;
  // zLocation = zLocation - zVelocity;// * accelerationSpeed;


  //moving the camera
  cameraPos.x = xLocation + ( cameraDistance * sin(cameraRotation.x) * cos(cameraRotation.y) );
  cameraPos.y = height + ( cameraDistance * sin(cameraRotation.y)  );
  cameraPos.z = zLocation + ( cameraDistance * cos(cameraRotation.x) * cos(cameraRotation.y) );

  cameraTarget.x = xLocation;
  cameraTarget.y = height;
  cameraTarget.z = zLocation;

  soundCheck();
}
// the 'draw' function
void makeDL00() {
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

  {
    
    CURRENT_GFX = 0;
    // Vec3d* square;
    // square = &squares[0];
    guPosition(&gfxTask->objectTransforms[CURRENT_GFX], 0.0f, -(objectAngle*(180/M_PI)), 0.0f, 1.0f, xLocation, height, zLocation);

    gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
      G_MTX_MODELVIEW | // operating on the modelview matrix stack...
      G_MTX_PUSH | // ...push another matrix onto the stack...
      G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
    );

    //moves each N, movement stays
    // guTranslate(&gfxTask->objectTransforms,-xLocation, zLocation,xMod);
    // guRotate(&gfxTask->objectTransforms, 45.0f,0.0f,0.0f,0.0f);
    // gSPMatrix(displayListPtr++,
    //   OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms)),
    //   G_MTX_MODELVIEW | // operating on the modelview matrix stack...
    //   G_MTX_PUSH | // ...push another matrix onto the stack...
    //   G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
    // );
    
    drawN64Logo();

    // pop the matrix that we added back off the stack, to move the drawing position 
    // back to where it was before we rendered this object
    gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
    CURRENT_GFX++;
    // guRotate(&gfxTask->objectTransforms[1], 90.0f, 1.0f, 0.0f, 0.0f);
    guPosition(&gfxTask->objectTransforms[CURRENT_GFX],
    0.0f, //angle it to be flat
    0.0f, 0.0f, 
    10.0f, //scale big
    0.0f,  //x
    0.0f,//y move down
    0.0f); //z

    // guTranslate(&gfxTask->objectTransforms[1], 0.0f, -200.0f, 0.0f);
    gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
      G_MTX_MODELVIEW | // operating on the modelview matrix stack...
      G_MTX_PUSH | // ...push another matrix onto the stack...
      G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
    );
    drawSquare();
    gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);

    for(i=0;i<objectCount; ++i){

      CURRENT_GFX++;

      guPosition(&gfxTask->objectTransforms[CURRENT_GFX], 0.0f, timer + -(discs[i].rotation*(180/M_PI)) ,0.0f,1.0f, discs[i].position.x, 50 + 30*sin(discs[i].position.y +(float)timer/30.0f), discs[i].position.z);

      gSPMatrix(displayListPtr++,
        OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[CURRENT_GFX])),
        G_MTX_MODELVIEW | // operating on the modelview matrix stack...
        G_MTX_PUSH | // ...push another matrix onto the stack...
        G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
      );
      drawSquare();
      gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
    }

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
  nuDebConTextPos(0,0,0);
  sprintf(conbuf,"count: %d, I: %d",objectCount, i);
  nuDebConCPuts(0, conbuf);

  for (i=1;i<MAX_DROPS;i++){
    nuDebConTextPos(0,0,i);
    
    if(objectCount == i){
      sprintf(conbuf,"%d:\t%5.1f <\n",i,discs[i].position.x);
    } else {  
      sprintf(conbuf,"%d:\t%5.1f     \n",i,discs[i].position.x);
    }
    nuDebConCPuts(0, conbuf);
  }
  /*
  ---------------------------end debug on screen-------------------------------------
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
  { -64,   0,  64,    0, 0, 0, 0x00, 0xff, 0x00, 0xff  },
  {  64,   0,  64,    0, 0, 0, 0x00, 0x00, 0x00, 0xff  },
  {  64,   0, -64,    0, 0, 0, 0x00, 0x00, 0xff, 0xff  },
  { -64,   0, -64,    0, 0, 0, 0xff, 0x00, 0x00, 0xff  },
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
void placeObj() {
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
        // discs[i].x = ((int)(xLocation/150))*150;
        discs[i].position.x = xLocation;
        discs[i].position.y = timer;
        // discs[i].z = ((int)(zLocation/150))*150;
        discs[i].position.z = zLocation;
        discs[i].rotation = objectAngle;
        discs[i].velocity = 20.0f;
        break;
      }
    }
  }
}
// this is an example of rendering a model defined as a set of static display lists
void drawN64Logo() {
  gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
  gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
  gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER);
  
  // The gSPDisplayList command causes the RCP to render a static display list,
  // then return to this display list afterwards. These 4 display lists are
  // defined in n64logo.h, and were generated from a 3D model using a conversion
  // script.
  // gSPDisplayList(displayListPtr++, N64Yellow_PolyList);
  // gSPDisplayList(displayListPtr++, N64Red_PolyList);
  // gSPDisplayList(displayListPtr++, N64Blue_PolyList);
  // gSPDisplayList(displayListPtr++, N64Green_PolyList);

  gSPDisplayList(displayListPtr++, Wtx_speaker);
  gDPPipeSync(displayListPtr++);
}

/* Provide playback and control of audio by the button of the controller */
void soundCheck(void)
{
  static int snd_no = 0;
  static int seq_no = 0;

  /* Change music of sequence playback depending on the top and bottom of 
  the cross key */
  if((contdata[0].trigger & U_JPAD) || (contdata[0].trigger & D_JPAD))
    {
      if(contdata[0].trigger & U_JPAD)
	{
	  seq_no--;
	  if(seq_no < 0) seq_no = 2;
	}
      else
	{
	  seq_no++;
	  if(seq_no > 2) seq_no = 0;
	}	  

      nuAuSeqPlayerStop(0);
      nuAuSeqPlayerSetNo(0,seq_no);
      nuAuSeqPlayerPlay(0);
    }

  /* Possible to play audio in order by right and left of the cross key */
  if((contdata[0].trigger & L_JPAD) || (contdata[0].trigger & R_JPAD))
    {
      if(contdata[0].trigger & L_JPAD)
	{
	  snd_no--;
	  if(snd_no < 0) snd_no = 10;
	}
      else
	{
	  snd_no++;
	  if(snd_no > 10) snd_no = 0;
	}	  

      /* Eleven sounds (sound data items) are provided.  Of these, the first 10 are sampled at 44 KHz and the 11th at 24 KHz. */
      nuAuSndPlayerPlay(snd_no); 
      if(snd_no < 10)
	nuAuSndPlayerSetPitch(44100.0/32000);
      else
	nuAuSndPlayerSetPitch(24000.0/32000);
    }

  /* Change tempo of sequence playback by the L and R buttons */
  if((contdata[0].trigger & L_TRIG) || (contdata[0].trigger & R_TRIG))
    {
      s32 tmp;
      tmp = nuAuSeqPlayerGetTempo(0);

      if(contdata[0].trigger & L_TRIG)
	{
	  tmp /= 10;
	  tmp *= 8;
	}
      else
	{
	  tmp /= 10;
	  tmp *= 12;
	}
      nuAuSeqPlayerSetTempo(0,tmp);
    }

  /* Fade out sound by pushing the Z button */
  if(contdata[0].trigger & Z_TRIG)
    {
      nuAuSeqPlayerFadeOut(0,200);
    }
}

// the nusystem callback for the stage, called once per frame
void stage00(int pendingGfx)
{
  // produce a new displaylist (unless we're running behind, meaning we already
  // have the maximum queued up)
  if(pendingGfx < 1)
    makeDL00();

  // update the state of the world for the next frame
  updateGame00();
}

