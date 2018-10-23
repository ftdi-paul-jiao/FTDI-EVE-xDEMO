/* 
    Sample code to demonstrate impliment a bean machine UI
    Author: Hawk
	Email : hawk.gao@ftdichip.com
	Date  : 2018/Jul
*/

#if defined(VC_EMULATOR)
#error "copy res/bean/bt81x.flash to res/flash, then comment this line"
#elif defined(VC_MPSSE) || defined(VC_FT4222)
#error "program res/bean/bt81x.flash to on-board flash, then comment this line"
#endif

#define UI_1_PATH     "ASTC_FLASH@4096"
#define UI_2_PATH     "ASTC_FLASH@134656"
#define UI_3_PATH     "ASTC_FLASH@265216"
#define UI_4_PATH     "ASTC_FLASH@395776"

#define ANIM_ADDR     1942336
#define ANIM_FRAM     284
#define ANIM_W        140
#define ANIM_H        140
#define ANIM_X        351
#define ANIM_Y        116

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

typedef enum UI_HDL_ {
    HDL_UI1 = 0,
    HDL_UI2,
    HDL_UI3,
    HDL_UI4,
} UI_HDL_ENUM;

ImgInfo_st info_hdr[] = {
    {UI_1_PATH,   0,0,0},
    {UI_2_PATH,   0,0,0},
    {UI_3_PATH,   0,0,0},
    {UI_4_PATH,   0,0,0},
};

bmpHDR_st bmp_hdr[] = {
    {COMPRESSED_RGBA_ASTC_4x4_KHR,EVE_LCD_WIDTH,EVE_LCD_HIGH,(FTU32)&info_hdr[HDL_UI1]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,EVE_LCD_WIDTH,EVE_LCD_HIGH,(FTU32)&info_hdr[HDL_UI2]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,EVE_LCD_WIDTH,EVE_LCD_HIGH,(FTU32)&info_hdr[HDL_UI3]},
    {COMPRESSED_RGBA_ASTC_4x4_KHR,EVE_LCD_WIDTH,EVE_LCD_HIGH,(FTU32)&info_hdr[HDL_UI4]},
};

FTU8 load_resources(bmpHDR_st *p_res, FTU8 num)
{
    if (appFlashSetFull()) {
        FTPRINT("\nflash error occur");
        return 1;
    }

    if(appBmpToRamG(HDL_UI1, RAM_G, p_res, num)){
        FTPRINT("\nload bitmap error occur");
        return 1;
    }

    return 0;
}

FTVOID loop_frame(FTU32 *pframe, FTU32 max)
{
    if (*pframe >= max-1) {
        /* this animation file from a gif
           the gif first 2 frame are blank
           so skip this two blank frame*/
        *pframe = 2;
    } else {
        *pframe = *pframe + 1;
    }
}

FTVOID bean_machine(FTU32 para)
{
	static FTU8 load = 1;
    static FTU32 frame = 2, anim_x = ANIM_X, anim_y = ANIM_Y;
    static FT16 seed = 0, move = 0, x1 = -1*EVE_LCD_WIDTH, x2 = 0, x3 = EVE_LCD_WIDTH;
    FTU16 x = (HAL_Read32(REG_TOUCH_SCREEN_XY) >> 16);

    if (load) {
        if(load_resources(bmp_hdr, sizeof(bmp_hdr)/sizeof(bmpHDR_st))) {
            return;
        }
        load = 0;
    }

	HAL_CmdBufIn(CMD_DLSTART);
	HAL_CmdBufIn(CLEAR_COLOR_RGB(255, 255, 255));
	HAL_CmdBufIn(CLEAR(1, 1, 1));

    CoCmd_ANIMFRAME(anim_x, anim_y, ANIM_ADDR, frame);
    loop_frame(&frame, ANIM_FRAM);

    HAL_CmdBufIn(BEGIN(BITMAPS));
    HAL_CmdBufIn(BITMAP_HANDLE(HDL_UI1));
    HAL_CmdBufIn(VERTEX2F(x1*EVE_PIXEL_UNIT,0));

    HAL_CmdBufIn(BITMAP_HANDLE(HDL_UI4));
    HAL_CmdBufIn(VERTEX2F(x2*EVE_PIXEL_UNIT,0));

    HAL_CmdBufIn(BITMAP_HANDLE(HDL_UI2));
    HAL_CmdBufIn(VERTEX2F(x3*EVE_PIXEL_UNIT,0));

    HAL_CmdBufIn(END());
	HAL_CmdBufIn(DISPLAY());
	HAL_CmdBufIn(CMD_SWAP);

	HAL_BufToReg(RAM_CMD, 0);

    appGP.appIndex = 0;
    
    if (0x8000 & x) {
        seed = 0;
        if (x2 >= EVE_LCD_WIDTH/2) {
            x1 = 0;
            x2 = EVE_LCD_WIDTH;
            x3 = EVE_LCD_WIDTH * 2;
            anim_x = EVE_LCD_WIDTH+ANIM_X;
        } else if (x2 <= (-1 * EVE_LCD_WIDTH/2)) {
            x1 = -2 * EVE_LCD_WIDTH;
            x2 = -1 * EVE_LCD_WIDTH;
            x3 = 0;
            anim_x = -1 * (EVE_LCD_WIDTH - ANIM_X);
        } else {
            x1 = -1 * EVE_LCD_WIDTH;
            x2 = 0;
            x3 = EVE_LCD_WIDTH;
            anim_x = ANIM_X;
        }
        return;
    }

    if (seed == 0) {
        seed = x;
        return;
    } else {
        move = x - seed;
        seed = x;
    }

    anim_x += move;
    x1 += move;
    x2 += move;
    x3 += move;
}

AppFunc APPS_UI[] = {
	bean_machine,
};

