/* 
    Applicaton, Integrated function for EVE
    Author: Hawk
    Date  : 2013/Oct
 */

#ifndef _EVE_APP_H_
#define _EVE_APP_H_



/* Attention, so far as I know, Arduino platform may force the number to 0
 * if you define the number larger than system allow
 * e.g. in Mege328 system (no external memory) MAX should not larger than 32K
 * UL define may be used when you have to use a number larger than 32K */
#if defined(DEF_81X) || defined(DEF_BT81X)
#define EVE_RAMG_SIZE       (1*1024*1024UL)
#else
#define EVE_RAMG_SIZE       (256*1024UL)
#endif
/* 0~15, 15 can only be used when no GRADIENT, BUTTON, KEYS be used 
 * at current display list */
#define EVE_BMP_EXT_HANDLE  (16) 
#define EVE_APP_ERR_CODE    (0xFF00UL) 

#define EVE_PIC_MEM_ADDR    RAM_G
#define EVE_FONT_INTERNAL   (16)

#define ZLIB_LEN              (0xFFFFFFFF)
#define ZFLH_LEN              (0xFFFFFFFE)
#define FLH_LEN               (0xFFFFFFFC)
#if defined(DEF_BT81X)
#define EVE_DBG_BUF_LEN       (128)
#else
#define EVE_DBG_BUF_LEN       (50)
#endif
typedef struct bmpHDR_ {
    FTC8  *path;
    FTC8  *path_lut; //for palette only
    FTU32 lut_src;   //for palette only
    FTU32 format;
    FTU32 len;
    FTU32 len_lut;   //for palette only
    FTU32 wide;
    FTU32 high;
}bmpHDR_st;
typedef struct audio_hd_st {
    FTU8 * path;
    FTU32 index;
    FTU32 dsize;
    FTU32 type;
    FTU16 freq;
    FTU8 vol;
}audio_hd_t;
typedef enum appRet_ {
    APP_OK          = 0,
    APP_ERR_CALC    = EVE_APP_ERR_CODE,
    APP_ERR_NULL,
    APP_ERR_OPEN,
    APP_ERR_HDL_EXC,
    APP_ERR_LEN,
} appRet_en;

typedef FTVOID (* AppFunc) (FTU32);

FTU8 appFileExist (FTC8 *path);
FTU32 appFileToRamG (FTC8 *path, FTU32 inAddr, FTU8 chkExceed, FTU8 *outAddr, FTU32 outLen);
appRet_en appWaitCal (FTVOID);
FTVOID appUI_FillBmpDL(FTU32 bmpHdl, FTU32 ramgAddr, bmpHDR_st *pbmpHD, FTU32 nums);
appRet_en appLoadBmp(FTU32 ramgAddr, bmpHDR_st *pbmpHD, FTU32 nums);
appRet_en appBmpToRamG(FTU32 bmpHdl, FTU32 ramgAddr, bmpHDR_st *pbmpHD, FTU32 nums);
FTU32 appGetLinestride(bmpHDR_st bmpHD);
FTVOID appUI_DbgPrint (FTC8 *p_fname, FTU32 fline);
FTU32 appEveCRC(FTU32 eve_addr, FTU32 len);
FTVOID appEveZERO(FTU32 eve_addr, FTU32 len);
FTU8 appFlashPath (FTC8 *path, FTU32 *len);
FTVOID SegmentOperation (FTU32 handle, FTU32 src, FTU32 des, FTU32 len, FTU8 toCoPro);

#if defined(DEF_BT81X)
FTVOID resWrFlash (FTU32 para);
FTU8 appFlashSetFull(FTVOID);
FTVOID appFlashErase(FTVOID);
FTU8 appFlashToEVE(FTU32 flash_addr, FTU32 eve_addr, FTU32 len);
FTU32 appFlashVerify(FTU8 *golden_file, FTU32 flash_addr);
FTU32 appFlashProg(FTU8 *f_file, FTU32 f_addr);
FTU32 appFlashAddr(FTC8 *path);
FTVOID appFlashUnzip(FTC8 *path, FTU32 src);
#endif

FTVOID UI_INIT (FTVOID);
FTVOID UI_END (FTVOID);

#endif
