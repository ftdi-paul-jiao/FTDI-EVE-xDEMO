/* 
    Applicaton, Integrated function for FT800
    Author: Hawk
    Email : hawk.gao@ftdichip.com	
    Date  : 2013/Oct
 */
#include "platform.h"
#include "EVE_Gpu.h"
#include "EVE_HAL.h"
#include "EVE_APP.h"

#define FONT_CAL 27
#define FORCE_WAIT_COUNT 3
#define FORCE_WAIT_TIME 500
#define RETRY_COUNT  8
/* 
 * for FT9XX delay has to be larger than 10 
 * use a larger wait time (100) to expand the system's
 * adaptability
 */
#define READ_ID_WAIT 100
/* 
 * longer delay for CAP touch engin ready
 * for RES touch engin, 10 would be enough 
 */
#define CLK_DELAY 100
FTU8 dbg_str_buf[EVE_DBG_BUF_LEN] = {0};
FTU8 EVE_ID = 0;

typedef struct wrFuncPara_ {
    FTRES res;
    FTU32 Src;
    FTU32 Des;
    FTU32 len;
}wrFuncPara;

wrFuncPara fPara = {0};

#if !defined(STM32F4)&&!defined(MSVC2010EXPRESS)&&!defined(MSVC2012EMU)
FTU8 file_buff[MCU_BLOCK_SIZE] = {0};
#endif

#if defined(CAL_NEEDED)
STATIC appRet_en appCalCmd (FTU8 font, FTC8 *str1, FTC8 *str2)
{
    FTU32 ret_addr;

    HAL_CmdBufIn(CMD_DLSTART);

    HAL_CmdBufIn(CLEAR_COLOR_RGB(0,0,0));
    HAL_CmdBufIn(CLEAR(1,1,1));
    HAL_CmdBufIn(COLOR_RGB(255,255,255));

    CoCmd_TEXT(FT800_LCD_WIDTH/2,FT800_LCD_HIGH/2,font,OPT_CENTER,str1);
    CoCmd_TEXT(FT800_LCD_WIDTH/2,FT800_LCD_HIGH/3*2,font,OPT_CENTER,
               (str2 == NULL)?"NULL":str2);
    HAL_CmdBufIn(CMD_CALIBRATE);
    HAL_CmdBufIn(0);
    ret_addr = HAL_CalResultAddr();
    HAL_BufToReg(RAM_CMD,0);

    return (0 != HAL_Read32(ret_addr))?APP_OK:APP_ERR_CALC;
}

STATIC appRet_en appCalForce (FTC8 *dataPath)
{
    FTU32 CData[FT800_CAL_PARA_NUM] = {0},
          reg = REG_TOUCH_TRANSFORM_A, i = 0;

    /* make the tag part invalid */
#if defined(STM32F4)
    stm32f4_invaild_tag();
#elif defined(MSVC2012EMU)
    vc2012emu_invaild_tag(dataPath);
#elif defined(MSVC2010EXPRESS)
    vc2010_invaild_tag(dataPath);
#elif defined(FT9XXEV)
    ft9xx_invaild_tag(dataPath);
#else
    arduino_invaild_tag();
#endif

    /* do calibration to get the CData */
#if defined(STM32F4)
    if (APP_OK == appCalCmd(FONT_CAL,(FTC8 *)"Tap the flashing point (STM32F4)",dataPath)) {
#elif defined(MSVC2012EMU)
    if (APP_OK == appCalCmd(FONT_CAL,(FTC8 *)"Tap the flashing point (VC2012Emu)",dataPath)) {
#elif defined(MSVC2010EXPRESS)
    if (APP_OK == appCalCmd(FONT_CAL,(FTC8 *)"Tap the flashing point (VC2010)",dataPath)) {
#elif defined(FT9XXEV)
    if (APP_OK == appCalCmd(FONT_CAL,(FTC8 *)"Tap the flashing point (FT9XX)",dataPath)) {
#else
    if (APP_OK == appCalCmd(FONT_CAL,(FTC8 *)"Tap the flashing point (Arduino)",dataPath)) {
#endif
        for (i = 0; i < FT800_CAL_PARA_NUM; reg+=FTU32_LEN,i++) {
            CData[i] = HAL_Read32(reg);
        }
        /* save the CData */
#if defined(STM32F4)
        stm32f4_save_cdata((FTU16 *)CData);
#elif defined(MSVC2012EMU)
        vc2012emu_save_cdata(dataPath, (FTU8 *)CData);
#elif defined(MSVC2010EXPRESS)
        vc2010_save_cdata(dataPath, (FTU8 *)CData);
#elif defined(FT9XXEV)
        ft9xx_save_cdata(&FT9xxFile,dataPath, (FTU8 *)CData);
#else
        arduino_save_cdata((FTU8 *)CData);
#endif

        /* make the tag part valid */
#if defined(STM32F4)
        stm32f4_vaild_tag();
#elif defined(MSVC2012EMU)
        vc2012emu_vaild_tag();
#elif defined(MSVC2010EXPRESS)
        vc2010_vaild_tag();
#elif defined(FT9XXEV)
        ft9xx_vaild_tag();
#else
        arduino_vaild_tag();
#endif
    }
    return APP_OK;
}

STATIC appRet_en appCal (FTU8 force, FTC8 *dPath)
{
    FTU32 CData[FT800_CAL_PARA_NUM] = {0};
    FTU32 reg = REG_TOUCH_TRANSFORM_A, i = 0;

    FTPRINT("\nCalibration");
#if defined(MSVC2012EMU) || defined(MSVC2010EXPRESS) || defined(FT9XXEV)
    if (force || (dPath == NULL)) {
#else
        if (force) {
#endif
            return appCalForce(dPath);
    }

#if defined(STM32F4)
    if (!stm32f4_is_tag_vaild()) {
#elif defined(MSVC2012EMU)
    if (!vc2012emu_is_tag_vaild(dPath)) {
#elif defined(MSVC2010EXPRESS)
    if (!vc2010_is_tag_vaild(dPath)) {
#elif defined(FT9XXEV)
    if (!ft9xx_is_tag_vaild(&FT9xxFile,dPath)) {
#else
    if (!arduino_is_tag_vaild()) {
#endif
        return appCalForce(dPath);
    }

#if defined(STM32F4)
    stm32f4_restore_cdata((FTU16 *)CData);
#elif defined(MSVC2012EMU)
    vc2012emu_restore_cdata(dPath, (FTU8 *)CData);
#elif defined(MSVC2010EXPRESS)
    vc2010_restore_cdata(dPath, (FTU8 *)CData);
#elif defined(FT9XXEV)
    ft9xx_restore_cdata(&FT9xxFile,dPath, (FTVOID *)CData);
#else
    arduino_restore_cdata((FTU8 *)CData);
#endif
    while (reg <= REG_TOUCH_TRANSFORM_F) {
        HAL_Write32(reg, CData[i]);
        i++;
        reg += FTU32_LEN;
    }
    return APP_OK;
}
#endif

FTU32 appGetLinestride(bmpHDR_st bmpHD)
{
    FTU32 linestride = 0;

    switch (bmpHD.format) {
        case L1:
            linestride = bmpHD.wide/8;
            break;
        case L2:
            linestride = bmpHD.wide/4;
            break;
        case L4:
            linestride = bmpHD.wide/2;
            break;
        case L8:
        case ARGB2:
        case RGB332:
#ifdef DEF_81X
        case PALETTED8:
        case PALETTED565:
        case PALETTED4444:
#else
        case PALETTED:
#endif 
            linestride = bmpHD.wide;
            break;
        case ARGB4:
        case RGB565:
        case ARGB1555:
        default:
            linestride = bmpHD.wide*2;
            break;
    }

    return linestride;
}
/* 
 * call back routine for multi-platform
 * read data from Src (MCU, EVE, Flash, SD Card) to Des (MCU, EVE, Flash, SD Card)
 * at a time 
 * set the length as return
 */
FTVOID resWrBuf (FTU32 para)
{
    wrFuncPara *wfp = (wrFuncPara *) para;
#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
    fseek(wfp->res,wfp->Src,SEEK_SET);
    if (wfp->len > 0) {
        fread((FTVOID *)wfp->Des,1,wfp->len,wfp->res);
    }
#elif defined(STM32F4)
    FTU32 i;
    for (i = 0; i < wfp->len ; i++) {
        *(FTU8 *)(wfp->Des+i) = *(FTU8 *)(wfp->res+wfp->Src+i);
    }
#elif defined(FT9XXEV)
    FTU32 l;
    f_lseek(wfp->res,wfp->Src);
    if (wfp->len > 0) {
        f_read(wfp->res, (FTVOID *)wfp->Des,wfp->len,&l);
    }
#else/* Arduino */
    FTU32 i,l;
    FTU8 p[SDC_SECTOR_SIZE] = {0};

    /* only accept forward read
     * make sure not backward over read 
     * the previous finished read info */
    if (wfp->res.offset > wfp->Src) {
        wfp->len = 0;
        FTPRINT("\nresWrBuf: backward reading");
        return;
    }

    for (l = 0, i = wfp->Src%SDC_SECTOR_SIZE; l < wfp->len; ) {
        wfp->res.readsector(p);
        for (; i < SDC_SECTOR_SIZE && l < wfp->len; i++,l++) {
            *(FTU8 *)(wfp->Des+l) = p[i];
        }
        i = 0;
    }

#endif
}
/* 
 * call back routine for multi-platform
 * read data from Src (MCU, EVE, Flash, SD Card) to Des (EVE-CMD, EVE-DLP, EVE-RAM_G)
 * block by block
 * in loop buffer in EVE (CMD buffer), or RAM_G, or DLP
 * set the length as return
 */
STATIC FTVOID resEveMemOperation (FTU32 para, FTU8 isCmdLoopBuf)
{
    wrFuncPara *wfp = (wrFuncPara *) para;
    /* make sure the data length into EVE is 4bytes aligned */
    FTU32 i,block,l,file_len = BYTES4ALIGN(wfp->len);
    FTU8 * p = 0;

    if (isCmdLoopBuf) {
        block = CMDBUF_SIZE/2;
    } else {
        block = MCU_BLOCK_SIZE;
    }

#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
    p = (FTU8 *)malloc(block);
    if (p == 0) {
        FTPRINT("\neve mem: no memory");
        wfp->len = 0;
        return;
    }
#elif defined(STM32F4)
    p = (FTU8 *)wfp->res+wfp->Src;
#else
    p = file_buff;
/* it may not be a necessary step, but leave it here for now */
#if 0 
    /* only accept increasly read
     * make sure not over read the previous reading */
    if (wfp->res.offset > wfp->Src) {
        FTPRINT("\neve mem: offset error");
        wfp->len = 0;
        return;
    }
#endif
#endif

    for (i = 0; i < file_len; i += l) {
        l = ((file_len - i) > block)?block:(file_len - i);
#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)        
        fread(p,1,l,wfp->res);
#elif defined(FT9XXEV)
        /* 
           f_read need to pass a variable to last para 
           just reuse wfp->len for it, nothing special meaning
         */
        f_read(wfp->res, (FTVOID *)p,l,&(wfp->len));
#elif defined(STM32F4)
        p += l;
#else
        wfp->res.readsector(p);
#endif
        if (isCmdLoopBuf) {
            HAL_CmdWait(HAL_EVELoopMemWr(RAM_CMD, HAL_Read32(REG_CMD_WRITE), CMDBUF_SIZE, p, l));
        } else {
            HAL_Write8Src(wfp->Des+i, p, l);
        }
    }
    wfp->len = file_len;
#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
    free(p);
#endif
}
FTVOID resWrEve (FTU32 para)
{
    resEveMemOperation(para, 0);
}
FTVOID resWrEveCmd (FTU32 para)
{
    resEveMemOperation(para, 1);
}
FTVOID resIsZlib(FTU32 para)
{
    wrFuncPara *wfp = (wrFuncPara *) para;
#if defined(STM32F4)
    FTU8 header[2] = {0};
    header[0] = *(FTU8 *)(wfp->res+wfp->Src);
    header[1] = *(FTU8 *)(wfp->res+wfp->Src+1);
#elif defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
    FTU8 header[2] = {0};
    fread(header,1,2,wfp->res);
    fseek(wfp->res,wfp->Src,SEEK_SET);
#elif defined(FT9XXEV)
    FTU8 header[2] = {0};
    FTU32 l = 0;
    f_read(wfp->res, (FTVOID *)header,2,&l);
    f_lseek(wfp->res,wfp->Src);
#else /* Arduino */
    FTU8 header[512] = {0};
    wfp->res.readsector(header);
    wfp->res.seek(wfp->Src);
#endif
    /*
       zLib compression header
       +---+---+
       |CMF|FLG|
       +---+---+

       78 01 - No Compression/low
       78 9C - Default Compression
       78 DA - Best Compression 
     */
    if (header[0] == 0x78) {
        if (header[1] == 0x9C) {
            /* make sure command buffer clean */
            HAL_CmdWait((FTU16)HAL_Read32(REG_CMD_WRITE));
            /* then start sending the command */
            HAL_CmdToReg(CMD_INFLATE);
            HAL_CmdToReg(wfp->Des);
            resWrEveCmd(para);
            /* 
               give a special length for the return routine
               to do some special handle for zlib file
             */
            wfp->len = ZLIB_LEN;
            return;
        } else {
            /* 
               so far as I know
               our tools only generate default compressed file
             */
            wfp->len = 0;
            return; 
        }
    }

    resWrEve(para);
}
FTU32 appResOpen (FTU8 *path)
{
#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
    wrFuncPara *pwfp = 0;
    FILE *pFile;

    if(NULL == path)
    {  
        FTPRINT("\nappResOpen: path null");
        return 0;
    }

    pFile = fopen((FTC8 *)path,"rb");
    if(NULL == pFile)
    {  
        FTPRINT("\nappResOpen: file open error");
        return 0;
    }

    pwfp = &fPara;
    if (pwfp) {
        pwfp->res = pFile;
    }

    return (FTU32)pwfp;
#elif defined(STM32F4)
    return stm32f4fileopen(path, (FTU32)&fPara);
#elif defined(FT9XXEV)
    wrFuncPara *pwfp = 0;
    if (FR_OK != f_open(&FT9xxFile, (const TCHAR*)path, FA_READ)) {
        FTPRINT("\nappResOpen: file open error");
        return 0;
    }
    pwfp = &fPara;
    if (pwfp) {
        pwfp->res = &FT9xxFile;
    }

    return (FTU32)pwfp;
#else/* Arduino */
    wrFuncPara *pwfp = 0;
    Reader sdC;

    if (0 == sdC.openfile((FT8 *)path)) {
        FTPRINT("\nappResOpen: file open error");
        return 0;
    }

    pwfp = &fPara;
    if (pwfp) {
        pwfp->res = sdC;
    }

    return (FTU32)pwfp;
#endif
}
FTU32 appResSize (FTU32 resHDL)
{
#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
    wrFuncPara *para = (wrFuncPara *)resHDL;
    FTU32 size;

    fseek(para->res,0,SEEK_END);
    size = ftell(para->res);
    fseek(para->res,0,SEEK_SET);

    return BYTES4ALIGN(size);
#elif defined(STM32F4)
    return BYTES4ALIGN(stm32f4filesize(resHDL));
#elif defined(FT9XXEV)
    wrFuncPara *para = (wrFuncPara *)resHDL;
    return BYTES4ALIGN(f_size(para->res));
#else/* Arduino */
    wrFuncPara *para = (wrFuncPara *)resHDL;
    return BYTES4ALIGN(para->res.size);
#endif
}
/*
 * The reason why use this way other than directly
 * use open/read/write/close is:
 * some system do not has file system, and some
 * has file system, in order to make the code 
 * use same structure for better managment 
 * between each platform, I make a middle level
 * API (appResOpen,appResToDes,appResClose)
 * for multiple and flexability usage of "file" handling
 * in writeFunc
 */
FTU32 appResToDes (FTU32 resHDL, FTU32 Des, FTU32 Src, FTU32 len, AppFunc writeFunc)
{
    wrFuncPara *para = (wrFuncPara *)resHDL;

    if (writeFunc) {
        para->Src = Src;
        para->Des = Des;
        para->len = len;
        writeFunc(resHDL);
        /* writeFunc may change the len so use para after writeFunc executed */
        return para->len;
    }

    return 0;
}
FTVOID appResClose (FTU32 resHDL)
{
#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
    wrFuncPara *para = (wrFuncPara *)resHDL;
    fclose(para->res);
#elif defined(STM32F4)

#elif defined(FT9XXEV)
    wrFuncPara *para = (wrFuncPara *)resHDL;
    f_close(para->res);
#else/* Arduino */

#endif
}
/* the chkExceed flag provid the flexibility while using puzzle overlap function */
FTU32 appFileToRamG (FTC8 *path, FTU32 inAddr, FTU8 chkExceed, FTU8 *outAddr, FTU32 outLen)
{
    FTU32 Len, resHDL;

    resHDL = appResOpen((FTU8 *)path);
    if (resHDL == 0) {
        FTPRINT("\nappFileToRamG: file open error");
        return 0;
    }

    Len = appResSize(resHDL);
    if (chkExceed && (FT800_RAMG_SIZE < inAddr + Len)) {
        appResClose(resHDL);
        FTPRINT("\nappFileToRamG: FT800_RAMG_SIZE < inAddr + Len");
        return 0;
    }

    Len = appResToDes(resHDL,inAddr,0,Len,resIsZlib);

    if (outAddr) {
        appResToDes(resHDL,(FTU32)outAddr,0,outLen,resWrBuf);
    }

    appResClose(resHDL);

    return Len;
}
appRet_en appLoadBmp(FTU32 ramgAddr, bmpHDR_st *pbmpHD, FTU32 nums)
{
    FTU32 i, src, l;

    for (i = 0, src = ramgAddr; i < nums; i++) {
        l = appFileToRamG(pbmpHD[i].path,src,1,0,0);
        if (pbmpHD[i].len == 0) {
            /* only when input length == 0, set the real length */
            pbmpHD[i].len = l;
        }
        if (l) {
            /* 
               when using zlib compressed file (*.bin)
               the actual decompressed size would not
               the return value of appFileToRamG
               so calculate the output (decompressed) size by image format
             */
            if (ZLIB_LEN == pbmpHD[i].len) {
                pbmpHD[i].len = appGetLinestride(pbmpHD[i])*pbmpHD[i].high;
            }
        } else {
            FTPRINT("\nappLoadBmp: Len 0");
            return APP_ERR_LEN;
        }
        src += pbmpHD[i].len;
#ifdef DEF_81X
        if (PALETTED8 == pbmpHD[i].format || 
            PALETTED565 == pbmpHD[i].format || 
            PALETTED4444 == pbmpHD[i].format) {
            /* only when lut_src == 0, set the src */
            if (pbmpHD[i].lut_src == 0) {
                pbmpHD[i].lut_src = src;
            }
            l = appFileToRamG(pbmpHD[i].path_lut,pbmpHD[i].lut_src,1,0,0);
            if (pbmpHD[i].len_lut == 0) {
                /* only when input length == 0, set the real length */
                pbmpHD[i].len_lut = l;
            }
            if (l) {
                /* same reason as above, lookup table alway less than 1K */
                if (ZLIB_LEN == pbmpHD[i].len_lut) {
                    pbmpHD[i].len_lut = 1024;
                }
            } else {
                FTPRINT("\nappLoadBmp: 81X Lut 0");
                return APP_ERR_LEN;
            }

            src += pbmpHD[i].len_lut;
        }
#else
        if (PALETTED == pbmpHD[i].format) {
            pbmpHD[i].lut_src = RAM_PAL;
            /* only when length == 0, set the real length */
            if (pbmpHD[i].len_lut == 0) {
                pbmpHD[i].len_lut = appFileToRamG(pbmpHD[i].path_lut,pbmpHD[i].lut_src,0,0,0);
            }
            if (pbmpHD[i].len_lut) {
                /* same reason as above, lookup table alway less than 1K */
                if (ZLIB_LEN == pbmpHD[i].len_lut) {
                    pbmpHD[i].len_lut = 1024;
                }
            } else {
                FTPRINT("\nappLoadBmp: 80X Lut 0");
                return APP_ERR_LEN;
            }    
        }
#endif
    }

    return APP_OK;
}
/*
 * You may do this bitmap related display list setup here
 * or do it in your own routine
 * all base on your actual application needed
 * it just one of the way to tell the EVE about bitmap information
 */
FTVOID appUI_FillBmpDL(FTU32 bmpHdl, FTU32 ramgAddr, bmpHDR_st *pbmpHD, FTU32 nums)
{
    FTU32 i, src;

    for (i = 0, src = ramgAddr; i < nums; i++) {
        HAL_DlpBufIn(BITMAP_HANDLE(i+bmpHdl));
        HAL_DlpBufIn(BITMAP_SOURCE(src));
        HAL_DlpBufIn(BITMAP_LAYOUT(pbmpHD[i].format,appGetLinestride(pbmpHD[i]),pbmpHD[i].high));
#ifdef DEF_81X
        HAL_DlpBufIn(BITMAP_LAYOUT_H(appGetLinestride(pbmpHD[i]) >> 10,pbmpHD[i].high>>9));
#endif       
        /* 
         * select NEAREST or BILINEAR base on your image and requirement
         * NEAREST: make the image shap clear
         * BILINEAR: make the image shap smooth
         */
        HAL_DlpBufIn(BITMAP_SIZE(NEAREST,BORDER,BORDER,pbmpHD[i].wide,pbmpHD[i].high));
#ifdef DEF_81X
        HAL_DlpBufIn(BITMAP_SIZE_H(pbmpHD[i].wide >> 9,pbmpHD[i].high>>9));
#endif         
        src += pbmpHD[i].len;
#ifdef DEF_81X
        if (PALETTED8 == pbmpHD[i].format || 
                PALETTED565 == pbmpHD[i].format || 
                PALETTED4444 == pbmpHD[i].format) {
            src += pbmpHD[i].len_lut;
        }
#endif
    }

    HAL_DlpBufIn(DISPLAY());
    HAL_BufToReg(RAM_DL,0);
}

appRet_en appBmpToRamG(FTU32 bmpHdl, FTU32 ramgAddr, bmpHDR_st *pbmpHD, FTU32 nums)
{
    if (nums > FT800_BMP_EXT_HANDLE || bmpHdl >= FT800_BMP_EXT_HANDLE) {
        FTPRINT("\nappBmpToRamG: items exceed");
        return APP_ERR_HDL_EXC;
    }

    if (APP_OK != appLoadBmp(ramgAddr,pbmpHD,nums) ) {
        return APP_ERR_LEN;
    }

    appUI_FillBmpDL(bmpHdl, ramgAddr, pbmpHD, nums);

    return APP_OK;
}

STATIC FTVOID appUI_GetEVEID (FTVOID)
{
    EVE_ID = HAL_Read8(EVE_ID_REG);
}

STATIC FTVOID appUI_SpiInit ( FTVOID )
{
    /* 
       the SPI clock shall not exceed 11MHz before system clock is enabled. 
       After system clock is properly enabled, 
       the SPI clock is allowed to go up to 30MHz.	
     */
#ifdef MSVC2010EXPRESS
    vc2010_spi_init();
#elif defined(MSVC2012EMU)
    //do nothing
#elif defined(STM32F4)
    stm32f4SPI1Init(SPI_BaudRatePrescaler_8);
#elif defined(FT9XXEV)
    /*spi sck = system clock/SPI_Div, 100MHz/16=6.25MHz*/
    ft9xx_spi_init(1,16);
#else
    /*spi sck = system clock/DIVx, 16MHz/2 = 8MHz*/
    SPI.begin();
    SPI.setClockDivider(SPI_CLOCK_DIV2);
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
#endif
}
STATIC FTVOID appUI_EVEPathCfg ( FTVOID )
{
#if defined(MSVC2010EXPRESS) || defined(MSVC2012EMU)
    //do nothing
#elif defined(STM32F4)
    //do nothing
#elif defined(FT9XXEV)
    ft9xx_init();

    ft9xx_sdc_init();
#else
    /* set the GPIO pin */
    pinMode(FT800_SPI_CS, OUTPUT);
    digitalWrite(FT800_SPI_CS, HIGH);

    pinMode(FT800_PD_N, OUTPUT);
    digitalWrite(FT800_PD_N, HIGH);

    arduino_sdcardInit();
#endif

    HAL_McuCmdBufInit();

    appUI_SpiInit();
}
STATIC FTVOID appUI_EVEActive ( FTVOID )
{
    HAL_Cfg(FT_GPU_ACTIVE_M);
#ifdef DEF_81X
    /* 81X need more action for the active */
    HAL_Cfg(FT_GPU_ACTIVE_M);
#endif
    FTDELAY(50);
}

STATIC FTVOID appUI_EVEClk ( FTVOID )
{
    FTPRINT("\nOSC: ");
#if defined(TRIM_NEEDED)
    FTPRINT("internal");
#else
    FTPRINT("external");
    /* Set the clk to external clock */
    HAL_Cfg(FT_GPU_EXTERNAL_OSC);  
    FTDELAY(CLK_DELAY);
#endif
#ifndef DEF_81X
    /* default 48MHz, no need to config
    HAL_Cfg(FT_GPU_PLL_48M);  
    FTDELAY(CLK_DELAY);
    */
#endif
}

#define PWC_DELAY 20
STATIC FTVOID appUI_EVEPwdCyc ( FTU8 OnOff )
{
#ifdef MSVC2010EXPRESS
    FT_WriteGPIO(ftHandle, 0xBB, OnOff?0x08:0x88);
    FTDELAY(PWC_DELAY);

    FT_WriteGPIO(ftHandle, 0xBB, OnOff?0x88:0x08);
    FTDELAY(PWC_DELAY);
#elif defined(MSVC2012EMU)
    /* Do nothing */
#elif defined(STM32F4)
    /* Share the PD13/LED3 pin with PD pin of EVE */
    STM_EVAL_LEDOff(LED3);
    FTDELAY(PWC_DELAY);

    STM_EVAL_LEDOn(LED3);
    FTDELAY(PWC_DELAY);
#elif defined(FT9XXEV)
    gpio_write(FT9XX_PD, OnOff?0:1);
    FTDELAY(PWC_DELAY);

    gpio_write(FT9XX_PD, OnOff?1:0);
    FTDELAY(PWC_DELAY);
#else
    digitalWrite(FT800_PD_N, OnOff?LOW:HIGH);
    FTDELAY(PWC_DELAY);

    digitalWrite(FT800_PD_N, OnOff?HIGH:LOW);
    FTDELAY(PWC_DELAY);
    /* 
     * I was not quite understand why a read action is needed
     * but for VM801Plus module, it seems like a critical action 
     */
    HAL_Read16(0);
#endif

}

STATIC FTVOID appUI_EVEGPIOCfg ( FTVOID )
{
    /* set DISP to output, then enable the DISP */
#if defined(DEF_81X)
    /*
       Bit 31-16: Reserved
       Bit 15 : Controlling the direction of pin DISP. For DISP functionality, this bit
       shall be kept intact.
       Bit 14-4: Reserved
       Bit 3-0: Controlling the direction of pin GPIO 3-0. (1 = output, 0 = input)
       For FT810/811, only GPIO 1-0 are available. For FT812/813, GPIO 3-0 are
       available.
     */
    HAL_Write16(REG_GPIOX_DIR,0x800F);
    /*
       Bit 31-16: Reserved
       Bit 15 : Setting or reading the level of pin DISP. 1 for high and 0 for low
       Bit 14-13:GPIO[3:0], TOUCHWAKE Drive Strength Setting
       (00:5mA - default, 01:10mA, 10:15mA, 11:20mA)
       Bit 12:PCLK, DISP, V/HSYNC, DE, R,G,B, BACKLIGHT Drive Strength Setting
       (0:5mA - default, 1:10mA)
       Bit 11 - 10:MISO, MOSI, IO2, IO3, INT_N Drive Strength Setting
       (00:5mA - default, 01:10mA, 10:15mA, 11:20mA)
       Bit 9: INT_N Type
       (0 : OD - default, 1 : Push-pull)
       Bit 8-4: Reserved
       Bit 3-0: Writing or reading the pin of GPIO 3-0. 1 for high and 0 for low.
       For FT810/811, only GPIO 1-0 are available. For FT812/813, GPIO 3-0 are available.
     */
    HAL_Write16(REG_GPIOX,0x800F);
#else
    /*
       Bit 0 - 7 : These bits configure the direction of GPIO pins of the FT800. Bit 0 controls
       the direction of GPIO0 and Bit 7 controls the direction of GPIO7. The bit value 1
       means the GPIO pin is set as an output, otherwise it means an input. After reset, only
       the GPIO7 is set to output by default.
     */
    HAL_Write8(REG_GPIO_DIR,0xFF);
    /*
       Bit 0 - 7 : These bits are versatile. Bit 0 , 1, 7 are used to control GPIO pin values.
       Bit 2 - 6 : These are used to configure the drive strength of the pins.
     */
    HAL_Write8(REG_GPIO,0xFF);
#endif

    /* mute sound to avoid pop sound */
    HAL_Write16(REG_SOUND,0x0060);
    HAL_Write8(REG_PLAY,0x01);
    while(HAL_Read8(REG_PLAY));
}

/*
 * Add the FT81X support, with the CTP controller
 * you may change the delay time base on your own HW and CTP
 */
STATIC FTVOID appUI_EVETchCfg ( FTVOID )
{
#if defined(DEF_CAP_NONMULTI) || defined(DEF_CAP_MULTI)
    FTDELAY(300);
    HAL_Write8(REG_CPURESET, 2);
    FTDELAY(300);
    HAL_Write16(REG_CYA_TOUCH,(HAL_Read16(REG_CYA_TOUCH) & 0x7fff));
    FTDELAY(300);
    HAL_Write8(REG_CPURESET, 0);
    FTDELAY(300);

#ifdef DEF_CAP_MULTI
    HAL_Write8(REG_CTOUCH_EXTENDED,CTOUCH_MODE_EXTENDED);
#endif

#else
    /*
     * for this code, I consider "resistance touch" + "WVGA" 
     * means using FTDI WH70R, it use default touch threshold: 
     * the lightest touch will be accepted
     * you may change it depends on your real system
     */
#if !defined(LCD_WVGA)
    HAL_Write16(REG_TOUCH_RZTHRESH,FT800_TOUCH_THRESHOLD);
#endif
    HAL_Write16(REG_CYA_TOUCH,(HAL_Read16(REG_CYA_TOUCH) | 0x8000));
#endif
}
STATIC FTVOID appUI_EVESetSPI (FTU32 type)
{
#ifdef DEF_81X
    if (type == 4) {
        HAL_Write8(REG_SPI_WIDTH, EVE_QSPI | FT800_SPI_DUMMY);
    } else if (type == 2) {
        HAL_Write8(REG_SPI_WIDTH, EVE_DSPI | FT800_SPI_DUMMY);
    } else {
        HAL_Write8(REG_SPI_WIDTH, EVE_SSPI);
    }
#endif

#if defined(FT9XXEV)
    /* use the highest speed of clk of SPI on MM900 module */
    if (type == 4) {
        ft9xx_spi_init(type,8);
    } else {
        ft9xx_spi_init(type,2);
    }
#endif
}
#define VER_FONT 25
#define CAL_FONT 20
#define CAL_WIDE 15
#define SYS_HANG (0)
STATIC FTVOID appUI_EVEBootupDisp ( FTU32 count )
{
    static FTU8 init = 0;
    if (init == 0) {
        FTPRINT("\nBootup info");
        init = 1;
    }
    do {
        HAL_CmdBufIn(CMD_DLSTART);
        HAL_CmdBufIn(CLEAR_COLOR_RGB(0,0,0));
        HAL_CmdBufIn(CLEAR(1,1,1));

        switch (EVE_ID) {
            case 0x10:
                CoCmd_TEXT(FT800_LCD_WIDTH/3,FT800_LCD_HIGH/4,
                        VER_FONT,OPT_CENTERX,"FT810");
                break;
            case 0x11:
                CoCmd_TEXT(FT800_LCD_WIDTH/3,FT800_LCD_HIGH/4,
                        VER_FONT,OPT_CENTERX,"FT811");
                break;
            case 0x12:
                CoCmd_TEXT(FT800_LCD_WIDTH/3,FT800_LCD_HIGH/4,
                        VER_FONT,OPT_CENTERX,"FT812");
                break;
            case 0x13:
                CoCmd_TEXT(FT800_LCD_WIDTH/3,FT800_LCD_HIGH/4,
                        VER_FONT,OPT_CENTERX,"FT813");
                break;
            default:
                /* only new FT81X able to read the EVE ID */
                CoCmd_TEXT(FT800_LCD_WIDTH/3,FT800_LCD_HIGH/4,
                        VER_FONT,OPT_CENTERX,"EVE Chip");
                break;
        }
#if defined(DEF_CAP_MULTI)
        CoCmd_TEXT(FT800_LCD_WIDTH/3*2,FT800_LCD_HIGH/4,
                VER_FONT,OPT_CENTERX,"CAP-M");
#elif defined(DEF_CAP_NONMULTI)
        CoCmd_TEXT(FT800_LCD_WIDTH/3*2,FT800_LCD_HIGH/4,
                VER_FONT,OPT_CENTERX,"CAP-NM");
#else
        CoCmd_TEXT(FT800_LCD_WIDTH/3*2,FT800_LCD_HIGH/4,
                VER_FONT,OPT_CENTERX,"RES");
#endif
        CoCmd_TEXT(FT800_LCD_WIDTH/2,FT800_LCD_HIGH/2,
                VER_FONT,OPT_CENTERX,"Ver: "APPS_VER);
        CoCmd_TEXT(FT800_LCD_WIDTH/2-10,FT800_LCD_HIGH/4*3,
                CAL_FONT,OPT_CENTERX,"MCU CMD Buf: ");
        CoCmd_NUMBER(FT800_LCD_WIDTH/2+40,FT800_LCD_HIGH/4*3,
                CAL_FONT,OPT_CENTERX,HAL_CmdBufSize());
#if defined(STM32F4)
        CoCmd_TEXT(FT800_LCD_WIDTH/2-30,FT800_LCD_HIGH/4*3+10,
                CAL_FONT,OPT_CENTERX,"File addr: ");
        CoCmd_NUMBER(FT800_LCD_WIDTH/2+40,FT800_LCD_HIGH/4*3+10,
                CAL_FONT,OPT_CENTERX,FILE_SADDR);
#endif
        if (SYS_HANG) {
            CoCmd_TEXT(FT800_LCD_WIDTH/2,FT800_LCD_HIGH - CAL_WIDE*2,
                    CAL_FONT,OPT_CENTERX,"system hange due to memory limit!");
        } else {
            CoCmd_TEXT(FT800_LCD_WIDTH/2,FT800_LCD_HIGH - CAL_WIDE*2,
                    CAL_FONT,OPT_CENTERX,"press and hold to force in calibration");
            CoCmd_NUMBER(FT800_LCD_WIDTH/2,FT800_LCD_HIGH - CAL_WIDE,
                    CAL_FONT,OPT_CENTERX,count);
        }

        HAL_CmdBufIn(DISPLAY());
        HAL_CmdBufIn(CMD_SWAP);
        HAL_BufToReg(RAM_CMD,0);
    } while (SYS_HANG);
}
#if defined(TRIM_NEEDED)
STATIC FTU32 appUI_EVEGetFrq (FTVOID)
{
    FTU32 t0, t1;
    FT32 r = 15625;

    t0 = HAL_Read32(REG_CLOCK); /* t0 read */
#if defined(FT9XXEV)
    __asm__
        (
         "   move.l  $r0,%0"
         "\n\t"
         "   mul.l   $r0,$r0,100"
         "\n\t"
         "1:"
         "\n\t"
         "   sub.l   $r0,$r0,3"
         "\n\t" /* Subtract the loop time = 4 cycles */
         "   cmp.l   $r0,0"
         "\n\t" /* Check that the counter is equal to 0 */
         "   jmpc    gt, 1b"
         "\n\t"
         /* Outputs */ :
         /* Inputs */  : "r"(r)
         /* Using */   : "$r0"
        );
#elif defined(STM32F4) || defined(MSVC2012EMU) || defined(MSVC2010EXPRESS)
    FTDELAY(r/1000);
#else
    delayMicroseconds(r);
#endif
    t1 = HAL_Read32(REG_CLOCK); /* t1 read */
    /* bitshift 6 places is the same as multiplying 64 */
    return ((t1 - t0) * 64); 
}
#endif
STATIC FTVOID appUI_EVEClkTrim ( FTVOID )
{
#if defined(TRIM_NEEDED)
#define LOW_FREQ_BOUND  58800000L//98% of 60Mhz
    FTU32 f;
    FTU8 i;
    /* Trim the internal clock by increase the REG_TRIM register
       till the measured frequency is within the acceptable range.*/
    for (i=0; (i < 31) && ((f= appUI_EVEGetFrq()) < LOW_FREQ_BOUND); i++)
    {
        /* increase the REG_TRIM register value 
           automatically increases the internal clock */
        HAL_Write8(REG_TRIM, i);  
    }
    /* Set the final frequency to be used for internal operations */
    HAL_Write32(REG_FREQUENCY,f);  
#endif
}
STATIC FTVOID appUI_EVELCDCfg ( FTVOID )
{
    /*
       Width Height 
       HCycle HOffset HSync0 HSync1 
       VCycle VOffset VSync0 VSync1 
       PCLK Swizzle PCLKPol Cspread Dither
     */
    FT800_LCD lcd = {FT800_LCD_WIDTH,FT800_LCD_HIGH, 
#if defined(LCD_WVGA)
        /* PCLK is critical,
           sometime it may need to set to larger number (2,3)
           some smaller number (set to 1), may case under run issue 
           while too much commands needs to be executed*/
        /* 
        this setting seems better than right side
        in real pratice of AllyTech project
         */
        1058,40,0,20,//928,88,0,48, 
        525,25,0,10,//525,32,0,3, 
        2,0,1,0,1};
#elif defined(LCD_QVGA)
        408,70,0,10, 
        263,13,0, 2, 
        8,2,0,1,1};
#elif defined(LCD_HVGA)
        400,40,0,10, 
        500,10,0, 5, 
        //4,2,1,1,1};
        5,2,1,1,1}; //for ME810A_HV35R pclk is 5
#elif defined(LCD_WQVGA)
        548,43,0,41, 
        292,12,0,10, 
        5,0,1,1,1};
#else
#error "undefined LCD"
#endif

    /* config the LCD related parameters */
    HAL_Write16(REG_HSIZE, lcd.Width);
    HAL_Write16(REG_VSIZE, lcd.Height);
    HAL_Write16(REG_HCYCLE, lcd.HCycle);
    HAL_Write16(REG_HOFFSET, lcd.HOffset);
    HAL_Write16(REG_HSYNC0, lcd.HSync0);
    HAL_Write16(REG_HSYNC1, lcd.HSync1);
    HAL_Write16(REG_VCYCLE, lcd.VCycle);
    HAL_Write16(REG_VOFFSET, lcd.VOffset);
    HAL_Write16(REG_VSYNC0, lcd.VSync0);
    HAL_Write16(REG_VSYNC1, lcd.VSync1);
    HAL_Write8(REG_SWIZZLE, lcd.Swizzle);
    HAL_Write8(REG_PCLK_POL, lcd.PCLKPol);
    HAL_Write16(REG_CSPREAD, lcd.Cspread);
    HAL_Write16(REG_DITHER, lcd.Dither);

    /* the backlight default is highest 
    HAL_Write8(REG_PWM_DUTY,128);
    */

    /* the RGB output default is 6x6x6 
        R:8,7,6
        G:5,4,3
        B:2,1,0
        default: 6x6x6
        0: 8x8x8
    HAL_Write32(REG_OUTBITS,0);
     */

#if defined(LCD_HVGA) && defined(FT9XXEV)
    /*
     * spi sck = system clock/SPI_Div, 100MHz/256=390KHz
     * ILI9488 looks like need 500KHz, 100MHz/128=781KHz, 
     * for FT900 no 200 can be selected 
     */
    ft9xx_spi_init(1,128);
    ft9xx_init_ili9488();
#endif
    /* start to display */
    HAL_Write8(REG_PCLK,lcd.PCLK);
}

STATIC FTU8 appUI_EVEVerify (FTVOID)
{
    FTU8 count = RETRY_COUNT;

    FTPRINT("\nRead ID: ");
    while (HAL_Read8(REG_ID) != FT800_ID && count) {
        FTPRINT(".");
        count--;
        FTDELAY(READ_ID_WAIT);
    }

    if (count) {
        FTPRINT("Done");
        /* for FT81X, users are recommended to 
           read 4 bytes data from address 0xC0000 
           before application overwrites this address,
           since it is located in RAM_G. */
        appUI_GetEVEID();
    }
    return count;
}

STATIC FTVOID appUI_EVEClnScrn (FTVOID)
{
    HAL_CmdBufIn(CMD_DLSTART);
    HAL_CmdBufIn(CLEAR_COLOR_RGB(0,0,0));
    HAL_CmdBufIn(CLEAR(1,1,1));
    HAL_CmdBufIn(DISPLAY());
    HAL_CmdBufIn(CMD_SWAP);
    HAL_BufToReg(RAM_CMD,0);
}

STATIC appRet_en appUI_WaitCal (FTVOID)
{
#if defined(CAL_NEEDED)
    FTU8 i = 0;
    appRet_en ret;

    /* Cal would judge whether it should do the calibration or not  */
    do {
        /* keep doing the calibration until it success */
        ret = appCal(i,CDATA_PATH);
    } while (ret != APP_OK);

    /* wait for a while, 
     * let user decide if need to force calibration
     * or see some debug information */
    i = FORCE_WAIT_COUNT;
    while (!TOUCHED) {
        appUI_EVEBootupDisp(i);

        FTDELAY(FORCE_WAIT_TIME);
        if (0 == --i) {
            break;
        }
    }

    /* deciede to do the calibration or not depend on user's input */
    if (i) {
        /* into the calibration, after press released */
        while (TOUCHED) {
            FTDELAY(FORCE_WAIT_TIME);
        }
        do {
            /* keep doing the calibration until it success */
            ret = appCal(i,CDATA_PATH);
        } while (ret != APP_OK);
    }
#endif
    appUI_EVEClnScrn();
    return APP_OK; 
}
FTVOID appUI_DbgPrint (FTC8 *p_fname, FTU32 fline)
{
    sprintf((char *)dbg_str_buf,"%s:%d",p_fname,(int)fline);
}
FTVOID UI_INIT (FTVOID)
{
    FTPRINT("\nEVE init");
    
    appUI_EVEPathCfg();

    /* in some very old BASIC board, 
     * the input should be 0,
     * but all the offical board outside should be 1
     * so leave it 1 */
    appUI_EVEPwdCyc(1);

    appUI_EVEActive();

    appUI_EVEClk();

    if (!appUI_EVEVerify()) {
        return;
    }
    
    FTPRINT("\nDisplay init");
    /* 
     * After recognizing the type of chip, 
     * perform the trimming if necessary 
     */
    appUI_EVEClkTrim();

    appUI_EVEGPIOCfg();

    appUI_EVETchCfg();
    /* clear the screen before enable the LCD
       to avoid messy info on LCD during bootup */
    appUI_EVEClnScrn();

    appUI_EVELCDCfg();

    /* after use single SPI to config the EVE
       set the SPI base on real HW: SPI/DSPI/QSPI */
    appUI_EVESetSPI(EVE_SPI_TYPE); 

    appUI_WaitCal();
   
    /* give a default string when DBG_PRINT not enable */
    strcpy((char *)dbg_str_buf,"Display end by user, or error happen!");
}
/*
 * Only when error happen
 * or user intend to end the display
 * process would reach here
 */
FTVOID UI_END (FTVOID)
{
    HAL_CmdBufIn(CMD_DLSTART);
    HAL_CmdBufIn(CLEAR_COLOR_RGB(0xFF,0,0));
    HAL_CmdBufIn(CLEAR(1,1,1));
    CoCmd_TEXT(FT800_LCD_WIDTH/2,FT800_LCD_HIGH/2,24,
               OPT_CENTER,dbg_str_buf);
    HAL_CmdBufIn(DISPLAY());
    HAL_CmdBufIn(CMD_SWAP);
    HAL_BufToReg(RAM_CMD,0);
}
