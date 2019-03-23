#define TS_FLAG_JPG		1
#define TS_FLAG_6S		11
#define TS_FLAG_6S_ONLY	20
#define TS_FLAG_START	21
#define TS_FLAG_MID		22
#define TS_FLAG_END		23
#define TS_FLAG_ONE		24//only one ts

#define TYPE_JPG 0
#define TYPE_MKV 1
#define TYPE_TS  2
#define TYPE_M3U 3

#define FRAND_LEN 20

typedef struct
{
	int today_req;	//  0/1
	int record_en;	//  0 ok, 1 1-only pic  -1  nothing
	char date[12];
	char aws_access_keyid[24];
	char secret_aceess_keyid[44];
	char host[128];
	char record_name[128];//   /tmp/record
	char jpg_filename[128];//   /tmp/record
	char deviceid[64];
	char bucket[128]; //文件存储桶
	char regon[32];
}AMAZON_INFO;


typedef struct
{
	int offset;//ffmpeg---offset
	int record_oder;  //de
	int m3u8_oder; //
	char ts_flag; //-1 jpg; 0 6s ; 1 start;  2 mid   3 end
	char ts_tlen; //use int write_m3u8
	char datetime[24];// 0T9I35TDdAKxg
	char jpg[64];//  /tmp/0T9I35TDdAKxg_021413.jpg
	char m3u8_6s[64];
	char m3u8name[64]; //  /tmp/0T9I35TDdAKxg_021413.m3u8
	char file_name[64];// /tmp/0T9I35TDdAKxg_021413.mkv
}ts_info;

void amazon_put_even_thread(ts_info *ts);
void ts_info_set(ts_info *ts, char flag, int tlen, char *fname);


