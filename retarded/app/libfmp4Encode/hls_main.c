
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include "curl/curl.h"


#include "hls_file.h"
#include "hls_media.h"
#include "hls_mux.h"
#include "mod_conf.h"
#include "typeport.h"
#include "hls_main.h"


//#include "lame/lame.h"


#define SERVER_TEST			//服务端测试开关

#define INPUT_MP4_FILE      "/jffs0/fmp4.mp4"   	//输入的MP4源文件

//m3u8 param
#define TS_FILE_PREFIX      "ZWG_TEST"              //切片文件的前缀(ts文件)
#define M3U8_FILE_NAME      "ZWG_TEST"         //生成的m3u8文件名
#define URL_PREFIX          "/jffs0/"               //生成目录
//#define MAX_NUM_SEGMENTS    50                      //最大允许存储多少个分片
#define SEGMENT_DURATION    15                       //每个分片文件的裁剪时长  


/*获取文件的名字，仅仅只要名字，不要路径*/
static char* get_pure_filename(char* filename)
{
    int len = strlen(filename);
    int pos = len - 1;
    while (pos >= 0 && filename[pos]!='/')
    	--pos;
    return &filename[pos + 1];
}

/*获取文件的名字，仅仅只要名字，不要路径,不要后缀*/
char mp4_name[64] = {0};
static char* get_pure_filename_without_postfix(char* filename)
{
    int len = strlen(filename);
    int pos = len - 1;
    while (pos >= 0 && filename[pos]!='/')
    	--pos;

	strcpy(mp4_name,&filename[pos + 1]);//+1为了去掉 ‘/’
	DEBUG_LOG("mp4_name = %s\n",mp4_name);
	
	len = strlen(mp4_name);
	int i = 0;
	for(i = 0 ; i<=len ; i++)
	{
		if(mp4_name[i] == '.')
		{
			mp4_name[i] = '\0'; //从 '.'处截断
			break;
		}
	}

	DEBUG_LOG("after cut , mp4_name = %s\n",mp4_name);
	
    return mp4_name;
}


/*
获取文件纯路径 /ramfs/aaa.m3u8--->  /ramfs/
注意：使用结束后需要释放 返回值
*/
char* get_pure_pathname(char* filename)
{
    int len = strlen(filename);
    int pos2 = len;
    while (pos2 >= 0 && filename[pos2]!='/') 
	{
    	pos2--;
    }
    char* str=(char*)malloc(sizeof(char)*(pos2+1));
	int bbb=0;
    for(bbb=0; bbb<=pos2; bbb++)
    	str[bbb]=filename[bbb];
    str[pos2+1]=0;
    return str;
}

/*******************************************************************************
*@ Description    :构造并生成 m3u8文件,计算可以生成TS切片文件的个数
*@ Input          :<hsl_out_info>: 切片后的最终结果输出（这里需要填充m3u8文件信息）
					<mp4_file>：mp4文件描述信息
					<playlist>：输出m3U8文件,播放列表要存储的位置（绝对路径）
*@ Output         :<numberofchunks>：TS分片文件的个数
*@ Return         :成功：0 ; 失败: -1
*@ attention      :
*******************************************************************************/
int  generate_playlist_test(hls_out_info_t* hsl_out_info,FILE_info_t* mp4_file, char* playlist, int* numberofchunks)
{
	media_handler_t* 	media;			//媒体操作句柄（系列函数）
	file_source_t*   	source;			//文件操作句柄（系列函数）
	file_handle_t* 		handle;			//文件句柄（要进行TS切片的文件）
	media_stats_t* 		stats;
	int 				piece;
	int 				stats_size;
	char* 				stats_buffer;
	int 				source_size;
	char*				pure_filename;
	FILE* f;
	
	char* filename = mp4_file->file_name;
	
	//---获取媒体操作的函数句柄-----------------------------
	media  = get_media_handler(filename);
	if ( !media )
	{
		ERROR_LOG("get_media_handler faile !\n");
		return -1;
	}
		

	//---获取文件操作的函数句柄-----------------------------
	source_size = get_file_source(NULL, filename, NULL, 0);//return sizeof(file_source_t);
	source 	= (file_source_t*)malloc(source_size);
	if ( !source )
	{
		ERROR_LOG("malloc failed!\n");
		return -1;
	}
		

	source_size  = get_file_source(NULL, filename, source, source_size);
	if ( source_size <= 0 )
	{
		ERROR_LOG("get_media_handler faile !\n");
		goto ERR;
	}
	
	//---打开mp4文件--------------------------------------------
	handle 	= (char*)malloc(source->handler_size);
	if ( !handle )
	{
		ERROR_LOG("malloc failed!\n");
		goto ERR;
	}

	if ( !source->open(source, handle, filename, FIRST_ACCESS) )
	{
		ERROR_LOG("open file failed !\n");
		goto ERR;
	}
	
	//---获取文件的状态信息--------------------------------------------
	stats_size 			= media->get_media_stats(mp4_file,NULL, handle, source, NULL, 0);
	stats_buffer		= (char*)malloc(stats_size);
	if ( !stats_buffer )
	{
		source->close(handle, 0);
		ERROR_LOG("malloc failed!\n");
		goto ERR;
	}
	memset(stats_buffer,0,stats_size);
	
	stats_size 	  = media->get_media_stats(mp4_file,NULL, handle, source, (media_stats_t*)stats_buffer, stats_size);

	//---生成m3u8文件--------------------------------------------
	DEBUG_LOG("into position G\n");	
	char* playlist_buffer = NULL;
	pure_filename = get_pure_filename_without_postfix(filename); //get only filename without any directory info
	if (pure_filename)
	{
		DEBUG_LOG("into position G1\n");	
		int playlist_size 		= generate_playlist((media_stats_t*)stats_buffer, pure_filename, NULL, 0, NULL, &numberofchunks);
		playlist_buffer 	= (char*)malloc( playlist_size);//用于缓存 m3u8文件
		if ( !playlist_buffer )
		{
			source->close(handle, 0);
			ERROR_LOG("malloc failed!\n");
			goto ERR;
		}
		DEBUG_LOG("into position H\n"); 

		playlist_size 			= generate_playlist((media_stats_t*)stats_buffer, pure_filename, playlist_buffer, playlist_size, NULL, &numberofchunks);
		if (playlist_size <= 0)
		{
			source->close(handle, 0);
			ERROR_LOG("generate_playlist failed!\n");
			goto ERR;
		}

		if(get_run_mode() == HLS_FILE_MODE)//文件模式：写入到文件后buf直接释放
		{
			f = fopen(playlist, "wb");
			if (f)
			{
				DEBUG_LOG("write m3u8 file....size(%d)...\n",playlist_size); 
				fwrite(playlist_buffer, 1, playlist_size, f);
				fclose(f);
			}
			
			hsl_out_info->m3u_buf = NULL;
			hsl_out_info->m3u_buf_size = 0;
		}
		else //if(get_run_mode() == HLS_MEMO_MODE)//内存模式：不写入文件，直接返回buf
		{
			hsl_out_info->m3u_buf = playlist_buffer;
			hsl_out_info->m3u_buf_size = playlist_size;
		}
		
		
		DEBUG_LOG("into position L\n"); 

	}

	source->close(handle, 0);
	if(get_run_mode() == HLS_FILE_MODE)//文件模式才释放，内存模式则由上层释放
	{
		if (playlist_buffer)	free(playlist_buffer);
	}	
	if (stats_buffer)		free(stats_buffer);
	if (handle)				free(handle);
	if (source)				free(source);
	DEBUG_LOG("into position M\n"); 
	return 0;
ERR:
	source->close(handle, 0);
	if (playlist_buffer)	free(playlist_buffer);
	if (stats_buffer)		free(stats_buffer);
	if (handle)				free(handle);
	if (source)				free(source);
	ERROR_LOG("into position M1\n"); 
	return -1;
	
}

char* buf_start = NULL;
char* buf_end = NULL;


/*******************************************************************************
*@ Description    :计算媒体文件的分片信息（每片开始的是IDR帧，每片分多少帧，帧的范围区间等）
					并进行切片
*@ Input          :<hsl_out_info>最终输出结果描述信息（这里需要更新里边的 ts_buf）
					<mp4_file>文件描述信息
					<out_filename>当前分片的TS文件名
					<piece>当前分片的序号
*@ Output         :
*@ Return         :成功 ： 0 ; 失败 ： -1
*@ attention      :
*******************************************************************************/
int  generate_piece(hls_out_info_t* hsl_out_info,FILE_info_t* mp4_file, char* out_filename, int piece)
{
	media_handler_t* 	media;
	file_source_t*   	source;
	file_handle_t* 		handle;
	media_stats_t* 		stats;//媒体 trak 的描述信息 
	int 				stats_size;
	char* 				stats_buffer;
	int 				source_size;
	char*				pure_filename;
	int 				data_size;
	media_data_t* 		data_buffer; //一个 TS文件对应从trak中取出帧数据的描述信息
	int 				muxed_size;
	char* 				muxed_buffer;
	FILE* f;

	char* filename = mp4_file->file_name;
	
	//获取媒体处理句柄（MP4、MP3、wav）
	media  = get_media_handler(filename);
	if ( !media )
	{
		ERROR_LOG("get_media_handler error!\n");
		return -1;
	}
		
	
	//---获取文件操作句柄的大小（打开、读写、关闭系列函数）----------------------------------
	source_size = get_file_source(NULL, filename, NULL, 0);//此处相当于 sizeof(file_source_t)

	source 	= (file_source_t*)malloc(source_size);
	if ( !source )
	{
		ERROR_LOG("malloc failed !\n");
		return -1;
	}

	source_size  = get_file_source(NULL, filename, source, source_size);
	if ( source_size <= 0 )
	{
		ERROR_LOG("get_file_source error!\n");
		goto ERR;
	}
		
	//----------------------------------------------------------------------------------------

	handle 	= (char*)malloc(source->handler_size);
	if ( !handle )
	{
		ERROR_LOG("malloc failed !\n");
		goto ERR;
	}


	if ( !source->open(source, handle, filename, FIRST_ACCESS) )
	{
		ERROR_LOG("open %s failed !\n",filename);
		goto ERR;
	}

	//---获取输入媒体文件的信息-------------------------------------------------------------------
	stats_size = media->get_media_stats(mp4_file,NULL, handle, source, NULL, 0);
	if ( stats_size <= 0)
	{
		source->close(handle, 0);
		ERROR_LOG("get_media_stats failed!\n");
		goto ERR;
	}

	stats_buffer = (char*)malloc( stats_size);
	if ( !stats_buffer )
	{
		source->close(handle, 0);
		ERROR_LOG("malloc faield !\n");
		goto ERR;
	}

	stats_size = media->get_media_stats(mp4_file,NULL, handle, source, (media_stats_t*)stats_buffer, stats_size);
	if ( stats_size <= 0)
	{
		source->close(handle, 0);
		ERROR_LOG("get_media_stats failed!\n");
		goto ERR;
	}
	//----------------------------------------------------------------------------------------------

	//---获取输入媒体文件的数据-------------------------------------------------------------------
	DEBUG_LOG("into position I\n");	
	data_size = media->get_media_data(mp4_file,NULL, handle, source, (media_stats_t*)stats_buffer, piece, NULL, 0);
	if (data_size <= 0)
	{
		source->close(handle, 0);
		ERROR_LOG("get_media_data failed!\n");
		goto ERR;
	}

	data_buffer = (media_data_t*)malloc(data_size);
	if ( !data_buffer )
	{
		source->close(handle, 0);
		ERROR_LOG("malloc failed ! malloc size = %d\n",data_size);
		goto ERR;
	}	
	memset(data_buffer,0,data_size);
	
	buf_start = (char*)data_buffer;
	buf_end = (char*)data_buffer + data_size;
	

	DEBUG_LOG("============Buffer size(%d) start_pos = %x  end_pos = %x\n",data_size,buf_start,buf_end); 
#if 1
	char* debug_write = (char*)((char*)data_buffer + data_size - 1);
	DEBUG_LOG("into debug_write...write pos = %x\n",debug_write);
	*debug_write = 1;
	DEBUG_LOG("back of debug_write...\n");
#endif

	data_size = media->get_media_data(mp4_file,NULL, handle, source, (media_stats_t*)stats_buffer, piece, data_buffer, data_size);
	if (data_size <= 0)
	{
		source->close(handle, 0);
		ERROR_LOG("get_media_data failed !\n");
		goto ERR;
	}
	//----------------------------------------------------------------------------------------------

	//----生成TS文件--------------------------------------------------------------------------------
	DEBUG_LOG("into position K\n");
	
	muxed_size = mux_to_ts((media_stats_t*)stats_buffer, data_buffer, NULL, 0);
	if ( muxed_size <= 0 )
	{
		source->close(handle, 0);
		ERROR_LOG("mux_to_ts failed !\n");
		goto ERR;
	}

	muxed_buffer = (char*)malloc(muxed_size);
	if ( !muxed_buffer )
	{
		source->close(handle, 0);
		ERROR_LOG("malloc failed ! malloc_size(%d)\n",muxed_size);
		goto ERR;
	}
	
	DEBUG_LOG("into position L\n"); 

	muxed_size = mux_to_ts((media_stats_t*)stats_buffer, data_buffer, muxed_buffer, muxed_size);
	if ( muxed_size <= 0 )
	{
		source->close(handle, 0);
		ERROR_LOG("mux_to_ts faile !\n");
		goto ERR;
	}
	//----------------------------------------------------------------------------------------------

	//----写入输出的TS文件------------------------------------------------------------------------------------------
	if(get_run_mode() == HLS_FILE_MODE)
	{
		f = fopen(out_filename, "wb");
		if (f)
		{
			DEBUG_LOG("fwrite TS file ...... size(%d)......\n",muxed_size);
			fwrite(muxed_buffer, 1, muxed_size, f);
			fclose(f);
		}
		else
		{
			ERROR_LOG("fopen %s failed !\n",out_filename);
		}
		DEBUG_LOG("into position N\n");
		hsl_out_info->ts_array[piece].ts_buf = NULL;
		strncpy(hsl_out_info->ts_array[piece].ts_name,out_filename,32);
		hsl_out_info->ts_array[piece].ts_buf_size = 0;
	}
	else //if(get_run_mode() == HLS_MEMO_MODE)
	{
		hsl_out_info->ts_array[piece].ts_buf = muxed_buffer;
		strncpy(hsl_out_info->ts_array[piece].ts_name,out_filename,32);
		hsl_out_info->ts_array[piece].ts_buf_size = muxed_size;
	}
	
	// FOR TEST
	/*
	if(piece == 0) {
		media_stats_t* p;
		p=stats_buffer;
		for (int kkk=0; kkk<300; kkk++)
			printf("\nPTS %d = %f",kkk,p->track[0]->pts[kkk]);
	}
	 */

	//---释放资源----------------------------------------------------------------
	source->close(handle, 0);
	if (source) 		free(source);
	if (handle) 		free(handle);
	if (stats_buffer) 	free(stats_buffer);
	if (data_buffer) 	free(data_buffer);
	if(get_run_mode() == HLS_FILE_MODE)
	{
		if (muxed_buffer)	free(muxed_buffer);
	}
	return 0;
	
ERR:
	source->close(handle, 0);
	if (source) 		free(source);
	if (handle) 		free(handle);
	if (stats_buffer) 	free(stats_buffer);
	if (data_buffer) 	free(data_buffer);
	if (muxed_buffer)	free(muxed_buffer);
	return -1;

}



typedef struct stream_t{
	char* buf;
	int pos;
	int size;
	int alloc_buffer;
} stream_t;

/*
@size：一次拷贝的大小
@nmemb：一共拷贝多少块
将buffer 中的数据放到stream中的buf中，
*/
static size_t fileWriteCallback(void *buffer, size_t size, size_t nmemb, void *stream)
{
	int bs = size * nmemb;
	stream_t* s = (stream_t*)stream;
	if (s->alloc_buffer)
	{
		if (!s->buf || s->size == 0) //计算需要初始化的内存大小
		{
			int cs = bs + 100;
			if (cs < 10000)
				cs = 10000;

			s->buf 	= (char*)malloc(cs);
			s->size = cs;
			s->pos 	= 0;
		}
		else
		{
			if (s->pos + bs >= s->size)
			{
				s->buf = (char*)realloc(s->buf, s->size * 2);
				s->size *= 2;
			}
		}
	}

	if (s->buf && s->size > s->pos + bs) //确定分配的内存足够
	{
		memcpy(s->buf + s->pos, buffer, bs);
		s->pos += bs;
	}

	return bs;

}

/// \return number of bytes processed
static size_t headerfunc_short ( char * ptr, size_t size, size_t nmemb, int* error_code )
{
	if (!strncmp ( ptr, "HTTP/1.1", 8 )) 
	{
		*error_code   = atoi ( ptr + 9 );
	}
	return nmemb * size;
}


/*
@playlist：返回http接收到的数据buf

*/
int download_file_to_mem(char* http_url, char** playlist)
{
	return 0;

	#if 0
	CURL *curl;
	CURLcode res = CURLE_CHUNK_FAILED;

	char Buf[1024];
	char range[1024];
	int offset = 30000;
	int size = 4096;

	int http_error_code = 0;
	stream_t stream = {0};

	if (playlist)
		stream.alloc_buffer = 1;
	else
		stream.alloc_buffer = 0;

	curl = curl_easy_init();
	if(curl) 
	{
		curl_easy_setopt(curl, CURLOPT_URL, http_url);

		curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, fileWriteCallback );
		curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &stream ); //设置回调函数中(fileWriteCallback)的void *stream 指针的来源。
		curl_easy_setopt ( curl, CURLOPT_HEADERFUNCTION, headerfunc_short );
		curl_easy_setopt ( curl, CURLOPT_HEADERDATA, &http_error_code );
		curl_easy_setopt ( curl, CURLOPT_VERBOSE, 1 );

		/* Perform the request, res will get the return code */
		res = curl_easy_perform(curl);
		/* Check for errors */
		if(res != CURLE_OK)
		{
			fprintf(stderr, "curl_easy_perform() failed: %s\n", 	curl_easy_strerror(res));
		}

		curl_easy_cleanup(curl);
	}

	if (playlist)
	{
		*playlist = stream.buf;
	}
	return res;
	#endif

}

int get_segments_count(char* playlist)
{
	int c = 0;
	int i = 0;
	int len = strlen(playlist);
	for(i = 0; i < len - 1; ++i)
	{
		if (playlist[i] == 0x0A && playlist[i+1] != '#')
		{
			++c;
		}
	}
	return c;
}

void get_segment_name(char* segment_name, int segment_name_size, char* playlist, int segment_num){
	int c = 0;
	int i = 0;
	int k;
	int len = strlen(playlist);
	for(i = 0; i < len - 1; ++i){
		if (playlist[i] == 0x0A && playlist[i+1] != '#'){
			if (c == segment_num){
				k = 0;
				while( i + k + 1 < len && playlist[i+k+1] != 0x0A && k < segment_name_size ){
					segment_name[k] = playlist[i+k+1];
					++k;
				}
				segment_name[k] = 0;

				break;
			}
			++c;
		}
	}
}

void get_file_url(char* file_url, int file_url_size, char* http_url, char* segment_name){
	int pos = strlen(http_url);
	int i;
	while(pos >= 0 && http_url[pos] != '/'){
		--pos;
	}
	for(i = 0; i < pos; ++i){
		file_url[i] = http_url[i];
	}
	file_url[pos] = 0;
	strcat(file_url, "/");
	strcat(file_url, segment_name);
}

#if 0
void process_hls_stream(char* http_url)
{
	char* playlist = NULL;
	int i;
	while (download_file_to_mem(http_url, &playlist) != CURLE_OK);

	if (playlist)
	{
		int segments_count = get_segments_count(playlist);

		for(i=0; i < segments_count; ++i)
		{
			char file_url[2048];
			char segment_name[2048];

			get_segment_name(segment_name, sizeof(segment_name), playlist, i);

			get_file_url(file_url, sizeof(file_url), http_url, segment_name);
			while( download_file_to_mem(file_url, NULL) != CURLE_OK);
		//	usleep(1000000);
		}

		free(playlist);
	}

}
#endif

//typedef struct size_error_t{
//	size_t size;
//	int error;
//}size_error_t;
//
///// \return number of bytes processed
//static size_t headerfunc_content_length ( char * ptr, size_t size, size_t nmemb, size_error_t* se )
//{
//	if (!strncmp ( ptr, "HTTP/1.1", 8 )) {
//		 se->error  = atoi ( ptr + 9 );
//	}
//	if (!strncmp ( ptr, "Content-Length:", 15 )) {
//		 se->size  = atoi ( ptr + 16 );
//	}
//
//	return nmemb * size;
//}
//
//CURLcode curl_get_resource_size(char* url, size_t* size, int* error_code){
//	 CURLcode res = CURLE_CHUNK_FAILED;
//	 CURL* ctx = curl_easy_init();
//	 if (ctx){
//		 size_error_t se;
//		 struct curl_slist *headers = NULL;
//
//		 headers = curl_slist_append(headers,"Accept: */*");
//		 if (headers){
//			 curl_easy_setopt(ctx,CURLOPT_HTTPHEADER, 	headers );
//			 curl_easy_setopt(ctx,CURLOPT_NOBODY,		1 );
//			 curl_easy_setopt(ctx,CURLOPT_URL,			url );
//			 curl_easy_setopt(ctx,CURLOPT_NOPROGRESS,	1 );
//			 curl_easy_setopt(ctx, CURLOPT_HEADERFUNCTION, headerfunc_content_length );
//			 curl_easy_setopt(ctx, CURLOPT_HEADERDATA,	&se );
//			 curl_easy_setopt(ctx, CURLOPT_VERBOSE, 0 );
//
//			 res = curl_easy_perform(ctx);
//			 curl_easy_cleanup(ctx);
//
//			 if (res == CURLE_OK){
//				 if (size){
//					 *size = se.size;
//				 }
//				 if (error_code){
//					 *error_code = se.error;
//				 }
//			 }
//		 }
//	 }
//	 return res;
//}
//
//typedef struct range_request_t{
//	char* buf;
//	int pos;
//	int size;
//} range_request_t;
//
//static size_t range_request_func(void *buffer, size_t size, size_t nmemb, void *stream)
//{
//	int bs = size * nmemb;
//	range_request_t* rr = (stream_t*)stream;
//
//	if (rr->buf && rr->size >= rr->pos + bs){
//		memcpy(rr->buf + rr->po s, buffer, bs);
//		rr->pos += bs;
//	}
//
//	return bs;
//
//}
//
//CURLcode curl_get_data(char* http_url, char* buffer, long long offset, long long requested_size, long long * received_size){
//	CURL *curl;
//	CURLcode res = CURLE_CHUNK_FAILED;
//	char range_str[128];
//	range_request_t rr;
//
//	rr.buf = buffer;
//	rr.pos = 0;
//	rr.size = requested_size;
//
//	curl = curl_easy_init();
//	if(curl) {
//		curl_easy_setopt(curl, CURLOPT_URL, http_url);
//
//		snprintf(range_str, sizeof(range_str), "%lld-%lld", (long long)offset, (long long)(offset + requested_size - 1));
//
//
//		curl_easy_setopt ( curl, CURLOPT_WRITEFUNCTION, range_request_func );
//		curl_easy_setopt ( curl, CURLOPT_WRITEDATA, &rr );
//		curl_easy_setopt ( curl, CURLOPT_VERBOSE, 0 );
//		curl_easy_setopt ( curl, CURLOPT_RANGE, range_str);
//
//		/* Perform the request, res will get the return code */
//		res = curl_easy_perform(curl);
//		/* Check for errors */
//		if(res != CURLE_OK){
//			fprintf(stderr, "curl_easy_perform() failed: %s\n", 	curl_easy_strerror(res));
//		}else{
//			if (received_size)
//				*received_size = rr.pos;
//		}
//
//		curl_easy_cleanup(curl);
//	}
//
//	return res;
//}

int hex_to_int(char c){
		switch (c){
			case '0': return 0;
			case '1': return 1;
			case '2': return 2;
			case '3': return 3;

			case '4': return 4;
			case '5': return 5;
			case '6': return 6;
			case '7': return 7;

			case '8': return 8;
			case '9': return 9;
			case 'a':
			case 'A': return 10;

			case 'b':
			case 'B': return 11;


			case 'c':
			case 'C': return 12;

			case 'd':
			case 'D': return 13;

			case 'e':
			case 'E': return 14;

			case 'f':
			case 'F': return 15;

		}
	}

	char convert_str_to_char(char c1, char c2){
		return (hex_to_int(c1) << 4) | hex_to_int(c2);
	}

	char* get_arg_value(/*request_rec * r,*/ char* args, char* key){
		int i,j;
		int args_len = strlen(args);
		int key_len= strlen(key);
		//char* result = apr_pcalloc(r->pool, args_len + 1);
		char* result = (char*)malloc(args_len + 1);
		int level = 0;
		memset(result, 0, args_len + 1);

		for(i = 0; i < args_len; ++ i){
			int found = 0;
			if (i == 0){
				found = (strncmp(&args[ i + j ], key, key_len) == 0) ? 1 : 0;
			}else{
				if (args[i - 1] == '&'){
					found = (strncmp(&args[ i + j ], key, key_len) == 0) ? 1 : 0;
				}
			}
			if (found){
				j = 0;
				i+=key_len+1;
				if (i + j + 3< args_len && args[i+j] == '%' && args[i+j+1] == '2' && args[i+j+2] == '2')
					i+=3;

				while ( i + j < args_len && args[j] != '&'){
					result[j] = args[i+j];
					++j;
				}

				if (j > 3 && result[j-3] == '%' && result[j-2] == '2' && result[j-1] == '2'){
					j-=3;
					result[j] =0 ;
				}
				break;
			}

		}

		return result;
	}





extern void hls_exit(void);
/*******************************************************************************
*@ Description    : fMP4文件 TS 切片主入口函数
*@ Input          :<mp4_file> fmp4 源文件信息
*@ Output         :
*@ Return         :成功：out_ts_info_t* 指针 失败：NULL
*******************************************************************************/
hls_out_info_t* hls_main (FILE_info_t* mp4_file)
{
	printf("\n\n****start slice...... **********************************\n");
	if(NULL == mp4_file)
	{
		ERROR_LOG("illegal arguement!\n");
		return NULL;
	}
	

	hls_out_info_t* hsl_out_info = (hls_out_info_t*)malloc(sizeof(hls_out_info_t));
	if(NULL == hsl_out_info)
	{
		ERROR_LOG("malloc failed !\n");
		return NULL;
	}
	memset(hsl_out_info,0,sizeof(hls_out_info_t));
	
	//确定TS程序的运行模式
	if(0 != strlen(mp4_file->file_name) && NULL == mp4_file->file_buf)
	{
		DEBUG_LOG("set_run_mode = HLS_FILE_MODE!\n");
		set_run_mode(HLS_FILE_MODE);
		hsl_out_info->hls_mode = 1;
	}
	else if(0 == strlen(mp4_file->file_name) && NULL != mp4_file->file_buf)
	{
		DEBUG_LOG("set_run_mode = HLS_MEMO_MODE!\n");
		set_run_mode(HLS_MEMO_MODE);
		hsl_out_info->hls_mode = 2;
	}
	else
	{
		ERROR_LOG("set_run_mode failed! illegal arguement! \n");
		goto ERR;
	}
		
	set_encode_audio_bitrate(16000);
	set_segment_length(mp4_file->segment_duration /*SEGMENT_DURATION*/);// 设置单个TS文件切片时长
	set_allow_mp4(1);	//采用MP4文件切片模式（目前只支持该模式，且内部只调试过fmp4文件的逻辑）
	set_logo_filename(NULL);

	char path[64];
	snprintf(path,50,"%s%s.m3u8",URL_PREFIX,M3U8_FILE_NAME);
	int counterrr=0; //ts分片文件的个数

	/*---# memory 模式下，伪造一个目录文件，目的只是让后边的操作针对MP4进行--------------------*/
	if(get_run_mode() == HLS_MEMO_MODE)
	{
		strcpy(mp4_file->file_name,"/jffs0/fmp4.mp4"); 
	}
		
	/*---生成m3u8文件，计算可以生成TS切片文件的个数--------------------------------------------*/
	if(generate_playlist_test(hsl_out_info,mp4_file,path,&counterrr) < 0)
	{
		ERROR_LOG("generate_playlist_test failed !\n");
		goto ERR;
	}
	
	strncpy(hsl_out_info->m3u_name,path,32);
	hsl_out_info->ts_num = counterrr;
	/*---生成TS切片文件------------------------------------------------------------------------*/
	int i = 0;
	
	for(i = 0; i < counterrr; ++i) //循环一次生成一个TS切片文件 
	{
		DEBUG_LOG("into position O\n"); 
		char tmp[64];//输出的TS文件名
		snprintf(tmp,50, "%s%s_%d.ts",get_pure_pathname(URL_PREFIX),get_pure_filename_without_postfix(mp4_file->file_name), i);
		strncpy(hsl_out_info->ts_array[i].ts_name,tmp,32);
		if(generate_piece(hsl_out_info,mp4_file, tmp, i) < 0)
		{
			ERROR_LOG("generate_piece %s i=%d error!\n",mp4_file->file_name,i);
			goto ERR;
		}
	}

	DEBUG_LOG("At the position of  END!\n");
	hls_exit();
	printf("****END slice...... **********************************\n\n");
	return hsl_out_info;
ERR:
	hls_exit();
	if(hsl_out_info) free(hsl_out_info);
	printf("****slice err!!...... **********************************\n\n");
	return NULL;
}

void hls_main_exit(hls_out_info_t* hls_info)
{
	if(hls_info != NULL)
	{
		if(hls_info->m3u_buf) 
		{
			free(hls_info->m3u_buf);
			hls_info->m3u_buf = NULL;
		}

		int i = 0;
		for(i=0 ; i<hls_info->ts_num ; i++)
		{
			if(hls_info->ts_array[i].ts_buf)
			{
				free(hls_info->ts_array[i].ts_buf);
				hls_info->ts_array[i].ts_buf = NULL;
			}
				
		}
		
	}
}










