
#include <assert.h>
#include <nusys.h> 
/*includes */
#include "graphic.h"
#include "main.h"
#include "globals.h"
#include "stage00.h"
#include "title.h"

/*borrowed stuff */
#include "font.h"
#include "movie.c"
#include "n64logo.h"

#ifdef N_AUDIO
#include <nualsgi_n.h>
#else
#include <nualsgi.h>
#endif

extern NUContData contdata[];

int gfxtwo;
int menuTimer;
float skyscale;
float scaletwo;
int debugMode;
float debugY;
float debugX;
int transition;
int transitionStartTime;
float transTemp;

float gAngle;
float oAngle;
float exclAngle;
float screenScale;

// the 'setup' function
void initStage00() {  
  menuTimer = 0;
  transitionStartTime = 0;
  skyscale = 1.0f;
  scaletwo = 1.0f;
  debugMode = FALSE;
  debugX = 0.0f;
  debugY = 0.0f;
  transition = FALSE;

  gAngle = 0.0f;
  oAngle = 0.0f;
  exclAngle = 0.0f;
  screenScale = 1.0f; 
  nuAuSeqPlayerSetNo(0,2);
  nuAuSeqPlayerPlay(0);
}

// the 'update' function
void updateGame00() {
  
  // read controller input from controller 1 (index 0)
  // nuContDataGetEx(contdata, 0);
  
    

    if( debugMode == TRUE ){
      //debug controls
      if (contdata[0].button & U_CBUTTONS)
        debugY += 1.0f;
      else if (contdata[0].button & D_CBUTTONS)
        debugY -= 1.0f;
      if (contdata[0].button & L_CBUTTONS)
        debugX -= 1.0f;
      else if (contdata[0].button & R_CBUTTONS)
        debugX += 1.0f;
    }else{
      //standard actions
      if(transition == FALSE){
        if (contdata[0].trigger & START_BUTTON){
          nuAuSndPlayerPlay(1);
          transitionStartTime = menuTimer;
          transition = TRUE;
        }
        if (contdata[0].button & A_BUTTON){
          skyscale+=0.1f;
        }else if(contdata[0].button & B_BUTTON){
          skyscale-=0.1f;
        }
        if (contdata[0].button & Z_TRIG){
          scaletwo+=0.1f;
        }else if(contdata[0].button & R_TRIG){
          scaletwo-=0.1f;
        }
      }
    }
    
    //switching debug mode
    if(contdata[0].trigger & L_TRIG){
      nuDebConClear(NU_DEB_CON_WINDOW0);
      //switches from 0 to 1 and vise versa
      debugMode ^= 1;
    }

    if( transition == TRUE ){
      transTemp = ((menuTimer - transitionStartTime)/10.0f);
      screenScale = 1.0f + transTemp * transTemp * transTemp;
      if ( screenScale >= 60.0f ){
        nuDebConClear(NU_DEB_CON_WINDOW0);
        nextStage();
      }
    }

    gAngle = sin(menuTimer/4.6f)*5.0f;
    oAngle = sin(menuTimer/6.0f)*3.0f;
    exclAngle = sin(menuTimer/5.0f)*-5.0f;

    menuTimer++;
    // screenScale = sin(menuTimer/10.0f) + 2.0f;
  
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
 
    // guOrtho(&gfxTask->projection,
	  // -(float)SCREEN_WD/2.0F, (float)SCREEN_WD/2.0F,
	  // -(float)SCREEN_HT/2.0F, (float)SCREEN_HT/2.0F,
	  // -100.0F, 100.0F, 1.0F);
    guOrtho(&gfxTask->projection,
	  -(float)SCREEN_WD/2.0f/(screenScale), (float)SCREEN_WD/2.0f/(screenScale),
	  -(float)SCREEN_HT/2.0f/(screenScale), (float)SCREEN_HT/2.0f/(screenScale),
	  -100.0F, 100.0F, 1.0F);

  gfxtwo = -1;

  gSPMatrix(
    displayListPtr++,
    OS_K0_TO_PHYSICAL(&(gfxTask->projection)),
    G_MTX_PROJECTION | // using the projection matrix stack...
    G_MTX_LOAD | // don't multiply matrix by previously-top matrix in stack
    G_MTX_NOPUSH // don't push another matrix onto the stack before operation
  );

  // gSPMatrix(displayListPtr++,
  //   OS_K0_TO_PHYSICAL(&(gfxTask->modelview)),
  //   // similarly this combination means "replace the modelview matrix with this new matrix"
  //   G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH 
  // );
  
  
  
  /* Read the texture for the movie by PI */
  ReadMovie( 0, gfxTask );

  /* Draw the texture read by MovieBuf (MovieBuf[0]~MovieBuf[3] is unused data) */
  DrawMovie( gfxTask, &(gfxTask->MovieBuf[4]));
  // shadetri();
  // drawSkyBox(gfxTask);

  
  gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
  

  drawTitle(gfxTask);  
  
  if(debugMode == TRUE){
    drawDebug(gfxTask);
  }
  sprintf(outstring,"Press START");
  if(transition == TRUE){
    Draw8Font(114 ,170, (int)(menuTimer/5)%2, 0);    
  }else{
    Draw8Font(114 ,170, TEX_COL_WHITE, 0);
  }
 //----------------------------------------------------
  gDPFullSync(displayListPtr++);
  gSPEndDisplayList(displayListPtr++);


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
  if ( debugMode == TRUE ){
    nuDebConTextPos(0,0,0);
    sprintf(conbuf,"x: %f, y:", screenScale);
    nuDebConCPuts(0, conbuf);
  }
  
  

  /*
  ---------------------------end debug on screen---------------------------------------
  */
  /*  character written to frame buffer */
  nuDebConDisp(NU_SC_SWAPBUFFER);
}

// the nusystem callback for the stage, called once per frame
void stage00(int pendingGfx)
{
  // produce a new displaylist (unless we're running behind, meaning we already
  // have the maximum queued up)
  if(pendingGfx < 1)
    makeDL00();

  nuContDataGetExAll(contdata);

  // update the state of the world for the next frame
  updateGame00();
}
void nextStage(){
  /* Remove the call-back function.  */
  nuGfxFuncRemove();
  //when a button is pressed, reset the scene
  nuAuSeqPlayerStop(0);
  stage = 1;
}
// void drawSkyBox(GraphicsTask *gft){
//   gfxtwo++;
//   guPosition(&gft->objectTransforms[gfxtwo], 
//               90.0f,0.0f,0.0f, // rotation
//               1.0f,             // scale
//               0.0f, 0.0f, -100.0f);// translation
//   gSPMatrix(displayListPtr++,
//       OS_K0_TO_PHYSICAL(&(gft->objectTransforms[gfxtwo])),
//       G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH
//   );
//   gfxtwo++;
//   guScale(&gft->objectTransforms[gfxtwo], 
//               5.4f, 1.0f, 4.1f ); // xyz scale
  
//   gSPMatrix(displayListPtr++,
//       OS_K0_TO_PHYSICAL(&(gft->objectTransforms[gfxtwo])),
//       G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH
//   );
//   gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
//   gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
//   gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
//   gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
//   gSPDisplayList(displayListPtr++, Wtx_skybox);
//   gDPPipeSync(displayListPtr++);
// }
void drawTitle(GraphicsTask *gft){
  gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
  gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
  gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
  
  gfxtwo++;
  guPosition(&gft->objectTransforms[gfxtwo], 
              90.0f,0.0f,-90.0f+oAngle, // rotation
              1.0f,             // scale
              0.0f, -30.0f, 0.0f);// translation
  
  gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gft->objectTransforms[gfxtwo])),
      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH
  );
  gSPDisplayList(displayListPtr++, Wtx_title_Text);
  gDPPipeSync(displayListPtr++);
  gfxtwo++;
  guPosition(&gft->objectTransforms[gfxtwo], 
              90.0f,0.0f,-90.0f + gAngle, // rotation
              1.0f,             // scale
              0.0f, -30.0f, 0.0f);// translation
  
  gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gft->objectTransforms[gfxtwo])),
      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH
  );
  gSPDisplayList(displayListPtr++, Wtx_title_Text_003_Text_002);
  gDPPipeSync(displayListPtr++);
  gfxtwo++;
  guPosition(&gft->objectTransforms[gfxtwo], 
              90.0f,0.0f,-90.0f + exclAngle, // rotation
              1.0f,             // scale
              0.0f, -30.0f, 0.0f);// translation
  
  gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gft->objectTransforms[gfxtwo])),
      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH
  );
  gSPDisplayList(displayListPtr++, Wtx_title_Text_004_Text_003);
  gDPPipeSync(displayListPtr++);

  gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
}
void drawDebug(GraphicsTask *gft){
  gfxtwo++;
  guPosition(&gft->objectTransforms[gfxtwo], 
              0.0f,0.0f,0.0f, // rotation
              0.1f,             // scale
              debugX, debugY, 0.0f);// translation
  gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gft->objectTransforms[gfxtwo])),
      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH
  );
  gSPDisplayList(displayListPtr++, N64Red_PolyList);
  gSPDisplayList(displayListPtr++, N64Green_PolyList);
  gSPDisplayList(displayListPtr++, N64Blue_PolyList);
  gSPDisplayList(displayListPtr++, N64Yellow_PolyList);
  gDPPipeSync(displayListPtr++);
}
