#ifndef MK1MF_BUILD
    /* auto-generated by util/mkbuildinf.pl for crypto/cversion.c */
    #define CFLAGS cflags
    /*
     * Generate CFLAGS as an array of individual characters. This is a
     * workaround for the situation where CFLAGS gets too long for a C90 string
     * literal
     */
    static const char cflags[] = {
        'c','o','m','p','i','l','e','r',':',' ','a','r','m','-','h','i','m','i',
        'x','1','0','0','-','l','i','n','u','x','-','g','c','c',' ','-','I','.',
        ' ','-','I','.','.',' ','-','I','.','.','/','i','n','c','l','u','d','e',
        ' ',' ','-','O',' ','-','D','_','_','L','I','T','E','O','S','_','_',' ',
        '-','D','L','_','E','N','D','I','A','N',' ','-','O','3',' ','-','W','a',
        'l','l','\0'
    };
    #define PLATFORM "platform: linux-x86"
    #define DATE "built on: Wed May 15 11:27:03 2019"
#endif