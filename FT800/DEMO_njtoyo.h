/* 
    Sample code to show how to display continous JPEG in many windows
    Author: Hawk
	Email : hawk.gao@ftdichip.com
	Date  : 2016/Apr
    when MCU support QSPI
    set the EVE_SPI_TYPE to 4 for better performance 
*/
#ifdef DEMO_NJTOYO

FTU8 jpeg_a[] = ROOT_PATH"njtoyo\\A00.jpg";
FTU8 jpeg_b[] = ROOT_PATH"njtoyo\\B00.jpg";
FTU8 jpeg_c[] = ROOT_PATH"njtoyo\\C00.jpg";
FTU8 jpeg_d[] = ROOT_PATH"njtoyo\\D00.jpg";

/* make sure FIFOSIZE larger than JPG file size
   and less than (1024-LCD_WIDTH*LCD_HIGH*2/1024) */
#define FIFOSIZE        (25*1024)
/* Based on my current understanding, EVE JPEG decode
   to RGB565 for display, so file len in RAM_G
   should be wide * high * 2 */
#define BMPSIZE         (320*240*2)
/* leave a gap between 
   fifo address and bitmap address
   so far I don't know why
   use larger than 1 makes it work
   I use 4 for better align*/
#define BUF_GAP          (4)

#define FIFOADDR         (RAM_G)
#define BMPADDR          (FIFOADDR+FIFOSIZE+BUF_GAP)

#define EVE_CODE_ADDR      (BMPADDR_4+BMPSIZE)

bmpHDR_st jpeg_bmp[4] = {
    {(FTC8 *)jpeg_a,0,0,RGB565,BMPSIZE,0,320,240},
    {(FTC8 *)jpeg_b,0,0,RGB565,BMPSIZE,0,320,240},
    {(FTC8 *)jpeg_c,0,0,RGB565,BMPSIZE,0,320,240},
    {(FTC8 *)jpeg_d,0,0,RGB565,BMPSIZE,0,320,240}
};

typedef struct jpeg_positon_ {
    FTU32 X;
    FTU32 Y;
}jpeg_positon_st;

typedef struct jpeg_play_ {
    FTU8  play;
    jpeg_positon_st pic;
    jpeg_positon_st txt;
}jpeg_play_st;

jpeg_play_st jpeg_play_list[4] = {
    {1, {0,        0        },{320/2,        240/2      }},
    {1, {(800-320),0        },{800 - (320/2),240/2      }},
    {1, {0,        (480-240)},{320/2,        480 - 240/2}},
    {1, {(800-320),(480-240)},{800 - (320/2),480 - 240/2}}
};

#define TAG_START  (10)
#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
#define FILE_INDEX 18
#elif defined(FT9XXEV)
#define FILE_INDEX 8
FTU8 frame_rate[4] = {0};
FTU8 tick = '|';
#else
#error "not for this platform"
#endif

/*
 * detect press, mark it
 * then leave it unchange
 * until it release
 * then do the change
 */
FTVOID pressTHENrelease (FTVOID)
{
    static FTU8 pre_t = 0;
    FTU32 reg_tag = 0;

	if (TOUCHED) {
		reg_tag = HAL_Read32(REG_TOUCH_TAG);
		if (TAG_START <= reg_tag && TAG_START+4 >= reg_tag) {
			if (pre_t == reg_tag) {
				return;
			}
			pre_t = reg_tag;
		}
	} else if (TAG_START <= pre_t && TAG_START+4 >= pre_t) {
        jpeg_play_list[pre_t - TAG_START].play = 
			jpeg_play_list[pre_t - TAG_START].play?0:1;
        pre_t = 0;
    } 
}
#if defined(DEF_TIMER)
void timerISR(void)
{
    static FTU8 pre_mark[4] = {0};
    FTU8 i,j;

    if (timer_is_interrupted(timer_select_a) == 1)
    {
        tick = (tick == '|')?'-':'|';
        for (i = 0; i < 4; i++) {
            j = (*(FTU8 *)(jpeg_bmp[i].path + FILE_INDEX) - '0')*10;
            j += (*(FTU8 *)(jpeg_bmp[i].path + FILE_INDEX + 1) - '0')%10;
            if (pre_mark[i] <= j) {
                frame_rate[i] = j - pre_mark[i];
            } else {
                frame_rate[i] = 79 - pre_mark[i] + j;
            }
            pre_mark[i] = j;
        }
    }
}
#endif
FTINDEF FTU32 mfifoImageWrite (FTU32 mfifo_addr, FTU32 mfifo_size,FTU32 disp_addr,FTU32 opt,FTU32 resHDL, FTU32 file_len)
{
	FTU32 mfifo_rd, mfifo_wr;

	/* mfifo size should not less than file len ,
	 * mfifo address must different with display area in RAMG */
	if ((mfifo_size < file_len) || 
		((disp_addr >= mfifo_addr) && 
		 (disp_addr <= (mfifo_addr + mfifo_size)))) {
		return 0;
	}

	/* set the MEDIAFIFO space in RAMG*/
	CoCmd_MEDIAFIFO(mfifo_addr, mfifo_size);

	HAL_BufToReg(RAM_CMD, 0);

	/* write data to RAM_G */
	appResToDes(resHDL, mfifo_addr, 0, file_len, resWrEve);

	HAL_Write32(REG_MEDIAFIFO_WRITE, file_len);

	CoCmd_LOADIMAGE(disp_addr,opt);

	HAL_BufToReg(RAM_CMD, 0);

	/* wait till previous action finished */
	do {
		mfifo_rd = HAL_Read32(REG_MEDIAFIFO_READ); 
		mfifo_wr = HAL_Read32(REG_MEDIAFIFO_WRITE);
	} while (mfifo_rd != mfifo_wr);

	return file_len;
}

FTINDEF FTU32 ImageToRamG(FTU8 *path, FTU32 ramgAddr, FTU32 fifoAddr, FTU32 fifoSize)
{
	FTU32 resHDL, Len;

	resHDL = appResOpen(path);
	if (resHDL == 0) {
		DBGPRINT;
		return 0;
	}

	Len = appResSize(resHDL);
	if (FT800_RAMG_SIZE < ramgAddr + Len) {
		appResClose(resHDL);
		DBGPRINT;
		return 0;
	}

    /*
     * Command Buffer: |---------------not used for Image file buffer--------------|
     * RAMG          : |---***display area***---***media fifo JPEG file data***---|
     */
    mfifoImageWrite(fifoAddr,fifoSize,ramgAddr,OPT_MEDIAFIFO,resHDL,Len);

	appResClose(resHDL);

	return Len;
}

FTVOID change4file (FTU32 para)
{
    static FTU8 frame[4] = {0};
    FTU8 i;

    appGP.appIndex = 0;

    for (i = 0; i < 4; i++) {
        if (jpeg_play_list[i].play) {
            if (frame[i] >= 79) {
                frame[i] = 0;
            } else {
                frame[i]++;
            }
            *(FTU8 *)(jpeg_bmp[i].path + FILE_INDEX) = '0' + frame[i]/10;
            *(FTU8 *)(jpeg_bmp[i].path + FILE_INDEX + 1) = '0' + frame[i]%10;
        }
    }
}

FTU8 dump4jpeg (FTVOID)
{
    FTU32 len,i;

    pressTHENrelease();

    for (i = 0; i < 4; i++) {
        if (jpeg_play_list[i].play) {
            len = ImageToRamG((FTU8 *)jpeg_bmp[i].path,BMPADDR + i*BMPSIZE,FIFOADDR,FIFOSIZE);
            if (len == 0) {
                DBGPRINT;
                return 1;
            }
        }
    }

    return 0;
}

FTVOID disp4jpeg (FTU32 para)
{
#define FPS_OFFSET_Y 0
#define FPS_OFFSET_X 80

    static FTU32 preDLaddr = 0;
    FTU8 i;

    appGP.appIndex = 2;

    if (preDLaddr == 0) {
        FillBmpDL(0, BMPADDR, jpeg_bmp, 4);
        preDLaddr = 1;
    }

    if (dump4jpeg()) {
        DBGPRINT;
        return;
    }

    HAL_CmdBufIn(CMD_DLSTART);
    HAL_CmdBufIn(CLEAR_COLOR_RGB(0,0,0));
    HAL_CmdBufIn(CLEAR(1,1,1));

    HAL_CmdBufIn(BEGIN(BITMAPS));

    for (i = 0; i < 4; i++) {
        HAL_CmdBufIn(TAG_MASK(1));
        HAL_CmdBufIn(TAG(TAG_START+i));

        HAL_CmdBufIn(BITMAP_HANDLE(i));
        HAL_CmdBufIn(CELL(0));
        HAL_CmdBufIn(VERTEX2F(jpeg_play_list[i].pic.X*FT800_PIXEL_UNIT,
                    jpeg_play_list[i].pic.Y*FT800_PIXEL_UNIT));
        HAL_CmdBufIn(TAG_MASK(0));
    }
    HAL_CmdBufIn(END());

    HAL_CmdBufIn(COLOR_RGB(0,0xFF,0));

    for (i = 0; i < 4; i++) {
        CoCmd_TEXT(jpeg_play_list[i].txt.X,
                jpeg_play_list[i].txt.Y,25,
                OPT_CENTERX,&jpeg_bmp[i].path[FILE_INDEX-1]);
#if defined(DEF_TIMER)
        CoCmd_TEXT(800/2,0,25,OPT_CENTERX,&tick);
        CoCmd_TEXT(jpeg_play_list[i].pic.X,
                jpeg_play_list[i].pic.Y,25,
                0,"FPS:");
        CoCmd_NUMBER(jpeg_play_list[i].pic.X+FPS_OFFSET_X,
                jpeg_play_list[i].pic.Y,25,
                0,frame_rate[i]);
#endif
    }
    HAL_CmdBufIn(DISPLAY());
    HAL_CmdBufIn(CMD_SWAP);
    HAL_BufToReg(RAM_CMD,0);

    appGP.appIndex = 1;
}

AppFunc Apps[] = {
	disp4jpeg,
    change4file,
	/* Leave this NULL at the buttom of this array */
	NULL
};

#endif

