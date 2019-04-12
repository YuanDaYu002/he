 
/***************************************************************************
* @file: amazon_S3.c
* @author:   
* @date:  3,27,2019
* @brief:  
* @attention:
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h> 
#include <sys/types.h>
#include <fcntl.h>


#include <openssl/ossl_typ.h>
#include <openssl/hmac.h>  
#include <openssl/sha.h> 





#include "dlfcn.h"
//#include "curl/curl.h"
#include "cJSON/cJSON.h"
#include "amazon_S3.h"
#include "typeport.h"
#include "gh_http.h"


#define A_PUT_SUCCESS 1		//发送成功
#define A_PUT_FAIL 0		//发送失败

#define A_JPG_DIR			"picture"
#define A_VIDEO_DIR			"video"
#define A_VIDEO6s_DIR		"video6s"

#define AWS_REGON 			"cn-northwest-1"		   
#define AWS_HOST			"s3.cn-northwest-1.amazonaws.com.cn" 
#define AWS_ACCESS_KEY_ID 	"AKIAPFUPG7ABHT3W5P4Q"  
#define AWS_ACCESS_SECRET	"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" 

//#define AMAZON_SIGNATURE	"http://d285s66sa0r9bx.cloudfront.net"
#define AMAZON_QIANMIN		"http://d285s66sa0r9bx.cloudfront.net"

const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="; 

static AMAZON_INFO g_amazon_info = {0,-1," "," "," "," "," "," "," "," ", " "}; 


static char aws_yymmdd[12];  //上传 amazon web service 的时间戳
static char aws_TimeStamp[20];

/*---#获取设备的uiid------------------------------------------------------------*/
//extern char *ipc_get_tutk_guid(void);
//XZ1T6R2RBRP7PH7P111A  ##190
//WA1AY666S6YWG9NZ111A  ##网关 
char*uuid = "XZ1T6R2RBRP7PH7P111A";

char *ipc_get_tutk_guid(void)
{
	return uuid;
}


enum  amazon_environment_e
{
	AMAZON_DEVELOP = 0,  //开发环境
	AMAZON_TESTING = 1,	 //测试环境
	AMAZON_PRODUCE = 2	 //生产环境（线上，美国环境）
};

/*---#开发环境(0)/测试环境(1)/生产环境(2)  ---切换开关----------------------------------*/
int url_index = AMAZON_DEVELOP; //决定下方url数组的下标

/*---#验证设备图片视频上传接口 请求URL:-----------------------------------------------*/
char* url_verify_up_interface[3] = {
	"http://hle1879.picp.net:9020/device/get-info/deviceHandle",  		//开发环境
 	"http://www.pethin.com:9080/device/get-info/deviceHandle",	  		//测试环境
 	"http://service.home-linking.com:9010/device/get-info/deviceHandle" //生产环境
	}; 

/*---#添加设备的视频图片信息接口（推送图片视频文件） 请求URL:实际用的云服务返回的 URL 连接------------------*/
char *url_post_put[3] = {" "," "," "};
char *url_add_info_interface[3]={
	"http://hle1879.picp.net:9020/device/upload-info/addPicVideo", 		  //开发环境
 	"http://www.pethin.com:9080/device/upload-info/addPicVideo",		  //测试环境
 	"http://service.home-linking.com:9010/device/upload-info/addPicVideo" //生产环境
	};



char *curl_get(cJSON *pushMsg, char *url);


int is_amazon_info_update(void)
{
	return g_amazon_info.today_req;
}

/*******************************************************************************
*@ Description	  :  将输入的字符串转换成对应的ASCII值字符串
*@ Input		  : <input>:数据指针
					<inlen>:数据的长度
*@ Output		  : <out_hex>:十六进制文件
*@ Return		  :
*******************************************************************************/
void make_hex(unsigned char *input, int inlen, char *out_hex)
{
	int i = 0;	
	char tmp[3] = {0}; 
	out_hex[0] = '\0';
	for(i = 0; i < inlen; i++ )  
	{	snprintf(tmp,3,"%02x", input[i]);	
		strcat(out_hex, tmp);  
	}  
	//printf("output==%s\n", out_hex);
}

unsigned long get_file_size(const char *path)  
{  
    unsigned long filesize = -1;      
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = statbuff.st_size;  
    }  
    return filesize;  
}  

//Wed, 30 Aug 2017 07:12:00 +0000
int get_datetime(char *str)
{
	if(str == NULL)
		return -1;
	time_t timer=time(NULL); 
	struct tm *gmt;
	gmt = gmtime(&timer);
	strftime(str, 32, "%a, %e %b %G %H:%M:%S +0000", gmt);
	
	//printf("str=================%s===================%ld\n",str,timer);
	return 0;
}



/* */ 
char *base64_encode1(const char* data, int data_len) 
{ 
	//int data_len = strlen(data); 
	int prepare = 0; 
	int ret_len; 
	int temp = 0; 
	char *ret = NULL; 
	char *f = NULL; 
	int tmp = 0; 
	unsigned char changed[4]; 
	int i = 0; 
	ret_len = data_len / 3; 
	temp = data_len % 3; 
	if (temp > 0) 
	{ 
		ret_len += 1; 
	} 
	ret_len = ret_len*4 + 1; 
	ret = (char *)malloc(ret_len); 
	  
	if ( ret == NULL) 
	{ 
		printf("No enough memory.\n"); 
		exit(0); 
	} 
	memset(ret, 0, ret_len); 
	f = ret; 
	while (tmp < data_len) 
	{ 
		temp = 0; 
		prepare = 0; 
		memset(changed, '\0', 4); 
		while (temp < 3) 
		{ 
			//printf("tmp = %d\n", tmp); 
			if (tmp >= data_len) 
			{ 
				break; 
			} 
			prepare = ((prepare << 8) | (data[tmp] & 0xFF)); 
			tmp++; 
			temp++; 
		} 
		prepare = (prepare<<((3-temp)*8)); 
		//printf("before for : temp = %d, prepare = %d\n", temp, prepare); 
		for (i = 0; i < 4 ;i++ ) 
		{ 
			if (temp < i) 
			{ 
				changed[i] = 0x40; 
			} 
			else 
			{ 
				changed[i] = (prepare>>((3-i)*6)) & 0x3F; 
			} 
			*f = base[changed[i]]; 
			//printf("%.2X", changed[i]); 
			f++; 
		} 
	} 
	*f = '\0'; 
	  
	return ret; 
	  
} 

	


/*******************************************************************************
*@ Description    :  合成完整的文件上传 amazon URL 
*@ Input          :
					<type>: 文件的类型
					<finename>:文件名字
*@ Output         :
					<path>：文件的完整路径
					<url>：构造的url字符串
*@ Return         :成功 ：0  		失败：-1 
*******************************************************************************/
int amazon_info_complet(char *path, char *url, char *type, char *finename)
{
	if(path==NULL || url==NULL)
		return -1;
	
	strcpy(path,"/"); 
	strcat(path,g_amazon_info.bucket); 
	strcat(path,"/");	 
	strcat(path,g_amazon_info.deviceid);	 
	strcat(path,"/");	
	strcat(path,g_amazon_info.date); 
	strcat(path,"/");
	strcat(path,type);
	strcat(path,"/");
	if(0 == strncmp(finename,"/tmp/",5))
		strcat(path,finename+strlen("/tmp/"));// "/tmp/150108.mkv"
	else if(0 == strncmp(finename,"/jffs0/",7))
		strcat(path,finename+strlen("/jffs0/"));// "/jffs0/150108.mkv"
	else if(0 == strncmp(finename,"/ramfs/",7))
		strcat(path,finename+strlen("/ramfs/"));
	else
	{
		ERROR_LOG("Unconventional path!\n"); //非常规路径
		strcat(path,finename);
	}
		

	strcpy(url,"https://");
	strcat(url,g_amazon_info.host); // "https://s3.amazonaws.com/");//
	strcat(url, path);//path+1

	//合成后：https://s3.amazonaws.com/。。。。。

	return 0;
}

/*******************************************************************************
*@ Description    :  将http请求字符串转换成签名
*@ Input          : <http_req>:http请求（字符串）
*@ Output         : <stringToSign>:签名（字符串）
*@ Return         :
*******************************************************************************/
void make_StringToSign(const char *http_req, char *stringToSign)
{
	unsigned char sha[33] = {0}; //哈希值
	char hex[65] = {0};
	
	SHA256((unsigned char*)http_req, strlen(http_req), sha);//求哈希值
	make_hex(sha ,32, hex);

	
	strcpy(stringToSign, "AWS4-HMAC-SHA256\n");
	strcat(stringToSign, aws_TimeStamp);
	strcat(stringToSign, "\n");
	char Scope[64];
	snprintf(Scope,64,"%s/%s/s3/aws4_request\n", aws_yymmdd, g_amazon_info.regon);
	strcat(stringToSign, Scope);
	strcat(stringToSign, hex);
	//printf("stringToSign==%s\n",stringToSign);
}


/*******************************************************************************
*@ Description    :生成数字签名  
*@ Input          :<YourSecretAccessKeyID>
					<StringToSign>
					<Signature>
*@ Output         :
*@ Return         :
*******************************************************************************/
#if 1
void make_signatue(char *YourSecretAccessKeyID, char *StringToSign, char *Signature)//SECRET_ACCESS_KEYID
{
	int len = 0;

	unsigned char Hamc_sha1[36];  unsigned char Hamc_sha2[36];
	const EVP_MD * engine = NULL;
	engine = EVP_sha256();
	HMAC_CTX ctx; 

	char AWS4_key[64]; strncpy(AWS4_key, "AWS4",64); strcat(AWS4_key, YourSecretAccessKeyID); 
	HMAC_CTX_init(&ctx);  	
	HMAC_Init_ex(&ctx, AWS4_key, strlen(AWS4_key), engine, NULL);  	
	HMAC_Update(&ctx, (unsigned char*)aws_yymmdd, strlen(aws_yymmdd));
	HMAC_Final(&ctx, Hamc_sha1, &len);	//printf("len =%d\n",len);
	HMAC_CTX_cleanup(&ctx); 

	HMAC_CTX_init(&ctx); 
	HMAC_Init_ex(&ctx, Hamc_sha1, len, engine, NULL);  	
	HMAC_Update(&ctx, (unsigned char*)g_amazon_info.regon, strlen(g_amazon_info.regon));////////////gai
	HMAC_Final(&ctx, Hamc_sha2, &len);	
	HMAC_CTX_cleanup(&ctx); 


	HMAC_CTX_init(&ctx); 
	HMAC_Init_ex(&ctx, Hamc_sha2, len, engine, NULL);  	
	HMAC_Update(&ctx, (unsigned char*)"s3", strlen("s3"));////////////gai
	HMAC_Final(&ctx, Hamc_sha1, &len);	//printf("len =%d\n",len);
	HMAC_CTX_cleanup(&ctx); 

	HMAC_CTX_init(&ctx); 
	HMAC_Init_ex(&ctx, Hamc_sha1, len, engine, NULL);  	
	HMAC_Update(&ctx, (unsigned char*)"aws4_request", strlen("aws4_request"));////////////gai
	HMAC_Final(&ctx, Hamc_sha2, &len);	//printf("len =%d\n",len);
	HMAC_CTX_cleanup(&ctx); 

	HMAC_CTX_init(&ctx); 
	HMAC_Init_ex(&ctx, Hamc_sha2, len, engine, NULL);  	
	HMAC_Update(&ctx, (unsigned char*)StringToSign, strlen(StringToSign));////////////gai
	HMAC_Final(&ctx, Hamc_sha1, &len);	//printf("len =%d\n",len);
	HMAC_CTX_cleanup(&ctx); 
	
	make_hex(Hamc_sha1 ,32, Signature);	
}

#else
void make_signatue(char *YourSecretAccessKeyID, char *StringToSign, char *Signature)//SECRET_ACCESS_KEYID
{
	int len = 0;

	unsigned char Hamc_sha1[36];  unsigned char Hamc_sha2[36];
	const EVP_MD * engine = NULL;
	engine = EVP_sha256();
	HMAC_CTX *ctx = NULL; 
	ctx = HMAC_CTX_new();
	
	char AWS4_key[64]; strcpy(AWS4_key, "AWS4"); strcat(AWS4_key, YourSecretAccessKeyID); 
	HMAC_CTX_reset(ctx);	//HMAC_CTX_init(&ctx);  	
	HMAC_Init_ex(ctx, AWS4_key, strlen(AWS4_key), engine, NULL);  	
	HMAC_Update(ctx, (unsigned char*)aws_yymmdd, strlen(aws_yymmdd));
	HMAC_Final(ctx, Hamc_sha1, &len);	//printf("len =%d\n",len);
	//HMAC_CTX_cleanup(&ctx); 

	HMAC_CTX_reset(ctx); 
	HMAC_Init_ex(ctx, Hamc_sha1, len, engine, NULL);  	
	HMAC_Update(ctx, (unsigned char*)g_amazon_info.regon, strlen(g_amazon_info.regon));////////////gai
	HMAC_Final(ctx, Hamc_sha2, &len);	
	//HMAC_CTX_cleanup(&ctx); 


	HMAC_CTX_reset(ctx); 
	HMAC_Init_ex(ctx, Hamc_sha2, len, engine, NULL);  	
	HMAC_Update(ctx, (unsigned char*)"s3", strlen("s3"));////////////gai
	HMAC_Final(ctx, Hamc_sha1, &len);	//printf("len =%d\n",len);
	//HMAC_CTX_cleanup(&ctx); 

	HMAC_CTX_reset(ctx); 
	HMAC_Init_ex(ctx, Hamc_sha1, len, engine, NULL);  	
	HMAC_Update(ctx, (unsigned char*)"aws4_request", strlen("aws4_request"));////////////gai
	HMAC_Final(ctx, Hamc_sha2, &len);	//printf("len =%d\n",len);
	//HMAC_CTX_cleanup(&ctx); 

	HMAC_CTX_reset(ctx); 
	HMAC_Init_ex(ctx, Hamc_sha2, len, engine, NULL);  	
	HMAC_Update(ctx, (unsigned char*)StringToSign, strlen(StringToSign));////////////gai
	HMAC_Final(ctx, Hamc_sha1, &len);	//printf("len =%d\n",len);
	HMAC_CTX_free(ctx); 
	
	make_hex(Hamc_sha1 ,32, Signature);	
}
#endif

/*******************************************************************************
*@ Description    :获取签名  
*@ Input          :<YourSecretAccessKeyID> 安全密匙
					<Stringtosign>
*@ Output         :
*@ Return         : 签名
*******************************************************************************/
#if 1
char *make_signatue_am(char *YourSecretAccessKeyID, char *Stringtosign)//SECRET_ACCESS_KEYID
{
	char *Signature = NULL;
	char Hamc_sha1[256];
	const EVP_MD * engine = NULL;
	engine = EVP_sha1();
	HMAC_CTX ctx;  
	HMAC_CTX_init(&ctx);  
	HMAC_Init_ex(&ctx, YourSecretAccessKeyID, strlen(YourSecretAccessKeyID), engine, NULL);  
	HMAC_Update(&ctx, (unsigned char*)Stringtosign, strlen(Stringtosign));		// input is OK; &input is WRONG !!!  
	int len = 0;
	HMAC_Final(&ctx, Hamc_sha1, &len);  
	HMAC_CTX_cleanup(&ctx); 
	
	Signature = base64_encode1(Hamc_sha1,20);
	//printf("\n%d  %d\nSignature=%s\n", strlen(Hamc_sha1),strlen(Signature),Signature);
	return Signature;
}

#else
char *make_signatue_am(char *YourSecretAccessKeyID, char *Stringtosign)//SECRET_ACCESS_KEYID
{
	char *Signature = NULL;
	char Hamc_sha1[256];
	const EVP_MD * engine = NULL;
	engine = EVP_sha1();
  
	HMAC_CTX *ctx = NULL; 
	ctx = HMAC_CTX_new();
	
	HMAC_CTX_reset(ctx);  
	HMAC_Init_ex(ctx, YourSecretAccessKeyID, strlen(YourSecretAccessKeyID), engine, NULL);  
	HMAC_Update(ctx, (unsigned char*)Stringtosign, strlen(Stringtosign));		// input is OK; &input is WRONG !!!  
	int len = 0;
	HMAC_Final(ctx, Hamc_sha1, &len);  
	HMAC_CTX_free(ctx); 
	
	Signature = base64_encode1(Hamc_sha1,20);
	//printf("\n%d  %d\nSignature=%s\n", strlen(Hamc_sha1),strlen(Signature),Signature);
	return Signature;
}

#endif


/*******************************************************************************
*@ Description    : amazon 云返回信息写入函数
					（libcurl 进行 http put 操作接收到 amazon 云 的返回信息） 
*@ Input          :<ptr> 所指向要写的数据
					<size>每个数据块的大小
					<nmemb>数据的块数
*@ Output         :<stream>信息的输出位置
*@ Return         :总数据的大小（size * nmemb）
*******************************************************************************/
size_t amazonWriteData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t data_size = size * nmemb;
	if(data_size<1024)
    	strcat((char *)stream, (char *)ptr);  
    return data_size;
}


/*******************************************************************************
*@ Description    :  将文件推送到 amazon 云上
*@ Input          :	<filename>:文件名（要推送到amazon的文件）
					<tsflag>:TS文件的文件标记
					<type>：文件类型
*@ Output         :
*@ Return         :成功：0  	失败：-1
*******************************************************************************/
int amazon_curl_send(char *filename, ts_flag_e tsflag, file_type_e type)
{	
	DEBUG_LOG("1000000013 ,filename(%s), tsflag(%d) type(%d)\n",filename,tsflag,type);
	char filetype[16]; //http put 头部分的文件类型
	char sub_dir[16];  // amazon 云端目标路径中的子项目录（不同文件类型推送路径不一样）	
	char path[256];
	char http_url[512];	
	int timeout;


	/*---构造应答的时间戳，年月日，时分秒---------*/
	time_t timer=time(NULL); 
	struct tm *gmt = gmtime(&timer);
	snprintf(aws_yymmdd,12 ,"%04d%02d%02d", 1900+gmt->tm_year, 1+gmt->tm_mon, gmt->tm_mday);
	snprintf(aws_TimeStamp, 20,"%04d%02d%02dT%02d%02d%02dZ", 1900+gmt->tm_year, 1+gmt->tm_mon, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
	DEBUG_LOG("date =%s, time=%s\n",aws_yymmdd, aws_TimeStamp);

	/*---构造amazon云端子目录（将文件推送到哪个子目录）-----------------*/
	if(tsflag == TS_FLAG_JPG)//TS文件类型是JPEG，则需要到jpeg的目录下去取
	{
		strcpy(sub_dir,A_JPG_DIR);
		strcpy(filetype,"image/jpeg");
		timeout = 50;
	}
	else if(tsflag == TS_FLAG_6S )
	{
		strcpy(sub_dir,A_VIDEO6s_DIR);
		if(type==TYPE_TS)
			strcpy(filetype,"video/mp2t");//x-mpeg
		else
			strcpy(filetype,"audio/mpegurl");
		timeout = 50;
	}
	else if(tsflag == TS_FLAG_ONE)
	{
		goto ERR;
	}
	else//ts文件属于告警录像文件
	{
		strcpy(sub_dir,A_VIDEO_DIR);
		if(type==TYPE_TS)
			strcpy(filetype,"video/mp2t");//x-mpeg
		else
			strcpy(filetype,"audio/mpegurl");
		timeout = 150;
	}
	
	/*---构造完整 amazon 云 文件推送的 url 字符串---------------*/
	amazon_info_complet(path, http_url, sub_dir, filename);
	unsigned long fsize = get_file_size(filename);
	
	////////////////////////////////////////////////////////////////aws
	/*---http PUT命令构造-----------------
	PUT
	
	filetype
	time_str
	aws_TimeStamp
	x-amz-acl:public-read
	*/
	char http_req[256];	
	http_req[0] = '\0';
	strcat(http_req,"PUT\n");	 
	strcat(http_req,"\n");
	strcat(http_req,filetype);	
	strcat(http_req,"\n");		

	char time_str[64];
	get_datetime(time_str);
	strcat(http_req,time_str);
	strcat(http_req,"\n");
	strcat(http_req,aws_TimeStamp);
	strcat(http_req,"\n");
	
	strcat(http_req,"x-amz-acl:public-read\n");//test		

	//strcat(http_req,"x-amz-content-sha256: UNSIGNED-PAYLOAD\n");
	
	//printf("http_req//////////////////\n%s\n////////////////\n",http_req);
	//////////////2  Signature
	
	/*---将请求字符串转换成数字签名--------------------------------*/
	char stringToSign[256] = {0};
	make_StringToSign(http_req , stringToSign);
	
	//////////////3  Signature
	unsigned char Signature[65] = {0};
	make_signatue(g_amazon_info.secret_aceess_keyid, stringToSign, Signature);

	/*---依据签名生成数字证书--------------------------------*/
	///////////////4 Authorization
	char Authorization[1024];	
	Authorization[0] = '\0';
	snprintf(Authorization,1024	,"Authorization: AWS4-HMAC-SHA256 "
							"Credential=%s/%s/%s/s3/aws4_request, "
							"SignedHeaders=content-type;host;x-amz-date;x-amz-acl, "
							"Signature=%s",
							g_amazon_info.aws_access_keyid, 
							aws_yymmdd, 
							g_amazon_info.regon,
							Signature
							);
	DEBUG_LOG("Authorization////////////////////\n%s\n///////////\n", Authorization);

	/*--- https 设置请求头------------------------------*/
	char * headers = (char*)malloc(1024*3);
	if(NULL == headers)
	{
		ERROR_LOG("malloc failed!\n");
		goto ERR;
	}
	memset(headers,0,1024*3);

	char content_type[128] = {0};
	snprintf(content_type,sizeof(content_type),"Content-Type: %s",filetype);
	strcat(headers,content_type);
	strcat(headers,"\r\n");
	
	char contentlen[64];
	snprintf(contentlen,64,"Content-Length: %ld", fsize);
	strcat(headers,contentlen);
	strcat(headers,"\r\n");
	
	char host[128];
	snprintf(host,sizeof(host),"Host: %s",g_amazon_info.host);
	strcat(headers,host);
	strcat(headers,"\r\n");

	
	char time_str1[64];	
	snprintf(time_str1,sizeof(time_str1),"Date: %s",time_str);
	strcat(headers,time_str1);
	strcat(headers,"\r\n");
	
	char aws_date[64];
	snprintf(aws_date,sizeof(aws_date),"X-Amz-Date: %s",aws_TimeStamp);//X-Amz-Date: 20150830T123600Z
	strcat(headers,aws_date);
	strcat(headers,"\r\n");
	
	strcat(headers,"x-amz-acl: public-read");
	strcat(headers,"\r\n");
	
	strcat(headers,"x-amz-content-sha256: UNSIGNED-PAYLOAD");
	strcat(headers,"\r\n");
	
	strcat(headers,Authorization);
	strcat(headers,"\r\n");
	
	strcat(headers,"Connection: Keep-Alive");
	strcat(headers,"\r\n");
	
	strcat(headers,"Expect: 100-continue");
	strcat(headers,"\r\n");

	char* response = http_upload_file(http_url,headers,content_type,filename);
	if(NULL == response)
	{
		ERROR_LOG("http_upload_file failed !\n");
		goto ERR;
	}
	DEBUG_LOG(" ---push local file = %s ---servers respone:%s---\n",filename, response);
	
#if 0
	/*---初始化 curl 的 handle ,设置curl选项----------------------------------*/
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);   
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);//设定为不验证证书和host 

	//////test
	if (0) {//
		curl_easy_setopt(curl, CURLOPT_PROXY, "47.75.136.27"); //test//47.75.146.250
		//curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, "admin:admin");
		curl_easy_setopt(curl, CURLOPT_PROXYPORT, 8888); 
		curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP); 
	}
	
	curl_easy_setopt(curl, CURLOPT_URL, http_url);//url
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_PUT, 1L); 
	/*---启用时允许HTTP发送文件,设置文件的 句柄 + 指针-------------------------*/
	curl_easy_setopt(curl,  CURLOPT_INFILE, fp); 
	curl_easy_setopt(curl,  CURLOPT_INFILESIZE, fsize);  
	/*---设置下载数据回调函数---------------------------*/
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, amazonWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);
	/*---设置屏蔽其他信号-------------------------------*/
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 
	/*---将服务器服务器返回的"Location: "放在header中递归的返回给服务器--*/
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	/*-----执行设置好的curl_easy_setopt选项------------*/
	int ret= curl_easy_perform(curl);
	curl_easy_cleanup(curl); //释放curl的 handle
#endif

	if(response) free(response);
	if(headers) free(headers);
	return 0;	
ERR:
	if(response) free(response);
	if(headers) free(headers);
	return -1;	
}



/*******************************************************************************
*@ Description    :  将文件推送到 amazon 云上（美国版）
*@ Input          :	<filename>:文件名（要推送到amazon的文件）
					<tsflag>:TS文件的文件标记
					<type>：文件类型
*@ Output         :
*@ Return         :成功 ： 0
					失败：-1
*******************************************************************************/
int amazon_curl_send_am(char *filename, char tsflag, char type)
{	
	DEBUG_LOG("1000000013 ,%s, %d\n",filename,tsflag);
	char filetype[16]; 
	char sub_dir[16]; //文件对应的子目录
	char path[256];
	char url[512];	
	int timeout;

	if(tsflag == TS_FLAG_JPG)
	{
		strcpy(sub_dir,A_JPG_DIR);
		strcpy(filetype,"image/jpeg");
		timeout = 50;
	}
	else if(tsflag == TS_FLAG_6S)
	{
		strcpy(sub_dir,A_VIDEO6s_DIR);
		if(type==TYPE_TS)
			strcpy(filetype,"video/mp2t");//x-mpeg
		else
			strcpy(filetype,"audio/mpegurl");
		timeout = 50;
	}
	else if(tsflag == TS_FLAG_ONE)
	{
		return -1;
	}
	else  //是告警的视频文件
	{
		strcpy(sub_dir,A_VIDEO_DIR);
		if(type==TYPE_TS)
			strcpy(filetype,"video/mp2t");//x-mpeg
		else
			strcpy(filetype,"audio/mpegurl");
		timeout = 150;
	}
	amazon_info_complet(path, url, sub_dir, filename);//获取文件上传云服务的 URL
	/*printf("1000011filename=%s,type=%s,path=%s\n url=%s\n%s,id=%s,key=%s\n",filename,sub_dir, path, url,
	 	filetype,g_amazon_info.aws_access_keyid,g_amazon_info.secret_aceess_keyid);
	*/
	
	FILE *fp = fopen(filename, "r"); 
	if (fp == NULL)
	{
		//remove(filename);	
		ERROR_LOG("open path==%s fail,%d\n",filename,errno);
		return -1;
	}	
	unsigned long fsize = get_file_size(filename);
	
	/*---http PUT命令构造-----------------
		PUT
		
		filetype
		time_str
		x-amz-acl:public-read
		path
		
	*/
	char Stringtosign[256];	
	Stringtosign[0] = '\0';
	strcat(Stringtosign,"PUT\n");	 
	strcat(Stringtosign,"\n");
	strcat(Stringtosign,filetype);	
	strcat(Stringtosign,"\n");	
	//strcat(Stringtosign,"image/jpeg\n");//"image/jpeg\n");	 	
	
	char time_str[64];
	get_datetime(time_str);
	strcat(Stringtosign,time_str);
	strcat(Stringtosign,"\n");

	if(type==TYPE_TS)
		strcat(Stringtosign,"x-amz-acl:public-read\n");//test 	
	 
	strcat(Stringtosign,path);	
	//printf("##########Stringtosign=#########\n%s\n#####################################\n", Stringtosign);

	/*---#获取数字签名------------------------------------------------------------*/
	char *Signature = NULL; //签名
	Signature = make_signatue_am(g_amazon_info.secret_aceess_keyid,Stringtosign);

	/*---#生成证书------------------------------------------------------------*/
	char Authorization[1024];
	Authorization[0] = '\0';
	strcpy(Authorization,"Authorization: AWS ");
	strcat(Authorization,g_amazon_info.aws_access_keyid);
	strcat(Authorization,":");
	strcat(Authorization,Signature);
	free(Signature);
	DEBUG_LOG("len=%d   %s\n",strlen(Authorization),Authorization);

	/*---#设置请求头------------------------------------------------------------*/
	char * headers = (char*)calloc(1024*3,sizeof(char));
	if(NULL == headers)
	{
		ERROR_LOG("malloc failed!\n");
		goto ERR;
	}
	//struct curl_slist *headers = NULL;
		
	char content_type[128]; 
	strcpy(content_type, "Content-Type: ");
	strcat(content_type,filetype);

	strcat(headers,content_type);
	strcat(headers,"\r\n");


	char contentlen[64];
	snprintf(contentlen,sizeof(contentlen),"Content-Length: %ld", fsize);
	strcat(headers,contentlen);
	strcat(headers,"\r\n");

	char host[128];
	snprintf(host,sizeof(host),"Host: %s",g_amazon_info.host);
	strcat(headers,host);
	strcat(headers,"\r\n");
	//strcat(headers,g_amazon_info.host); //"s3.amazonaws.com"); //bucket-1-20180131.s3.cn-northwest-1.amazonaws.com.cn
	DEBUG_LOG("host...................%s\n",g_amazon_info.host);
	
	char time_str1[64];
	strcpy(time_str1,"Date: ");
	strcat(time_str1,time_str);
	strcat(headers,time_str1);
	strcat(headers,"\r\n");

	strcat(headers,Authorization);
	strcat(headers,"\r\n");

	
	if(type==TYPE_TS) 
	{
		strcat(headers,"x-amz-acl: public-read");
		strcat(headers,"\r\n");
	}
		
	strcat(headers,"Connection: Keep-Alive");
	strcat(headers,"\r\n");
	strcat(headers,"Expect: 100-continue");
	strcat(headers,"\r\n");

	char*response = http_upload_file(url,headers,content_type,filename); 
	if(NULL == response)
	{
		ERROR_LOG("http_upload_file failed !\n");
		goto ERR;
	}
	DEBUG_LOG(" ---push local file = %s ---servers response:%s---\n",filename, response);

	
	#if 0
	/*---#初始化 + 设置选项------------------------------------------------------------*/
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);//设定为不验证证书和host    
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); 
	
	curl_easy_setopt(curl, CURLOPT_URL, url);//url
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);//超时时间（单位：s）
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);//将设置 http 请求头参数
	curl_easy_setopt(curl, CURLOPT_PUT, 1L); //设置用HTTP上传一个文件 
	curl_easy_setopt(curl,  CURLOPT_INFILE, fp); //文件句柄设置
	curl_easy_setopt(curl,  CURLOPT_INFILESIZE, fsize);  //文件大小设置
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, amazonWriteData);//设置一旦检测到有数据需要接收时的回调函数
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);//设置上方 amazonWriteData 回调函数的第4个参数（数据输出的大方）
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

	/*---#请求 + 打印返回信息------------------------------------------------------------*/
	int ret= curl_easy_perform(curl);
	curl_easy_cleanup(curl); 
	#endif

	
	if(fp) fclose(fp); 
	if(response) free(response);
	if(headers) free(headers);
	return 0;	
ERR:
	if(fp) fclose(fp); 
	if(response) free(response);
	if(headers) free(headers);
	return -1;
}




/*******************************************************************************
*@ Description    :依据 ts_flag 更新 m3u8文件  
*@ Input          :<ts_flag>文件的类型
					<sendok>是否上传OK(1 : ok ; 0 : fail)
					<putname>要推送的文件（名）
					<ts>TS文件描述结构
*@ Output         :
*@ Return         :成功 ： 0 失败：-1
*******************************************************************************/
int  m3u8_file_write(put_file_info_t *info, char sendok)
{	
	if(NULL == info)
	{
		ERROR_LOG("illegal arguemend !\n");
		return -1;
	}
	
	char  tsurl[256]; // m3u 文件上传 amazon云 的url字符串
	char sub_dir[16];//记录子目录（区分是   	 picture/video/video6s）	
	if(info->ts_flag == TS_FLAG_JPG) strcpy(sub_dir,A_JPG_DIR);
	else if(info->ts_flag == TS_FLAG_6S) strcpy(sub_dir,A_VIDEO6s_DIR);
	else strcpy(sub_dir,A_VIDEO_DIR);

	if(url_index != AMAZON_PRODUCE)//非线上（非美国）模式
		snprintf(tsurl,256,"https://%s/%s/%s/%s/%s/%s",g_amazon_info.host,g_amazon_info.bucket,g_amazon_info.deviceid,g_amazon_info.date,sub_dir,info->file_name + strlen("/ramfs/"));
	else //线上（美国）模式
		snprintf(tsurl,256,"%s/%s/%s/%s/%s",AMAZON_QIANMIN,g_amazon_info.deviceid,g_amazon_info.date,sub_dir,info->file_name + strlen(TS_FILE_PATH));
	DEBUG_LOG("....ts_url = %s\n", tsurl);
	

	//写 m3u8 文件
	char tmpstr[128];
	FILE* fn = NULL;
	fn = fopen(info->m3u8name, "a+");//附加的方式打开文件
	if(NULL == fn)
	{
		ERROR_LOG("open file failed !\n");
		return -1;
	}
		
	/*---单个 6s 预览视频的情况--------------------------------*/
	if(info->ts_flag == TS_FLAG_6S)
	{	
		if(sendok == A_PUT_SUCCESS)
		{
			/*---直接6s视频构建成单个的 m3u8 文件----------------- */					
			fputs("#EXTM3U\n", fn);
			fputs("#EXT-X-VERSION:3\n", fn);
			fputs("#EXT-X-TARGETDURATION:10\n", fn);
			fputs("#EXT-X-MEDIA-SEQUENCE:0\n", fn);
			fputs("#EXT-X-ALLOW-CACHE:YES\n\n", fn);//#EXT-X-DISCONTINUITY		
			fputs("#EXT-X-KEY:METHOD=NONE\n", fn);
			fputs("#EXT-X-DISCONTINUITY\n", fn);
			fputs("#EXTINF:6,\n", fn);
			fputs(tsurl, fn);
			fputs("\n", fn);
			fputs("#EXT-X-ENDLIST\n", fn);
		}
		else
		{
			//JPG 推送失败，m3u也不必推送了
		}

	}
	/*---只有一个 TS 文件的情况类似（和video6s的操作类似）-----*/
	else if(info->ts_flag == TS_FLAG_ONE)
	{
		if(sendok == A_PUT_SUCCESS)
		{
			fputs("#EXTM3U\n", fn);
			fputs("#EXT-X-VERSION:3\n", fn);
			fputs("#EXT-X-TARGETDURATION:36\n", fn);
			fputs("#EXT-X-MEDIA-SEQUENCE:0\n", fn);
			fputs("#EXT-X-ALLOW-CACHE:YES\n\n", fn);
			fputs("#EXT-X-KEY:METHOD=NONE\n", fn);
			fputs("#EXT-X-DISCONTINUITY\n", fn);
			snprintf(tmpstr,128, "#EXTINF:%d,\n", info->file_tlen); 
			fputs(tmpstr, fn);
			fputs(tsurl, fn);
			fputs("\n", fn);
			fputs("#EXT-X-ENDLIST\n", fn);	
		}
		else
		{
			//单独一个 TS 文件推送失败，m3u也不必推送了
		}

	}
	/*---有多个 TS文件的情况--------------------------------*/
	else  
	{	
		/*---如果是第一个 TS 文件---------------------------------*/
		if(info->ts_flag == TS_FLAG_START)
		{
			fputs("#EXTM3U\n", fn);
			fputs("#EXT-X-VERSION:3\n", fn);
			fputs("#EXT-X-TARGETDURATION:36\n", fn);
			fputs("#EXT-X-MEDIA-SEQUENCE:0\n", fn);
			fputs("#EXT-X-ALLOW-CACHE:YES\n\n", fn);
			fputs("#EXT-X-KEY:METHOD=NONE\n", fn);
			fputs("#EXT-X-DISCONTINUITY\n", fn);
			if(sendok == A_PUT_SUCCESS)
			{
				snprintf(tmpstr,128, "#EXTINF:%d,\n", info->file_tlen); 
				fputs(tmpstr, fn);
				fputs(tsurl, fn);
				fputs("\n", fn);
			}
			else
			{
				//第一个 TS 文件推送失败
			}

		}
		/*---如果是中间的 TS 文件---------------------------------*/
		else if(info->ts_flag == TS_FLAG_MID)
		{
			if(sendok == A_PUT_SUCCESS)//送成功，将该文件的TS文件名添加到m3u 文件的最后边。
			{
				snprintf(tmpstr,128, "#EXTINF:%d,\n", info->file_tlen); 
				fputs(tmpstr, fn);
				fputs(tsurl, fn);
				fputs("\n", fn);
			}
			else//中间的 TS 文件发送失败，标记后边的媒体文件和之前的媒体文件之间的编码不连续
			{
				fputs("#EXT-X-DISCONTINUITY\n", fn);
			}
		}
		/*---如果是最后一个 TS 文件---------------------------------*/
		else if(info->ts_flag == TS_FLAG_END)
		{
			if(sendok == A_PUT_SUCCESS)//发送成功，写入结束标记
			{
				snprintf(tmpstr,128, "#EXTINF:%d,\n", info->file_tlen); 
				fputs(tmpstr, fn);
				fputs(tsurl, fn);
				fputs("\n", fn);
				//只有成功了才写入结束标记符
				fputs("#EXT-X-ENDLIST\n", fn);
			}
			else
			{
				ERROR_LOG("the last TS file transport failed !\n");
				//最后一个TS文件发送失败,m3u8结束标记符不写，否则如果重传时传输成功，则该段 TS 文件会无法播放
			}
			//fputs("#EXT-X-ENDLIST\n", fn);		
		}
		else
		{
			ERROR_LOG("error TS_FLAG_XXX !\n");
		}
		
	}

	fclose(fn);
	
	return 0;
}


/*******************************************************************************
云服务推送消息参数：
	参数名			必选			类型			说明
	uiid		 是			string		设备uiid
	bucket		 是			string		文件存储桶
	picturePath	 是			string		缩略图上传路径
	video6sPath	 是			string		6秒视频索引文件上传路径
	videoPath	 是			string		完整视频索引文件上传路径
	time		 是			string		视频上传时间 格式： yyyyMMdd-HH:mm:ss

*@ Description    :  构建 post 消息,推送到 amazon 云服务器
*@ Input          :
*@ Output         :
*@ Return         :
*******************************************************************************/
void amazon_send_even_info(put_file_info_t *info)//"/bucket-1-20180131/3db73d9c19b14b0a9964bc97c98ca039/20180608/picture"
{
	//printf("20000000000============amazon_send_even_info!\n");
	char picpath[128];
	char vid6spath[128];
	char vidpath[128];
	char day[32];
	char *target = ipc_get_tutk_guid();
	if(strlen(g_amazon_info.deviceid)<10)return;
	
	time_t timer = time(NULL); 
	struct tm *lct = localtime(&timer);
	snprintf(day,32,"%4d%02d%02d-%02d:%02d:%02d",lct->tm_year+1900,lct->tm_mon+1,lct->tm_mday,lct->tm_hour, lct->tm_min, lct->tm_sec);
	
	cJSON *jpush;
	jpush = cJSON_CreateObject(); 
	
	/*---#设备uiid------------------------------------------------------------*/
	cJSON_AddStringToObject(jpush, "uiid", target);	
	
	/*---#文件存储桶------------------------------------------------------------*/
	cJSON_AddStringToObject(jpush, "bucket", g_amazon_info.bucket);
	
	/*---#缩略图上传路径------------------------------------------------------------*/
	snprintf(picpath,128,"%s/%s/%s/picture/%s",g_amazon_info.bucket, g_amazon_info.deviceid, g_amazon_info.date,info->file_name + 5);
	cJSON_AddStringToObject(jpush, "picturePath", picpath);
	
	/*---#6秒视频索引文件上传路径------------------------------------------------------------*/
	snprintf(vid6spath,128,"%s/%s/%s/video6s/%s",g_amazon_info.bucket, g_amazon_info.deviceid, g_amazon_info.date,info->file_name+5);
	cJSON_AddStringToObject(jpush, "video6sPath", vid6spath);

	if(strstr(info->file_name+5, "m3u8"))
	{
		/*---#完整视频索引文件上传路径------------------------------------------------------------*/
		snprintf(vidpath,128,"%s/%s/%s/video/%s",g_amazon_info.bucket, g_amazon_info.deviceid, g_amazon_info.date,info->file_name+5);
		cJSON_AddStringToObject(jpush, "videoPath", vidpath);
	}
	
	/*---#视频上传时间 格式： yyyyMMdd-HH:mm:ss------------------------------------------------------------*/
	cJSON_AddStringToObject(jpush, "time", day);

	/*---#推送!!!------------------------------------------------------------*/
	char *str = curl_get(jpush, url_add_info_interface[url_index]); 
	cJSON_Delete(jpush);
	free(str);
	
}


/*******************************************************************************
*@ Description    :  往云端推送m3u8文件+TS文件+JPEG+fmp4文件
*@ Input          :<arg>: put_file_info_t 信息
*@ Output         :
*@ Return         :
*******************************************************************************/
void* amazon_put_even(void *arg)
{
	if(NULL == arg)
	{
		ERROR_LOG("Illegal parameter!\n");
		pthread_exit(NULL);
	}
		
	/*---#备份参数------------------------------------------------------------*/
	put_file_info_t file_info;
	memcpy(&file_info,arg,sizeof(put_file_info_t));
	DEBUG_LOG("file_name : %s\n",file_info.file_name);
	int ret;
	static int failnum = 0; //上传失败的TS文件个数
	/*---#分文件类型传输------------------------------------------------------------*/
	if(file_info.file_type == TYPE_JPG)
	{
		if(url_index != AMAZON_PRODUCE)//开发环境/测试环境
			ret = amazon_curl_send(file_info.file_name , file_info.ts_flag ,file_info.file_type); 
		else //美国线上环境
			ret = amazon_curl_send_am(file_info.file_name, file_info.ts_flag ,file_info.file_type);
		if(ret != 0)//put 失败   
		{
			//异常逻辑
		}
	}
	else if(file_info.file_type == TYPE_TS)
	{
		if(url_index != AMAZON_PRODUCE)//开发环境/测试环境
			ret = amazon_curl_send(file_info.file_name, file_info.ts_flag, file_info.file_type); 
		else //美国线上环境
			ret = amazon_curl_send_am(file_info.file_name, file_info.ts_flag, file_info.file_type); 
		if(ret != 0)//put 失败
		{
			if(file_info.ts_flag >=TS_FLAG_START && file_info.ts_flag <= TS_FLAG_END)//有多个连续的ts文件的情况
			{
				m3u8_file_write(&file_info, A_PUT_FAIL);	
				DEBUG_LOG("-1----------------put failed !!!!\n");
			}
			failnum ++;
			ERROR_LOG("curl_send_ts fail:%d,,,%s\n",failnum,file_info.file_name);
		}
		else //put 成功
		{
			/*---该处等待之前的TS文件都上传成功？？？----------------------------------*/
			DEBUG_LOG("amazon_curl_send file success !\n");
			//更新m3u8文件
			m3u8_file_write(&file_info, A_PUT_SUCCESS);

			/*---推送 m3u8 文件----------------------------------------------------------------*/
			if(url_index != AMAZON_PRODUCE)
				ret = amazon_curl_send(file_info.m3u8name, file_info.ts_flag, TYPE_M3U);
			else
				ret = amazon_curl_send_am(file_info.m3u8name, file_info.ts_flag, TYPE_M3U);
			if(ret != 0)
			{
				/*--- m3u8 发送失败，要不要重传？？？------------------------------------------------*/
				printf("curl_send_ts fail:%d,,,%s\n",++failnum,file_info.m3u8name);
			}
			else  //m3u8文件发送成功，构造消息推送到云服务器,告知有新的图片视频文件上传
			{   
				if(file_info.ts_flag == TS_FLAG_START || file_info.ts_flag == TS_FLAG_ONE || g_amazon_info.record_en != 0)
					amazon_send_even_info(&file_info);
			}			
			
		}

	}
	else if(file_info.file_type == TYPE_M3U)
	{
		DEBUG_LOG("not support!\n");
		//m3u8文件已经按照动态生成的方式来重建了。这里不实现
	}
	else if(file_info.file_type == TYPE_FMP4)
	{
		DEBUG_LOG("not support!\n");
		//暂时不支持
	}


	//删除已经发送成功的文件，

	//释放资源
	
	pthread_exit(NULL);
	
}

/*******************************************************************************
*@ Description    :  设备图片视频上传线程
*@ Input          : <put_file> 需要上传的文件（JPEG/video6s/alarm_video）描述信息
*@ Output         :
*@ Return         :
*******************************************************************************/
void amazon_put_even_thread(put_file_info_t *put_file)
{ 
	pthread_t Thread_amazon_put_even_ID;
	if(pthread_create(&Thread_amazon_put_even_ID, NULL, amazon_put_even, (void *)put_file) != 0)
	{
		printf("amazon_put_even_thread: create thread failed; ");
	}
	pthread_detach(Thread_amazon_put_even_ID);
}


/*---#------------------------------------------------------------*/
/*---#------------------------------------------------------------*/
/*---#------------------------------------------------------------*/
/*******************************************************************************
*@ Description    : 推送消息到云服务器（视频图片信息推送接口） 
*@ Input          : <pushMsg> 推送的消息字符串（jason格式）
					<url>云服务的目标 url
*@ Output         :
*@ Return         :amazon 云 的返回信息（字符串指针）使用结束后需要free
*******************************************************************************/
char *curl_get(cJSON *pushMsg, char *url)
{
	char *postData = cJSON_Print(pushMsg);
	DEBUG_LOG("postData=%s;\n",postData);


	
	int ret_code = 0;
	char* ret_buf = NULL;
	char* content_type = "Content-Type: application/json";
	int ret = http_post_data(url,(char*)postData,content_type,&ret_code,&ret_buf);
	if(ret < 0)
	{
		ERROR_LOG("http_post_data failed !\n");
		return NULL;
	}

	#if 1  //DEBUG
		int tmp_fd = open("/jffs0/amazon_reply.txt",O_RDWR|O_CREAT|O_TRUNC,0777);
        if(tmp_fd < 0)
        {
            ERROR_LOG("open failed !\n");
            return NULL;
        };
            
        int tmp_ret = write(tmp_fd, ret_buf ,strlen(ret_buf));
        if(tmp_ret != strlen(ret_buf))
        {
            ERROR_LOG("write error!\n");
            close(tmp_fd);
            return NULL;
        }
        close(tmp_fd);
	#endif
	

	
	DEBUG_LOG("ret_buf=%s\n", ret_buf);
	 
	return ret_buf;
}


/*******************************************************************************
amazon "验证设备图片视频上传接口" 返回示例：
{
  "status": 200,
  "msg": "success",
  "model": {
	  "serverStatus":0, //0:上传缩略图,6s视频,长视频	1:只上传缩略图,6s视频
	  "date":"20180326", //上传日期
	  "host":"http://bucket-1-20180131.s3.amazonaws.com/", //上传域名
	  "bucket":"bucket-1-20180131", //上传桶目录
	  "deviceCode":"eef48c106eb84679b8e7d0ff36ec6449", //设备编码目录
	  "s3AccessKey":"AKIAIMP4QFD37K5F7KKQ", 	   "s3SecretAccessKey":"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"
  }
}


*@ Description    :  解析 amazon 云返回的信息(验证设备图片视频上传接口)
*@ Input          :<str>信息字符串指针
*@ Output         :<rinfo>解析后的数据存放位置
*@ Return         :成功 ： 0	
					失败：-1
*******************************************************************************/
int amazon_info_parse(AMAZON_INFO *rinfo,  const char *str)
{
	if(str == NULL ||rinfo== NULL)
		return -1;
	
	cJSON *jroot = cJSON_Parse(str);
	if(!jroot)
	{
		ERROR_LOG("amazon_info_parse fail!!\n");
		return -1;
	}	
	//memset(rinfo, 0, sizeof(AMAZON_INFO));
	
	cJSON *item, *item1;
	item = cJSON_GetObjectItem(jroot,"status");  //200
	if(item->valueint != 200)
	{ 
		//rinfo->record_en = 1;
		ERROR_LOG("status error!!! ,status=%d\n",item->valueint);
		return -1;
	}
	
	item = cJSON_GetObjectItem(jroot,"model"); 

	item1 = cJSON_GetObjectItem(item,"serverStatus"); //0:上传缩略图,6s视频,长视频  1:只上传缩略图,6s视频
	rinfo->record_en = item1->valueint;
	//if(rinfo->record_en == 1)
		//return 0;
	
	item1 = cJSON_GetObjectItem(item,"date"); //上传日期(需要自己更新的，不用服务器下发的)
	//mystrcpy(rinfo->date, item1->valuestring);  
	item1 = cJSON_GetObjectItem(item,"host"); //上传域名
	strcpy(rinfo->host, item1->valuestring);
	item1 = cJSON_GetObjectItem(item,"bucket"); //上传桶目录
	strcpy(rinfo->bucket, item1->valuestring);
	item1 = cJSON_GetObjectItem(item,"deviceCode"); //设备编码目录
	strcpy(rinfo->deviceid, item1->valuestring);
	item1 = cJSON_GetObjectItem(item,"s3AccessKey"); //公匙
	strcpy(rinfo->aws_access_keyid, item1->valuestring);
	item1 = cJSON_GetObjectItem(item,"s3SecretAccessKey"); //私匙
	strcpy(rinfo->secret_aceess_keyid, item1->valuestring);

	if(strstr(rinfo->host, "cn-")) //域名里边带有cn-，则地区是中国（用的是中国的服务器）
		strcpy(rinfo->regon, "cn-northwest-1");
	else  						   //否则，地区是美国（用的是美国的服务器）
		strcpy(rinfo->regon, "us-east-1"); 
	
	DEBUG_LOG("regon:%s\n", rinfo->regon);
	DEBUG_LOG(".....record_en=%d\n,data=%s\nhost=%s\nbucket=%s\ndeviceid=%s\nkeyid=%s\nsecret=%s\n",
									rinfo->record_en,rinfo->date,rinfo->host,rinfo->bucket,rinfo->deviceid,
									rinfo->aws_access_keyid,rinfo->secret_aceess_keyid);

	cJSON_Delete(jroot);
	return 0;
}

/*******************************************************************************
验证设备图片视频上传接口，上传参数：
	参数名			必选			类型			说明
	uiid		 是			string		设备uiid


*@ Description    :向 “验证设备图片视频上传接口” 推动消息（uiid），获取amazon 云服务的返回信息
					（解析后存放到 g_amazon_info)
*@ Input          :无需参数
*@ Output         :更新 变量 g_amazon_info
*@ Return         :
*******************************************************************************/
void* get_amazon_put_info(void *arg)
{ 
	cJSON *jpush;
	char *target = ipc_get_tutk_guid(); 
	jpush = cJSON_CreateObject(); 
	cJSON_AddStringToObject(jpush, "uiid", target);	 
	char *str = curl_get(jpush, url_verify_up_interface[url_index]); //推送设备的 uiid 到云，获得 amazon 云的返回信息
	cJSON_Delete(jpush); 
	
	if(0!=amazon_info_parse(&g_amazon_info, str))//解析亚马逊云的返回信息，解析到 g_amazon_info 变量
	{
		ERROR_LOG("amazon_info_parse fail!  %s\n",url_verify_up_interface[url_index]);
		return (void *)-1;
	}
	g_amazon_info.today_req = 1;

	
	free(str);
	pthread_exit(NULL);
	
}


/*******************************************************************************
*@ Description    :  验证设备图片视频上传接口 线程(被amazon_S3_req_thread调用)
*@ Input          :
*@ Output         :
*@ Return         :
*******************************************************************************/
void get_amazon_info_thread(void)
{
	pthread_t get_amazon_info_ID;
	if(pthread_create(&get_amazon_info_ID, NULL, get_amazon_put_info, NULL) != 0)
	{
		printf("amazon_S3_thread: create get_amazon_info_thread failed; ");
	}
	pthread_detach(get_amazon_info_ID);
}

extern int motion_detect_get_state(int *state);

/*******************************************************************************
*@ Description    :  负责更新 g_amazon_info （设备图片视频上传接口）的信息
*@ Input          : 无用
*@ Output         : 更新 g_amazon_info
*@ Return         :
*******************************************************************************/
static int amazon_S3_req_running = 0; //标记 线程对否正在运行 0：没有运行 1：正在运行（不重复启动）
void* amazon_S3_req(void *arg)
{
	if(amazon_S3_req_running == 1)
		pthread_exit(NULL);
	
	amazon_S3_req_running = 1;
	time_t timer;
	struct tm *lct;
	int i=0,j=0;
	int k=0;		//图片视频上传接口更新标志 0：需要更新   				1：已经更新
	int motion = 0; //移动侦测告警标志
	//sleep(30);
	//gInDebug = 1;//mytest...
	get_amazon_info_thread();  //初次启动需要先验证设备图片视频上传接口（获取服务器返回信息）。 

	timer = time(NULL); 
	lct = localtime(&timer);//更新设备信息更新时间
	snprintf(g_amazon_info.date,12,"%4d%02d%02d",lct->tm_year+1900,lct->tm_mon+1,lct->tm_mday);

	while(1)//更新策略可改，不一定这样,每天的第一条告警推送的时候进行更新
	{
		timer=time(NULL); 
		lct = localtime(&timer);
		
		motion_detect_get_state(&motion);
		
		/*---当标志复位后 + 产生移动告警 才会更新 （图片视频上传接口）的相关信息------*/
		if(g_amazon_info.today_req ==0 && motion!=0 && k==0)
		{
			printf("1000000000000000008 get_amazon_info_thread\n");
			k=1;
			get_amazon_info_thread();
		}

		
		if(i++ >= 30)  //1min进入一次 （30 * 2 = 60s ）
		{
			i = 0;
			/*---------------------------------------------------------
			每天凌晨0:00 开始的前200秒内会将标志位进行一次复位，
			标记 云服务器（图片视频上传接口）的相关信息 需要更新
			---------------------------------------------------------*/
			if((lct->tm_hour*3600 + lct->tm_min*60 + lct->tm_sec) < 200)
			{
				if(j == 0)//j用来控制标记只会复位一次
				{
					k = 0; //标记 “图片视频上传接口” 信息需要更新
					g_amazon_info.today_req = 0;
					j = 1;
				}
			}
			else  //其余时间将标记置零
			{
				j=0;	
			}
						
		}

		if(lct->tm_min == 0)//整分钟时间，更新一次日期信息
		{
			snprintf(g_amazon_info.date,12,"%4d%02d%02d",lct->tm_year+1900,lct->tm_mon+1,lct->tm_mday);			
		}


		
		motion=0;
		sleep(2);
	}
	
	amazon_S3_req_running = 0;
	pthread_exit(NULL);
	
}


/*******************************************************************************
*@ Description    :  负责更新 g_amazon_info （设备图片视频上传接口）的信息
*@ Input          :
*@ Output         :
*@ Return         :
*******************************************************************************/
void amazon_S3_req_thread(void)
{
	pthread_t Thread_amazon_S3_req_ID;
	if(pthread_create(&Thread_amazon_S3_req_ID, NULL, amazon_S3_req, NULL) != 0)
	{
		printf("amazon_S3_thread: create ipc_jpush_thread failed; ");
	}
	pthread_detach(Thread_amazon_S3_req_ID);
}












