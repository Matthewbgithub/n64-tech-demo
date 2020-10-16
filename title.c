#include <nusys.h>
//change these later
#define MOV_WRAP	6
#define MOVIE_SIZE	170.0	/*The length of one side of the map object */

/* spec */
extern u8 _moviedataSegmentRomStart[];
extern u8 _moviedataSegmentRomEnd[];

/*----------------------------------------------*/
/*  Read the movie data 				*/
/*  IN: MovieNum	The scene number of the movie */
/*      dp		The dynamic pointer 		*/
/*  RET:	None						*/
/*----------------------------------------------*/
void
ReadMovie( int MovieNum, GraphicsTask* dp )
{
    u32 r2rlen;
    u32 onecutlen;

    /* Compute the stored segment heads of the movie data */
    r2rlen = _moviedataSegmentRomEnd - _moviedataSegmentRomStart;

    /* Compute the length of each scene */
    onecutlen = r2rlen / 1;

    /* Copy data from ROM to RDRAM, practically */
    nuPiReadRom( (u32)(_moviedataSegmentRomStart + onecutlen * MovieNum),
		(void *)&(dp->MovieBuf[0]),
		onecutlen
		);
}

Vtx movie_vtx[] = {
    {          0.0, MOVIE_SIZE, 0.0,  0, 31 << MOV_WRAP, 00 << MOV_WRAP, 0, 0, 127, 0xff},
    {  -MOVIE_SIZE, MOVIE_SIZE, 0.0,  0, 00 << MOV_WRAP, 00 << MOV_WRAP, 0, 0, 127, 0xff},
    {  -MOVIE_SIZE,        0.0, 0.0,  0, 00 << MOV_WRAP, 31 << MOV_WRAP, 0, 0, 127, 0xff},
    {         0.0,         0.0, 0.0,  0, 31 << MOV_WRAP, 31 << MOV_WRAP, 0, 0, 127, 0xff},

    {   MOVIE_SIZE, MOVIE_SIZE, 0.0,  0, 63 << MOV_WRAP, 00 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0, MOVIE_SIZE, 0.0,  0, 32 << MOV_WRAP, 00 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0,        0.0, 0.0,  0, 32 << MOV_WRAP, 31 << MOV_WRAP, 0, 0, 127, 0xff},
    {   MOVIE_SIZE,        0.0, 0.0,  0, 63 << MOV_WRAP, 31 << MOV_WRAP, 0, 0, 127, 0xff},

    {          0.0,         0.0, 0.0,  0, 31 << MOV_WRAP, 32 << MOV_WRAP, 0, 0, 127, 0xff},
    {  -MOVIE_SIZE,         0.0, 0.0,  0, 00 << MOV_WRAP, 32 << MOV_WRAP, 0, 0, 127, 0xff},
    {  -MOVIE_SIZE, -MOVIE_SIZE, 0.0,  0, 00 << MOV_WRAP, 63 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0, -MOVIE_SIZE, 0.0,  0, 31 << MOV_WRAP, 63 << MOV_WRAP, 0, 0, 127, 0xff},

    {   MOVIE_SIZE,         0.0, 0.0,  0, 63 << MOV_WRAP, 32 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0,         0.0, 0.0,  0, 32 << MOV_WRAP, 32 << MOV_WRAP, 0, 0, 127, 0xff},
    {          0.0, -MOVIE_SIZE, 0.0,  0, 32 << MOV_WRAP, 63 << MOV_WRAP, 0, 0, 127, 0xff},
    {   MOVIE_SIZE, -MOVIE_SIZE, 0.0,  0, 63 << MOV_WRAP, 63 << MOV_WRAP, 0, 0, 127, 0xff},
};


/*--------------------------------------------------------------*/
/*  Draw the movie part of the center of the map 			*/
/*  IN:	dp The dynamic pointer 	*bmp The texture header address 	*/
/*  RET:None 							*/
/*--------------------------------------------------------------*/
void
DrawMovie( GraphicsTask* dp, unsigned short *bmp )
{
    /* The calculation of the model coordinate system  */
    guTranslate( &dp->title_trans, 0.0, 0.0, 0.0 );
    guRotateRPY( &dp->title_rotate, 0.0, 0.0, 90.0 );
    guScale( &dp->title_scale, 1.0, 1.0, 1.0 );
    
    /* Setting the model-matrix  */
    gSPMatrix(displayListPtr++, OS_K0_TO_PHYSICAL(&(dp->title_trans)),
	      G_MTX_MODELVIEW | G_MTX_LOAD | G_MTX_NOPUSH);
    
    gSPMatrix(displayListPtr++, OS_K0_TO_PHYSICAL(&(dp->title_rotate)),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);
    
    gSPMatrix(displayListPtr++, OS_K0_TO_PHYSICAL(&(dp->title_scale)),
	      G_MTX_MODELVIEW | G_MTX_MUL | G_MTX_NOPUSH);

    gDPSetCycleType(displayListPtr++,G_CYC_1CYCLE);

    gSPClearGeometryMode(displayListPtr++,0xFFFFFFFF);
    gSPSetGeometryMode(displayListPtr++, G_LIGHTING | G_ZBUFFER | G_SHADE | G_SHADING_SMOOTH |
		       G_CULL_BACK);


    /*Paste to the base by the decal mode */
    gDPSetRenderMode(displayListPtr++,G_RM_AA_ZB_XLU_DECAL, G_RM_AA_ZB_XLU_DECAL2);
    gDPSetColorDither(displayListPtr++,G_CD_DISABLE);
    gDPSetCombineMode (displayListPtr++,G_CC_DECALRGBA, G_CC_DECALRGBA);
    gDPSetTexturePersp (displayListPtr++,G_TP_PERSP);
    gDPSetTextureLOD (displayListPtr++,G_TL_TILE);

    // this line changes the blending mode
    gDPSetTextureFilter (displayListPtr++,G_TF_BILERP);
    gDPSetTextureConvert(displayListPtr++,G_TC_FILT);
    gDPSetTextureLUT (displayListPtr++,G_TT_NONE);
    gSPTexture(displayListPtr++,0x8000, 0x8000, 0, G_TX_RENDERTILE, G_ON);

    
    /* Load vertex data  */
    gSPVertex(displayListPtr++,&(movie_vtx[0]), 16, 0);

    /* Read the 64-dot X 64-dot texture by dividing to four  */
    /* In other words, read four pieces of 32-dot X 32-dot textures  */
    /* Read texture data (the upper-left part) */
    gDPLoadTextureTile(displayListPtr++,
		       bmp,
		       G_IM_FMT_RGBA,
		       G_IM_SIZ_16b,
		       64, 64,
		       0, 0, 31, 31,
		       0,
		       G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
		       0, 0,
		       G_TX_NOLOD, G_TX_NOLOD);
    gSP2Triangles(displayListPtr++,0,1,2,0,0,2,3,0);

    /* Read texture data (the upper-right part) */
    gDPLoadTextureTile(displayListPtr++,
		       bmp,
		       G_IM_FMT_RGBA,
		       G_IM_SIZ_16b,
		       64, 64,
		       32, 0, 63, 31,
		       0,
		       G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
		       0, 0,
		       G_TX_NOLOD, G_TX_NOLOD);
    gSP2Triangles(displayListPtr++,4,5,6,0,4,6,7,0);

    /* Read texture data (the bottom-left part) */
    gDPLoadTextureTile(displayListPtr++,
		       bmp,
		       G_IM_FMT_RGBA,
		       G_IM_SIZ_16b,
		       64, 64,
		       0, 32, 31, 63,
		       0,
		       G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
		       0, 0,
		       G_TX_NOLOD, G_TX_NOLOD);
    gSP2Triangles(displayListPtr++,8,9,10,0,8,10,11,0);

    /* Read texture data (the bottom-right part) */
    gDPLoadTextureTile(displayListPtr++,
		       bmp,
		       G_IM_FMT_RGBA,
		       G_IM_SIZ_16b,
		       64, 64,
		       32, 32, 63, 63,
		       0,
		       G_TX_WRAP | G_TX_NOMIRROR, G_TX_WRAP | G_TX_NOMIRROR,
		       0, 0,
		       G_TX_NOLOD, G_TX_NOLOD);
    gSP2Triangles(displayListPtr++,12,13,14,0,12,14,15,0);

    gDPPipeSync(displayListPtr++);
}
