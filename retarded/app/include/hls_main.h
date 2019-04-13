#ifndef HLS_MAIN_H
#define HLS_MAIN_H


/*---#切片后的TS文件描述信息------------------------------------------------------------*/	
typedef struct _ts_info_t
{
	char 	ts_name[32];	//ts文件名字（带绝对路径）
	char*	ts_buf;			//ts文件的数据缓存（退出后需free）
	int		ts_buf_size;	//ts文件的大小（就是返回的buf大小）
}ts_info_t;

#define MAX_TS_NUM  10  //最大生成的TS文件个数
typedef struct _hls_out_info_t
{
	int 		hls_mode;			 /*hls的模式：
										 1:文件模式（该模式下的文件名对应的文件真实存在，对应的buf应为NULL）;
										 2:内存模式（该模式下的文件名对应的文件都不是真实存在,实际数据存储在对应的buf中）。
									 */
	int 		ts_num;				 //实际TS文件总数，不能超过 MAX_TS_NUM
	ts_info_t	ts_array[MAX_TS_NUM];//每个TS文件描述信息（按切片的先后顺序排列）
	char		m3u_name[32];		 //m3u文件名（带绝对路径）
	char*		m3u_buf;			 //m3u文件的数据缓存
	int 		m3u_buf_size;		 //m3u文件的大小（就是返回的buf大小）
}hls_out_info_t;

/*---#为了支持直接传入文件缓存，liteos 的ramfs文件系统在写入大文件时很慢---------------*/
typedef struct _FILE_info_t
{
	int 		segment_duration;	//每段ts文件的切片时长
 	/*---使用文件名直接传入的方式（不使用请置空）-------------------------------------*/
	char 		file_name[32];		//MP4文件名（带绝对路径）
	/*---使用缓存buf直接传入的方式（不使用请置空）-------------------------------------*/
	char*		file_buf;			//文件buf起始位置	
	int			file_size;			//文件大小
}FILE_info_t;



hls_out_info_t* hls_main (FILE_info_t* mp4_file);
void hls_main_exit(hls_out_info_t* hls_info);
void hls_main_global_variable_reset(void);




#endif






