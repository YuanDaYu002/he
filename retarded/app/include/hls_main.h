#ifndef HLS_MAIN_H
#define HLS_MAIN_H

/*---#切片后的TS文件描述信息------------------------------------------------------------*/
typedef struct _out_ts_info_t
{
	int 		ts_num;				//TS文件个数（mp4文件切片后的TS文件）
	char		ts_name[10][32];	//TS文件名（按切片的先后顺序排列，带绝对路径）
	char		m3u_name[32];		//m3u文件名（带绝对路径）
}out_ts_info_t;

/*---#为了支持直接传入文件缓存，liteos 的ramfs文件系统在写入大文件时很慢---------------*/
typedef struct _mp4_FILE_info_t
{
	char 		file_name[32];		//MP4文件名
	/*---使用缓存buf直接传入的方式（不使用请置空）-------------------------------------*/
	char*		file_buf;			//文件buf起始位置	
	int			file_size;			//文件大小
}mp4_FILE_info_t;

out_ts_info_t* hls_main (mp4_FILE_info_t* mp4_file);
void hls_main_exit(out_ts_info_t* ts_info);





#endif

