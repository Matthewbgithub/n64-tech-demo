
#include <assert.h>
#include <nusys.h> 
/*includes */
#include "graphic.h"
#include "main.h"
#include "globals.h"
#include "stage00.h"
#include "title.h"

#include "skybox.h"
/*borrowed stuff */
// #include "texture.c"

#ifdef N_AUDIO
#include <nualsgi_n.h>
#else
#include <nualsgi.h>
#endif


int gfxtwo;
int menuTimer;
float skyscale;
float scaletwo;

// the 'setup' function
void initStage00() {  
  menuTimer = 0;
  skyscale = 1.0f;
  scaletwo = 1.0f;
}

// the 'update' function
void updateGame00() {
  
  // read controller input from controller 1 (index 0)
  nuContDataGetEx(contdata, 0);
  
    if (contdata[0].trigger & START_BUTTON){
      /* Remove the call-back function.  */
      nuGfxFuncRemove();
      //when a button is pressed, reset the scene
      stage = 1;
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
    
  menuTimer++;
  
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
 
    guOrtho(&gfxTask->projection,
	  -(float)SCREEN_WD/2.0F, (float)SCREEN_WD/2.0F,
	  -(float)SCREEN_HT/2.0F, (float)SCREEN_HT/2.0F,
	  -100.0F, 100.0F, 1.0F);

  gfxtwo = 0;

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
  
  
  guPosition(&gfxTask->objectTransforms[gfxtwo], 
              90.0f,0.0f,0.0f, // rotation
              1.0f,             // scale
              0.0f, 0.0f, -100.0f);// translation
  gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[gfxtwo])),
      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH
  );
  gfxtwo++;
  guScale(&gfxTask->objectTransforms[gfxtwo], 
              5.4f, 1.0f, 4.1f ); // xyz scale
  
  gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[gfxtwo])),
      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH
  );
  /* Read the texture for the movie by PI */
  // ReadMovie( 0, gfxTask );

  /* Draw the texture read by MovieBuf (MovieBuf[0]~MovieBuf[3] is unused data) */
  // DrawMovie( gfxTask, &(gfxTask->MovieBuf[4]));
  // shadetri();
  drawSkyBox();

  
  gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
  
  drawTitle(gfxTask);  
  
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

  nuDebConTextPos(0,0,4);
  sprintf(conbuf," press start ~");
  nuDebConCPuts(0, conbuf);
 
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

  // update the state of the world for the next frame
  updateGame00();
}

void drawSkyBox(){
  gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
  gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
  gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
  gSPDisplayList(displayListPtr++, Wtx_skybox);
  gDPPipeSync(displayListPtr++);
}
void drawTitle(GraphicsTask *gft){
  gfxtwo++;
  guPosition(&gft->objectTransforms[gfxtwo], 
              90.0f,0.0f,-90.0f, // rotation
              1.0f,             // scale
              sin(menuTimer)*5, cos(menuTimer)*5-30.0f, 0.0f);// translation
  
  gSPMatrix(displayListPtr++,
      OS_K0_TO_PHYSICAL(&(gft->objectTransforms[gfxtwo])),
      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH
  );

  gDPSetCycleType(displayListPtr++, G_CYC_1CYCLE);
  gDPSetRenderMode(displayListPtr++, G_RM_AA_ZB_OPA_SURF, G_RM_AA_ZB_OPA_SURF2);
  gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
  gSPSetGeometryMode(displayListPtr++, G_SHADE | G_SHADING_SMOOTH | G_ZBUFFER | G_LIGHTING);
  gSPDisplayList(displayListPtr++, Wtx_title_Text);
  gDPPipeSync(displayListPtr++);
  gSPDisplayList(displayListPtr++, Wtx_title_Text_003_Text_002);
  gDPPipeSync(displayListPtr++);
  gSPDisplayList(displayListPtr++, Wtx_title_Text_004_Text_003);
  gDPPipeSync(displayListPtr++);

  gSPPopMatrix(displayListPtr++, G_MTX_MODELVIEW);
}
