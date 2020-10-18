
#include <nusys.h>
#include "main.h"
#include "segment.h"
#include "stage01.h"
#include "stage00.h"

#ifdef N_AUDIO
#include <nualsgi_n.h>
#else
#include <nualsgi.h>
#endif

//declaring prototypes
void setAudioData(void);

/* The stage number  */
volatile int stage;

/* Declaration of the prototype  */
void stage00(int);
void stage01(int);

/* Declaration of the external function  */
void initStage00(void);
void makeDL00(void);
void updateGame00(void);

void initStage01(void);
void makeDL01(void);
void updateGame01(void);

NUContData contdata[NU_CONT_MAXCONTROLLERS];
// NUContData  contdata[1]; // storage for controller 1 inputs
u8 contPattern;		     /* The pattern connected to the controller  */


void mainproc(void)
{
  // initialize the graphics system
  nuGfxInit();

  // initialize the controller manager
  nuContInit();

  /* The initialization of audio  */
  nuAuInit();
  /* Register audio data on ROM  */
  setAudioData();

  /* Set the stage number to 0 */
  stage = 0;

  while(1)
    {
      switch(stage)
	{
	  /* 
	     Register the corresponding call-back function according to the 
	     stage number.  
		 The call-back function sets the value to the stage when another 
	     call-back function needs the register change.  
	     */
	case 0:
	  /* Set the stage value to -1 first, to wait for that the call-back 
	     function sets the value */
	  stage = -1;
	  /* The initialization of stage 0  */
	  initStage00();
	  /* The call-back register  */
	  nuGfxFuncSet((NUGfxFunc)stage00);
	  /* Start to display  */
	  nuGfxDisplayOn();
	  break;
	case 1:
	  stage = -1;
	  initStage01();
	  nuGfxFuncSet((NUGfxFunc)stage01);
	  nuGfxDisplayOn();
	  break;
	default:
	  break;
	}
      
      /* Wait for that the call-back function switches values (switch the scene) */
      while(stage == -1)
	;
      /* Clear the display  */
      nuGfxDisplayOff();
    }
}

/* Set audio data  */
void setAudioData(void)
{
  /* Register the bank to the sequence player  */
  nuAuSeqPlayerBankSet(_midibankSegmentRomStart,
		       _midibankSegmentRomEnd - _midibankSegmentRomStart,
		       _miditableSegmentRomStart);
  /* Register MIDI sequence data to the sequence player */
  nuAuSeqPlayerSeqSet(_seqSegmentRomStart);
  /* Register the bank to the sound player  */
  nuAuSndPlayerBankSet(_sfxbankSegmentRomStart,
		       _sfxbankSegmentRomEnd - _sfxbankSegmentRomStart,
		       _sfxtableSegmentRomStart);
}