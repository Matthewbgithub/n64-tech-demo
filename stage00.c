
#include <assert.h>
#include <nusys.h> 
/*includes */
#include "graphic.h"
#include "main.h"
#include "stage00.h"

#ifdef N_AUDIO
#include <nualsgi_n.h>
#else
#include <nualsgi.h>
#endif

// the 'setup' function
void initStage00() {  
  
}

// the 'update' function
void updateGame00() {
  
  // read controller input from controller 1 (index 0)
  nuContDataGetEx(contdata, 0);
  
    if (contdata[0].trigger & START_BUTTON){
      //when a button is pressed, reset the scene
      stage = 1;
    }
  
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

  gSPPerspNormalize(displayListPtr++, perspNorm);

  gSPMatrix(
    displayListPtr++,
    OS_K0_TO_PHYSICAL(&(gfxTask->projection)),
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
        guPosition(&gfxTask->objectTransforms[0], 0.0f, 0.0f, 0.0f, 0.6f, 0.0f, 0.0f, 0.0f);
        gSPMatrix(displayListPtr++,
            OS_K0_TO_PHYSICAL(&(gfxTask->objectTransforms[0])),
            G_MTX_MODELVIEW | // operating on the modelview matrix stack...
            G_MTX_PUSH | // ...push another matrix onto the stack...
            G_MTX_MUL // ...which is multipled by previously-top matrix (eg. a relative transformation)
        );
        drawSquareMENU();
        // pop the matrix that we added back off the stack, to move the drawing position 
        // back to where it was before we rendered this object
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

  nuDebConTextPos(0,0,0);
  sprintf(conbuf,"STAGE 0");
  nuDebConCPuts(0, conbuf);

  nuDebConTextPos(0,0,4);
  sprintf(conbuf,"---PRESS START TO START THE GAME---");
  nuDebConCPuts(0, conbuf);
 
  /*
  ---------------------------end debug on screen---------------------------------------
  */
  /*  character written to frame buffer */
  nuDebConDisp(NU_SC_SWAPBUFFER);
}
Vtx squareVertsMENU[] __attribute__((aligned (16))) = {
  //  x,   y,  z, flag, S, T,    r,    g,    b,    a
  { -10,   0,  10,    0, 0, 0, 0x00, 0xff, 0x00, 0xff  },
  {  10,   0,  10,    0, 0, 0, 0x00, 0x00, 0x00, 0xff  },
  {  10,   0, -10,    0, 0, 0, 0x00, 0x00, 0xff, 0xff  },
  { -10,   0, -10,    0, 0, 0, 0xff, 0x00, 0x00, 0xff  },
};

void drawSquareMENU() {
  
  // load vertex data for the triangles
  gSPVertex(displayListPtr++, &(squareVertsMENU[0]), 4, 0);


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

