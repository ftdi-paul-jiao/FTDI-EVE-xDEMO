/* 
    Sample code to demonstrate impliment a water machine UI
    Author: Hawk
	Email : hawk.gao@ftdichip.com
	Date  : 2018/Jun
*/

#if defined(VC_EMULATOR)
#error "copy res/water/eveflh.bin to res/eveflh, then comment this line"
#elif defined(VC_MPSSE) || defined(VC_FT4222)
#error "program res/water/eveflh.bin to eve-connected-flash, then comment this line"
#endif

#define BKGRD_PATH         "EVEFLH@30976:130560"
#define BKGRD_X            0
#define BKGRD_Y            0

#define BAR_PATH           "EVEFLH@4096:26880"
#define BAR_X              0
#define BAR_Y              0

#define NUM_PATH           "EVEFLH@219776:29568"
#define NUM_X              13
#define NUM_Y              69

#define TUBE_ANIM_ADDR     810368
#define TUBE_ANIM_FRAM     24
#define TUBE_W             222
#define TUBE_H             134
#define TUBE_X             255
#define TUBE_Y             69

#define DOWN_W             128
#define DOWN_H             64
#define DOWN_1_PATH        "EVEFLH@211584:8192"
#define DOWN_1_ANIM_ADDR   211456
#define DOWN_1_ANIM_FRAM   10
#define DOWN_1_X           34
#define DOWN_1_Y           203

#define UP_W               48
#define UP_H               52
#define UP_1_PATH          "EVEFLH@860352:2496"
#define UP_1_ANIM_ADDR     860224
#define UP_1_ANIM_FRAM     10
#define UP_1_X             144
#define UP_1_Y             0

#define SUB_UP_PATH        "EVEFLH@379904:130560"
#define SUB_DOWN_PATH      "EVEFLH@249344:130560"

/* I don't have 480x272 screen on my hand now,
   so use 800x480 screen to demonstrate it
#ifdef EVE_LCD_WIDTH
#undef EVE_LCD_WIDTH
#define EVE_LCD_WIDTH 480
#endif

#ifdef EVE_LCD_HIGH
#undef EVE_LCD_HIGH
#define EVE_LCD_HIGH 272
#endif
*/
ImgInfo_st info_hdr[] = {
    {BKGRD_PATH,   0,0,0},
    {BAR_PATH,    0,0,0},
    {NUM_PATH,    0,0,0},
    {DOWN_1_PATH, 0,0,0},
    {UP_1_PATH,   0,0,0},
    {SUB_UP_PATH, 0,0,0},
    {SUB_DOWN_PATH,0,0,0},
};
bmpHDR_st bmp_hdr_main[] = {
    {COMPRESSED_RGBA_ASTC_4x4_KHR,EVE_LCD_WIDTH,EVE_LCD_HIGH,(FTU32)&info_hdr[0]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,EVE_LCD_WIDTH,56,          (FTU32)&info_hdr[1]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,224,132,                   (FTU32)&info_hdr[2]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,DOWN_W,DOWN_H,             (FTU32)&info_hdr[3]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,UP_W,UP_H,                 (FTU32)&info_hdr[4]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,EVE_LCD_WIDTH,EVE_LCD_HIGH,(FTU32)&info_hdr[5]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,EVE_LCD_WIDTH,EVE_LCD_HIGH,(FTU32)&info_hdr[6]},
};

#define HDL_START   0
#define SUB_ID_UP   0xaa
#define SUB_ID_DOWN 0xbb

typedef enum ITEM_ {
    HDL_BKGRD = HDL_START,
    HDL_BAR,
    HDL_NUM,
    HDL_DOWN_1,
    HDL_UP_1,
    HDL_SUB_UP,
    HDL_SUB_DOWN,
} ITEM_ENUM;

typedef enum ITEM_TAG_ {
    TAG_DOWN_1 = 1,
    TAG_UP_1,
} ITEM_TAG_ENUM;

FTU8 load_resources(bmpHDR_st *p_res, FTU8 num)
{
    if (appEVEFLHSetFull()) {
        FTPRINT("\nflash error occur");
        return 1;
    }

    if(appBmpToRamG(HDL_START, RAM_G, p_res, num)){
        FTPRINT("\nload bitmap error occur");
        return 1;
    }

    return 0;
}

FTVOID loop_frame(FTU32 *pframe, FTU32 max)
{
    if (*pframe >= max-1) {
        *pframe = 0;
    } else {
        *pframe = *pframe + 1;
    }
}

FTVOID water_machine_sub(FTU32 para)
{
    FTU8 go_back = 0;
#if !defined(CAL_NEEDED)
    static FTU8 tick = 0;
#endif

	HAL_CmdBufIn(CMD_DLSTART);
	HAL_CmdBufIn(CLEAR_COLOR_RGB(255, 255, 255));
	HAL_CmdBufIn(CLEAR(1, 1, 1));

    HAL_CmdBufIn(BEGIN(BITMAPS));

    if (SUB_ID_UP == para) 
        HAL_CmdBufIn(BITMAP_HANDLE(HDL_SUB_UP));
    if (SUB_ID_DOWN == para) 
        HAL_CmdBufIn(BITMAP_HANDLE(HDL_SUB_DOWN));
    HAL_CmdBufIn(CELL(0));
    HAL_CmdBufIn(VERTEX2F(0,0));
 	
    HAL_CmdBufIn(END());
	HAL_CmdBufIn(DISPLAY());
	HAL_CmdBufIn(CMD_SWAP);

	HAL_BufToReg(RAM_CMD, 0);

#if !defined(CAL_NEEDED)
    if (++tick == 100) {
        tick = 0;
#else
    while (TOUCHED) {
#endif
        go_back = 1;
        appGP.appPara = 0;
    }

    appGP.appIndex = go_back?0:1;
}

FTVOID water_machine(FTU32 para)
{
	static FTU8 load = 1, slow_tub = 0, slow_up1 = 0, slow_down1 = 0;
	static FTU32 anim_tub = 0, anim_up1 = 0, anim_down1 = 0;
#if !defined(CAL_NEEDED)
    static FTU8 tick1 = 0, tick2 = 0;
#endif
    FTU8 tag = TOUCHED?HAL_Read8(REG_TOUCH_TAG):0;

	if (load) {
        if(load_resources(bmp_hdr_main, sizeof(bmp_hdr_main)/sizeof(bmpHDR_st))) {
            return;
        }
        load = 0;
	}
#if !defined(CAL_NEEDED)
    if (tick1 == 100 ) {
        tag = TAG_UP_1;
        if (50 == ++tick2) {
            tag = 0;
            tick2 = 0;
            tick1++;
        }
    } else if (tick1 == 200) {
        tag = TAG_DOWN_1;
        if (50 == ++tick2) {
            tag = 0;
            tick2 = 0;
            tick1++;
        }
    } else {
        tick1++;
    }
#endif
	HAL_CmdBufIn(CMD_DLSTART);
	HAL_CmdBufIn(CLEAR_COLOR_RGB(255, 255, 255));
	HAL_CmdBufIn(CLEAR(1, 1, 1));

    HAL_CmdBufIn(BEGIN(BITMAPS));

    /* main UI background */
    HAL_CmdBufIn(BITMAP_HANDLE(HDL_BKGRD));
    HAL_CmdBufIn(CELL(0));
    HAL_CmdBufIn(VERTEX2F(BKGRD_X,BKGRD_Y));
    
    /* main UI bar */
    HAL_CmdBufIn(BITMAP_HANDLE(HDL_BAR));
    HAL_CmdBufIn(CELL(0));
    HAL_CmdBufIn(VERTEX2F(BAR_X*EVE_PIXEL_UNIT,BAR_Y*EVE_PIXEL_UNIT));
   
    /* main UI number */
    HAL_CmdBufIn(BITMAP_HANDLE(HDL_NUM));
    HAL_CmdBufIn(CELL(0));
    HAL_CmdBufIn(VERTEX2F(NUM_X*EVE_PIXEL_UNIT,NUM_Y*EVE_PIXEL_UNIT));

    /* main UI down 1 */
    HAL_CmdBufIn(TAG_MASK(1));
    HAL_CmdBufIn(TAG(TAG_DOWN_1));
    if (TAG_DOWN_1 == tag) {
        CoCmd_ANIMFRAME(DOWN_1_X+DOWN_W/2+8, DOWN_1_Y+DOWN_H/2+4, DOWN_1_ANIM_ADDR, anim_down1);
        if (++slow_down1 % 3 == 0) {
            loop_frame(&anim_down1, DOWN_1_ANIM_FRAM);
        }
        appGP.appPara = SUB_ID_DOWN;
    } else {
        HAL_CmdBufIn(BITMAP_HANDLE(HDL_DOWN_1));
        HAL_CmdBufIn(CELL(0));
        HAL_CmdBufIn(VERTEX2F(DOWN_1_X*EVE_PIXEL_UNIT,DOWN_1_Y*EVE_PIXEL_UNIT));
    }
    HAL_CmdBufIn(TAG_MASK(0));
    
    /* main UI up 1 */
    HAL_CmdBufIn(TAG_MASK(1));
    HAL_CmdBufIn(TAG(TAG_UP_1));
    if (TAG_UP_1 == tag) {
        CoCmd_ANIMFRAME(UP_1_X+UP_W/2, UP_1_Y+UP_H/2+10, UP_1_ANIM_ADDR, anim_up1);
        if (++slow_up1 % 3 == 0) {
            loop_frame(&anim_up1, UP_1_ANIM_FRAM);
        }
        appGP.appPara = SUB_ID_UP;
    } else {
        HAL_CmdBufIn(BITMAP_HANDLE(HDL_UP_1));
        HAL_CmdBufIn(CELL(0));
        HAL_CmdBufIn(VERTEX2F(UP_1_X*EVE_PIXEL_UNIT,UP_1_Y*EVE_PIXEL_UNIT));
    }
    HAL_CmdBufIn(TAG_MASK(0));

    /* animation parts */
	CoCmd_ANIMFRAME(TUBE_X+TUBE_W/2, TUBE_Y+TUBE_H/2, TUBE_ANIM_ADDR, anim_tub);
    if (++slow_tub % 8 == 0) {
        loop_frame(&anim_tub, TUBE_ANIM_FRAM);
    }

	HAL_CmdBufIn(END());
	HAL_CmdBufIn(DISPLAY());
	HAL_CmdBufIn(CMD_SWAP);

	HAL_BufToReg(RAM_CMD, 0);

    if (tag) {
        appGP.appIndex = 0;
    } else {
        appGP.appIndex = appGP.appPara?1:0;
    }
}

AppFunc APPS_UI[] = {
	water_machine,
	water_machine_sub,
};

