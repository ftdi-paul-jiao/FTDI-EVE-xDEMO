-1. Everything here is ONLY for learning EVE's purpose, I DO NOT have
    any responsibility to any one or any organization, to their usage
	on any peices of my codes. I'll delete related parts, if any meterials
	have any license violate, just let me know.

0. The whole project was made and tested under the Win7 platform
   so the compile, debug, program all Windows based.

1. Execute "RunMe.bat" to build the related image, 
   and please DO read the words it gives out.
1.1 DO change the define in FT800_platform.h depend on your real system
    e.g. LCD resolution: '320x240', '800x480', EVE version: '80X', '81X', etc.
1.2 DO change the define in FT800_demo.h depend on what demo you would like to 
    play. For example, active DEMO_AVI and comment others, and make sure you know
    the code limits while playing DEMO_AVI: it only works on DEF_81X 
    under the LCD_WVGA resolution.

2. All the code are under the root directry of FT800, 
   others like "STM32F4", "VC2010Express", etc. 
   is compile platform related, you can ignore them.

3. This series of software is developed base on the learning from SampleApp, 
   SampleApp would have full supported function, 
   and may relatively easy be supported by R&D 
   (since SampleApp was developed by them, while this series 
   is developed by FAE).

4. You may think it's strange that why use BATCH file 
   to start related project, it's a long store, if you are interested in, 
   go doc/ReadMe_xDEMO.doc for more.

5. You may also think it's uncomfortable why not manage the related platform 
   in related directory ('src', 'inc', etc.) but hide them all in FT800, 
   still doc/ReadMe_xDEMO.doc would tell you more.

-1. ���в��Ͻ���ѧϰEVEʹ�ã��Ҳ����κ�������߸��ˣ��ο������κ�һ�δ��뵽�Լ���Ʒ��
    �������κ����⣬���κ����Ρ�����ʹ�õ��κ��زģ�������Ȩ����֪�Һ������ɾ��

0. ���������ǹ�����Win7�����ϣ����룬���ԣ���¼�ȹ��߶��ǻ���Windows

1. ���뻷�������С�����.bat���򿪣������������Ķ��������֡�
1.1 һ���ǵø�����ʵ��ϵͳ�޸�FT800_platform.h
    ����LCD ����: '320x240', '800x480', EVE оƬ�汾: '80X', '81X', etc.
1.2 һ���ǵø�����Ҫ���е�Demo�޸�FT800_demo.h
    ���缤��DEMO_AVIע�͵������ģ�����֪������DEMO_AVI������: ֻ֧��81XоƬ��
    ֻ֧��LCD_WVGA��Ļ

2. ���붼��FT800�ļ��и�Ŀ¼�£�������Ŀ¼��STM32F4������VC2010Express�����ȵȣ�
   ���涼��ƽ̨��ش��룬���������޹ء�

3. ���ƽ̨�ǻ��ڹ�˾����������SampleApp�޸Ķ��ɣ��󲿷�Ҳ�ο���SampleApp����Ҫ��˼
   ���˸о�ԭ����Щ���ҺͲ�̫�ö�������˽��д�����ϵ��
   �˸��кã�SampleApp������з�д�ģ������ϲ���Ǹ���񣬲ο��Ǳ�Ҳ�Ǽ��õġ�

4. �����֪��Ϊʲôʹ�����������Ӧ����ƽ̨�����Բο�doc/ReadMe_xDEMO.doc��
   ������������˵�������

5. Ҳ���ֳ���Աϲ��('src', 'inc', etc.)�����Ľṹ���ļ����÷�ʽ����Ҳϲ����
   �����ｫ���ж���������FT800����ļ����ͬ�����п��Եģ�
   ���廹�ǿ��Բο�doc/ReadMe_xDEMO.doc

6. ��˾���е��ĵ��������϶�Ӧ����Ӣ�ģ����ԣ���������ļ�����������Ӣ�ģ�
   ���в��㣬ʵ�ڶԲ�ס�ˡ�
