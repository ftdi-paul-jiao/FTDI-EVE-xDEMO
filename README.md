a. Everything here is ONLY for learning EVE's purpose, I DO NOT have
   any responsibility to any one or any organization, to their usage
   on any peices of my codes. I'll delete related parts, if any meterials
   have any license violate, just let me know.

b. The whole project structure may looks ugly because I was trying to merge
   all those platform in single project under "the more sharing code the better"
   requirement.

c. Execute "RunMe.bat" to build the related image, 
   and please DO read the words it gives out:
   
d. DO change the define in platform.h depend on your real system
    e.g. LCD resolution: '320x240', '800x480', EVE version: '80X', '81X', etc.
    
e. DO change the define in UI.h depend on what demo you would like to 
    play. For example, active UI_AVI and comment others, and make sure you know
    the code limits while playing UI_AVI: it only works on DEF_81X 
    under the LCD_WVGA resolution.

f. All the EVE code are sys_main\UI, others like "STM32F4", "VC2010Express", etc. 
   is platform related.

g. This series of software is developed base on the learning from SampleApp, 
   SampleApp would have full supported function, 
   and may relatively easy be supported by R&D 
   (since SampleApp was developed by them, while this series 
   is developed by FAE).

1. 所有材料仅做学习EVE使用,我不对任何团体或者个人,参考这里任何一段代码到自己产品里
   产生的任何问题,负任何责任.里面使用到任何素材,如有侵权,告知我后会立刻删除

2. 整个工程结构可能看起来很恶心,但这是基于为了将多款编译平台合并一起,
   能越多共享部分越好的要求

3. 编译环境请运行“编译.bat”打开,并根据需求阅读里面文字:

4. 一定记得根据你实际系统修改platform.h
    比如LCD 像素: '320x240', '800x480', EVE 芯片版本: '80X', '81X', etc.

5. 一定记得根据你要运行的Demo修改UI.h
    比如激活UI_AVI注释掉其它的，并且知道运行UI_AVI的限制: 只支持81X芯片，
    只支持LCD_WVGA屏幕

6. 代码都在sys_main\UI目录下,其他都是平台相关代码.

7. 这个平台是基于公司官网发布的SampleApp修改而成,大部分也参考了SampleApp的主要意思
   个人感觉原版有些零乱和不太好懂,于是私下写了这个系列
   人各有好,SampleApp是相关研发写的,如果更喜欢那个风格,参考那边也是极好的.

8. 公司所有的文档,理论上都应该是英文,所以,除了这个文件,其他都是英文,
   如有不便,实在对不住了.
