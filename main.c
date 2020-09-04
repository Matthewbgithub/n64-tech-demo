
#include <nusys.h>
#include "main.h"
#include "segment.h"
#include "stage01.h"

#ifdef N_AUDIO
#include <nualsgi_n.h>
#else
#include <nualsgi.h>
#endif

//declaring prototypes
void setAudioData(void);

NUContData  contdata[1]; // storage for controller 1 inputs
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

  // initialize the level
  initStage01();

  // set the update+draw callback to be called every frame
  nuGfxFuncSet((NUGfxFunc)stage01);

  // enable video output
  nuGfxDisplayOn();

  // send this thread into an infinite loop
  while(1)
    ;
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