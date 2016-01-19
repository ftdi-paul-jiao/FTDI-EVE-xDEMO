/* 
    Sample code to show how to display RAW file
    Author: Hawk
	Email : hawk.gao@ftdichip.com
	Date  : 2015/May
*/
#ifdef DEMO_DISPRAW

#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU) || defined(FT9XXEV)
#define DISPRAW_PATH ROOT_PATH"dispraw\\test.raw"
#else
#define DISPRAW_PATH ROOT_PATH"test.raw"
#endif

typedef struct app_para_st {
	FTU32 appIndex;
	FTU32 appPara;
}app_para_t;

app_para_t appGP = {0};

FTVOID dispraw (FTU32 para)
{
	bmpHDR_st bmp_header = {
		DISPRAW_PATH,
		0,
		RGB565,
		0,
		480,
		272
	};
	static FTU8 flag = 0;
	/* never mind, it's for debug,
	 * this part just for this routine jump out the outside caller when error happen */
	appGP.appIndex = 1;
	appGP.appPara = 0;

	/* only load the file once */
	if (flag == 0) {
		/* load bitmap resources data into FT800 */
		if(APP_OK != appBmpToRamG(0, RAM_G, &bmp_header, 1)){
			DBGPRINT;
			return;
		}
		flag = 1;
	}

	HAL_CmdBufIn(CMD_DLSTART);
	HAL_CmdBufIn(CLEAR_COLOR_RGB(0,0,0));
	HAL_CmdBufIn(CLEAR(1,1,1));

	HAL_CmdBufIn(BEGIN(BITMAPS));
    HAL_CmdBufIn(BITMAP_HANDLE(0));
    HAL_CmdBufIn(CELL(0));
    HAL_CmdBufIn(VERTEX2F(0,0));
	HAL_CmdBufIn(END());
	CoCmd_TEXT(FT800_LCD_WIDTH/2,FT800_LCD_HIGH/2,24,OPT_CENTERX,"show the RAW file on screen");

	HAL_CmdBufIn(DISPLAY());
	HAL_CmdBufIn(CMD_SWAP);
	HAL_BufToReg(RAM_CMD,0);

	appGP.appIndex = 0;
}

AppFunc Apps[] = {
	dispraw,
	/* Leave this NULL at the buttom of this array */
	NULL
};

#endif
