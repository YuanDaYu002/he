#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/hmac.h>  
#include <openssl/sha.h>  

#include "curl/curl.h"
#include<time.h>
#include<unistd.h>
#include "cJSON/cJSON.h"
#include <sys/stat.h>
#include "tfmanager.h"
#include "amazom_S3.h"
#include <errno.h>
#include "common_plus.h"
#include<ctype.h> 

#define A_PUT_SUCCESS 1
#define A_PUT_FAIL 0

#define A_JPG_DIR		"picture"
#define A_VIDEO_DIR		"video"
#define A_VIDEO6s_DIR	"video6s"

#define AWS_REGON 	"cn-northwest-1"//   "us-east-1"// "cn-northwest-1"//   
#define AWS_HOST	"s3.cn-northwest-1.amazonaws.com.cn" //"s3.amazonaws.com" //"s3.cn-northwest-1.amazonaws.com.cn" //
#define AWS_ACCESS_KEY_ID 	"AKIAPFUPG7ABHT3W5P4Q"  //"AKIAIMP4QFD37K5F7KKQ"  //  "AKIAPFUPG7ABHT3W5P4Q"  //
#define AWS_ACCESS_SECRET	"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" //"wKONXbuGA7hBmvkjwycdynRNzsjyBbFI8E59vEif"  //  


#define AMAZON_QIANMIN	"http://d285s66sa0r9bx.cloudfront.net"

static AMAZON_INFO g_amazon_info = {0,-1," "," "," "," "," "," "," "," ", " "}; //  {1,0,"20181029",AWS_ACCESS_KEY_ID,AWS_ACCESS_SECRET,AWS_HOST," "," ","123456","bucket-1-20180131",AWS_REGON};//
//"https://s3.amazonaws.com/bucket-1-20180131/XZ1T6R2RBRP7PH7P111A/test6/20170721-140120.jpg",
//"/bucket-1-20180131/XZ1T6R2RBRP7PH7P111A/test6/20170721-140120.jpg", 

static char aws_yymmdd[12];  
static char aws_TimeStamp[20];


const char base[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="; 
char* base64_encode1(const char* data, int data_len); 
char *base64_decode1(const char* data, int data_len); 
static char find_pos(char ch); 
extern char *ipc_get_tutk_guid(void);
extern char *url_get_put[3];
extern char *url_post_put[3];
extern int gInDebug;

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

static char find_pos(char ch)   
{ 
    char *ptr = (char*)strrchr(base, ch);//the last position (the only) in base[] 
    return (ptr - base); 
} 
/* */ 
char *base64_decode1(const char *data, int data_len) 
{ 
    int ret_len = (data_len / 4) * 3; 
    int equal_count = 0; 
    char *ret = NULL; 
    char *f = NULL; 
    int tmp = 0; 
    int temp = 0; 
    char need[3]; 
    int prepare = 0; 
    int i = 0; 
    if (*(data + data_len - 1) == '=') 
    { 
        equal_count += 1; 
    } 
    if (*(data + data_len - 2) == '=') 
    { 
        equal_count += 1; 
    } 
    if (*(data + data_len - 3) == '=') 
    {//seems impossible 
        equal_count += 1; 
    } 
    switch (equal_count) 
    { 
    case 0: 
        ret_len += 4;//3 + 1 [1 for NULL] 
        break; 
    case 1: 
        ret_len += 4;//Ceil((6*3)/8)+1 
        break; 
    case 2: 
        ret_len += 3;//Ceil((6*2)/8)+1 
        break; 
    case 3: 
        ret_len += 2;//Ceil((6*1)/8)+1 
        break; 
    } 
    ret = (char *)malloc(ret_len); 
    if (ret == NULL) 
    { 
        printf("No enough memory.\n"); 
        exit(0); 
    } 
    memset(ret, 0, ret_len); 
    f = ret; 
    while (tmp < (data_len - equal_count)) 
    { 
        temp = 0; 
        prepare = 0; 
        memset(need, 0, 4); 
        while (temp < 4) 
        { 
            if (tmp >= (data_len - equal_count)) 
            { 
                break; 
            } 
            prepare = (prepare << 6) | (find_pos(data[tmp])); 
            temp++; 
            tmp++; 
        } 
        prepare = prepare << ((4-temp) * 6); 
        for (i=0; i<3 ;i++ ) 
        { 
            if (i == temp) 
            { 
                break; 
            } 
            *f = (char)((prepare>>((2-i)*8)) & 0xFF); 
            f++; 
        } 
    } 
    *f = '\0'; 
    return ret; 
}

int amazon_put_even_enable(void)
{
	return g_amazon_info.record_en;
}


/*******************************************************************************
*@ Description    :  合成完整的文件上传 amazon URL 
*@ Input          :
					path：
					url:
					type:
					finename:
*@ Output         :
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
	strcat(path,finename+strlen("/tmp/"));// "/tmp/150108.mkv"

	strcpy(url,"https://");
	strcat(url,g_amazon_info.host); // "https://s3.amazonaws.com/");//
	strcat(url, path);//path+1

	return 0;
}


int amazon_put_enable(void)
{
	g_amazon_info.record_en = 0;
	return 0;
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


size_t amazonWriteData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t data_size = size * nmemb;
	if(data_size<1024)
    	strcat((char *)stream, (char *)ptr);  
    return data_size;
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

void  str_tolower(char *string, int len, char *out)
{
	int i;
	for(i=0;i<len;i++){
		out[i] = tolower(string[i]);
	}
	printf("out=%s\n",out);
}

void make_hex(unsigned char *input, int inlen, char *out_hex)
{
    int i = 0;  
    char tmp[3] = {0}; 
	out_hex[0] = '\0';
    for(i = 0; i < inlen; i++ )  
    {	sprintf(tmp,"%02x", input[i]);  
        strcat(out_hex, tmp);  
    }  
    //printf("output==%s\n", out_hex);
}


void make_StringToSign(const char *http_req, char *stringToSign)
{
	unsigned char sha[33] = {0}; 
	char hex[65] = {0};
	SHA256((unsigned char*)http_req, strlen(http_req), sha);
	make_hex(sha ,32, hex);

	
	strcpy(stringToSign, "AWS4-HMAC-SHA256\n");
	strcat(stringToSign, aws_TimeStamp);
	strcat(stringToSign, "\n");
	char Scope[64];
	sprintf(Scope, "%s/%s/s3/aws4_request\n", aws_yymmdd, g_amazon_info.regon);
	strcat(stringToSign, Scope);
	strcat(stringToSign, hex);
	//printf("stringToSign==%s\n",stringToSign);
}

void make_signatue(char *YourSecretAccessKeyID, char *StringToSign, char *Signature)//SECRET_ACCESS_KEYID
{
	int len = 0;

	unsigned char Hamc_sha1[36];  unsigned char Hamc_sha2[36];
	const EVP_MD * engine = NULL;
	engine = EVP_sha256();
	HMAC_CTX ctx; 

	char AWS4_key[64]; strcpy(AWS4_key, "AWS4"); strcat(AWS4_key, YourSecretAccessKeyID); 
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

void make_Authorization(unsigned char *Authorization, unsigned char *Signature )
{
	//char  out[24] = {0};
	//str_tolower(g_amazon_info.aws_access_keyid, strlen(g_amazon_info.aws_access_keyid), out);
	sprintf(Authorization,"Authorization: AWS4-HMAC-SHA256 "
	"Credential=%s/%s/%s/s3/aws4_request, "
	"SignedHeaders=content-type;host;x-amz-date;x-amz-acl, "
	"Signature=%s",
	g_amazon_info.aws_access_keyid, aws_yymmdd, g_amazon_info.regon,
	Signature);
}


size_t curlGetWriteData(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t data_size = size * nmemb;
    strcat((char *)stream, (char *)ptr);
    return data_size;
}


char *curl_get(cJSON *pushMsg, char *url)
{
	char *postData = cJSON_Print(pushMsg);
	printf("postData=%s;\n%s\n",postData,url);
	CURL *curl;
	char *str = (char *)calloc(1024, sizeof(char));

	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, "Content-Type: application/json");

	curl = curl_easy_init();
 
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postData); 
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, amazonWriteData); 
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str); 
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1); 
	int ret = curl_easy_perform(curl); 
	curl_easy_cleanup(curl); 
	printf("000 curl_get ret=%d str=%s\n",ret, str);
	 
	return str;
}



void amazon_send_even_info(ts_info *ts)//"/bucket-1-20180131/3db73d9c19b14b0a9964bc97c98ca039/20180608/picture"
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
	sprintf(day,"%4d%02d%02d-%02d:%02d:%02d",lct->tm_year+1900,lct->tm_mon+1,lct->tm_mday,lct->tm_hour, lct->tm_min, lct->tm_sec);
	
	cJSON *jpush;
	jpush = cJSON_CreateObject(); 
	cJSON_AddStringToObject(jpush, "uiid", target);	 
	cJSON_AddStringToObject(jpush, "bucket", g_amazon_info.bucket);
	sprintf(picpath,"%s/%s/%s/picture/%s",g_amazon_info.bucket, g_amazon_info.deviceid, g_amazon_info.date,ts->jpg+5);
	cJSON_AddStringToObject(jpush, "picturePath", picpath);
	
	sprintf(vid6spath,"%s/%s/%s/video6s/%s",g_amazon_info.bucket, g_amazon_info.deviceid, g_amazon_info.date,ts->m3u8_6s+5);
	cJSON_AddStringToObject(jpush, "video6sPath", vid6spath);

	if(strstr(ts->m3u8name+5, "m3u8")){
		sprintf(vidpath,"%s/%s/%s/video/%s",g_amazon_info.bucket, g_amazon_info.deviceid, g_amazon_info.date,ts->m3u8name+5);
		cJSON_AddStringToObject(jpush, "videoPath", vidpath);
	}
	
	cJSON_AddStringToObject(jpush, "time", day);
	char *str = curl_get(jpush, url_post_put[gInDebug]); 
	cJSON_Delete(jpush); 	
}

char *mystrcpy(char *dest, char *src)
{
	if(dest && src)
	{
		return strcpy(dest,src);
	}
	return dest;
}

int amazon_info_parse(AMAZON_INFO *rinfo,  const char *str)
{
	if(str == NULL ||rinfo== NULL)
		return -1;
	cJSON *jroot = cJSON_Parse(str);
	if(!jroot)
	{
		printf("amazon_info_parse fail!!\n");
		return -1;
	}	
	//memset(rinfo, 0, sizeof(AMAZON_INFO));
	
	cJSON *item, *item1;
	item = cJSON_GetObjectItem(jroot,"status");
	if(item->valueint != 200)
	{ 
		//rinfo->record_en = 1;
		printf("amazon_info_parse fail,status=%d\n",item->valueint);
		return -1;
	}
	
	item = cJSON_GetObjectItem(jroot,"model"); 

	item1 = cJSON_GetObjectItem(item,"serverStatus");
	rinfo->record_en = item1->valueint;
	//if(rinfo->record_en == 1)
		//return 0;
	
	item1 = cJSON_GetObjectItem(item,"date"); 
	//mystrcpy(rinfo->date, item1->valuestring);  
	item1 = cJSON_GetObjectItem(item,"host"); 
	mystrcpy(rinfo->host, item1->valuestring);
	item1 = cJSON_GetObjectItem(item,"bucket"); 
	mystrcpy(rinfo->bucket, item1->valuestring);
	item1 = cJSON_GetObjectItem(item,"deviceCode"); 
	mystrcpy(rinfo->deviceid, item1->valuestring);
	item1 = cJSON_GetObjectItem(item,"s3AccessKey"); 
	mystrcpy(rinfo->aws_access_keyid, item1->valuestring);
	item1 = cJSON_GetObjectItem(item,"s3SecretAccessKey"); 
	mystrcpy(rinfo->secret_aceess_keyid, item1->valuestring);

	if(strstr(rinfo->host, "cn-")) strcpy(rinfo->regon, "cn-northwest-1");
	else  strcpy(rinfo->regon, "us-east-1");
	//printf("regon:%s\n", rinfo->regon);
	//printf("...........record_en=%d\n,data=%s\nhost=%s\nbucket=%s\ndeviceid=%s\nkeyid=%s\nsecret=%s\n",
		//rinfo->record_en,rinfo->date,rinfo->host,rinfo->bucket,rinfo->deviceid,
		//rinfo->aws_access_keyid,rinfo->secret_aceess_keyid);

	cJSON_Delete(jroot);
	return 0;
}



void* get_amazon_put_info(void *arg)
{ 
	cJSON *jpush;
	char *target = ipc_get_tutk_guid(); 
	jpush = cJSON_CreateObject(); 
	cJSON_AddStringToObject(jpush, "uiid", target);	 
	char *str = curl_get(jpush, url_get_put[gInDebug]); 
	cJSON_Delete(jpush); 
	
	if(0!=amazon_info_parse(&g_amazon_info, str))
	{
		printf("amazon_info_parse fail!  %s\n",url_get_put[gInDebug]);
		return (void *)-1;
	}
	g_amazon_info.today_req = 1;

	
	free(str);
}


void get_amazon_info_thread(void)
{
	pthread_t get_amazon_info_ID;
	if(pthread_create(&get_amazon_info_ID, NULL, get_amazon_put_info, NULL) != 0)
	{
		printf("amazon_S3_thread: create get_amazon_info_thread failed; ");
	}
	pthread_detach(get_amazon_info_ID);
}



void* amazon_put_even(void *arg);
void* amazon_S3_req(void *arg)
{
	time_t timer;
	struct tm *lct;
	int i=0,j=0,k=0;
	int motion = 0; 
	//sleep(30);
	//gInDebug = 1;//mytest...
	get_amazon_info_thread();

	timer=time(NULL); 
	lct = localtime(&timer);
	sprintf(g_amazon_info.date,"%4d%02d%02d",lct->tm_year+1900,lct->tm_mon+1,lct->tm_mday);

	while(1)
	{
		timer=time(NULL); 
		lct = localtime(&timer);
		
		motion_detect_get_state(&motion);
		if(g_amazon_info.today_req ==0 && motion!=0 && k==0)
		{printf("1000000000000000008 get_amazon_info_thread\n");
			k=1;
			get_amazon_info_thread();
		}

		
		if(i++ >= 30)
		{
			i = 0;
			if((lct->tm_hour*3600 + lct->tm_min*60 + lct->tm_sec) < 200)
			{
				if(j == 0)
				{
					k = 0;
					g_amazon_info.today_req = 0;
					j = 1;
				}
			}
			else
				j=0;			
		}

		if(lct->tm_min == 0)
		{
			sprintf(g_amazon_info.date,"%4d%02d%02d",lct->tm_year+1900,lct->tm_mon+1,lct->tm_mday);			
		}


		
		motion=0;
		sleep(2);
	}
}


void amazon_S3_req_thread(void)
{
	pthread_t Thread_amazon_S3_req_ID;
	if(pthread_create(&Thread_amazon_S3_req_ID, NULL, amazon_S3_req, NULL) != 0)
	{
		printf("amazon_S3_thread: create ipc_jpush_thread failed; ");
	}
	pthread_detach(Thread_amazon_S3_req_ID);
}


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

int amazon_curl_send_am(char *filename, char tsflag, char type)//g_amazon_info.jpg_filename, "picture", image/jpeg
{	
	//printf("1000000013 ,%s, %d\n",filename,tsflag);
	char filetype[16];
	char sub_dir[16];
	char *str = (char *)calloc(1024*100, sizeof(char));
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
	else
	{
		strcpy(sub_dir,A_VIDEO_DIR);
		if(type==TYPE_TS)
			strcpy(filetype,"video/mp2t");//x-mpeg
		else
			strcpy(filetype,"audio/mpegurl");
		timeout = 150;
	}
	amazon_info_complet(path, url, sub_dir, filename);
	 //printf("1000011filename=%s,type=%s,path=%s\n url=%s\n%s,id=%s,key=%s\n",filename,sub_dir, path, url,
	 	//filetype,g_amazon_info.aws_access_keyid,g_amazon_info.secret_aceess_keyid);

	FILE *fp = fopen(filename, "r"); 
	if (fp == NULL)
	{
		//remove(filename);	
		printf("open path==%s fail,%d\n",filename,errno);
		return ;
	}	
	unsigned long fsize = get_file_size(filename);
	

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

	if(type==2)strcat(Stringtosign,"x-amz-acl:public-read\n");//test	
	
	char CanonicalizedResource[256];
	CanonicalizedResource[0] = '\0';
	strcat(CanonicalizedResource,path);	 
	strcat(Stringtosign,CanonicalizedResource);	
	//printf("##########Stringtosign=#########\n%s\n#####################################\n", Stringtosign);

	char *Signature = NULL;
	Signature = make_signatue_am(g_amazon_info.secret_aceess_keyid,Stringtosign);
	
	char Authorization[1024];
	Authorization[0] = '\0';
	strcpy(Authorization,"Authorization: AWS ");
	strcat(Authorization,g_amazon_info.aws_access_keyid);
	strcat(Authorization,":");
	strcat(Authorization,Signature);
	free(Signature);
	//printf("len=%d   %s\n",strlen(Authorization),Authorization);

	char append[128];
	strcpy(append, "Content-Type: ");
	strcat(append,filetype);	
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, append);//"Content-Type: image/jpeg")


	char contentlen[64];
	sprintf(contentlen,"Content-Length: %ld", fsize);
	headers = curl_slist_append(headers, g_amazon_info.host);//"s3.amazonaws.com"); //bucket-1-20180131.s3.cn-northwest-1.amazonaws.com.cn
	//printf("host...................%s\n",g_amazon_info.host);
	
	char time_str1[64];
	strcpy(time_str1,"Date: ");
	strcat(time_str1,time_str);
	headers = curl_slist_append(headers, time_str1);
	headers = curl_slist_append(headers, Authorization); 
	
	if(type==2)headers = curl_slist_append(headers, "x-amz-acl: public-read");
	headers = curl_slist_append(headers, "Connection: Keep-Alive");
	headers = curl_slist_append(headers, "Expect: 100-continue");
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);   
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); 
	
	curl_easy_setopt(curl, CURLOPT_URL, url);//url
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_PUT, 1L); 
	curl_easy_setopt(curl,  CURLOPT_INFILE, fp); 
	curl_easy_setopt(curl,  CURLOPT_INFILESIZE, fsize);  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, amazonWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	int ret= curl_easy_perform(curl);
	curl_easy_cleanup(curl); 

	fclose(fp); 
	printf("==return:%d  str:%s\n", ret, str);
	free(str);

	return ret;// str;	
}

/*******************************************************************************
*@ Description    :  
*@ Input          :
*@ Output         :
*@ Return         :
*******************************************************************************/
int amazon_curl_send(char *filename, char tsflag, char type)//g_amazon_info.jpg_filename, "picture", image/jpeg
{	
	//printf("1000000013 ,%s, %d\n",filename,tsflag);
	char filetype[16];
	char sub_dir[16];
	char *str = (char *)calloc(1024*100, sizeof(char));
	char path[256];
	char http_url[512];	
	int timeout;

	time_t timer=time(NULL); 
	struct tm *gmt = gmtime(&timer);
	sprintf(aws_yymmdd, "%04d%02d%02d", 1900+gmt->tm_year, 1+gmt->tm_mon, gmt->tm_mday);
	sprintf(aws_TimeStamp, "%04d%02d%02dT%02d%02d%02dZ", 1900+gmt->tm_year, 1+gmt->tm_mon, gmt->tm_mday, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
	//printf("date =%s, time=%s\n",aws_yymmdd, aws_TimeStamp);


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
	else
	{
		strcpy(sub_dir,A_VIDEO_DIR);
		if(type==TYPE_TS)
			strcpy(filetype,"video/mp2t");//x-mpeg
		else
			strcpy(filetype,"audio/mpegurl");
		timeout = 150;
	}
	amazon_info_complet(path, http_url, sub_dir, filename);

	//printf("1000011filename=%s,type=%s,path=%s\n url=%s\n%s,id=%s,key=%s\n",filename,sub_dir, path, http_url,
	 	//filetype,g_amazon_info.aws_access_keyid,g_amazon_info.secret_aceess_keyid,http_url);
	//printf("host:%s\n url:%s\n",g_amazon_info.host,http_url);
	
	FILE *fp = fopen(filename, "r"); 
	if (fp == NULL)
	{
		//remove(filename);	
		printf("open path==%s fail,%d\n",filename,errno);
		return ;
	}	
	unsigned long fsize = get_file_size(filename);
	
	////////////////////////////////////////////////////////////////aws
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
	char stringToSign[256] = {0};
	make_StringToSign(http_req , stringToSign);
	
	//////////////3  Signature
	unsigned char Signature[65] = {0};
	make_signatue(g_amazon_info.secret_aceess_keyid, stringToSign, Signature);
	
	///////////////4 Authorization
	char Authorization[1024];	Authorization[0] = '\0';
	make_Authorization(Authorization, Signature);
	//printf("Authorization////////////////////\n%s\n///////////\n", Authorization);

	char append[128];
	strcpy(append, "Content-Type: ");
	strcat(append,filetype);	
	struct curl_slist *headers = NULL;
	headers = curl_slist_append(headers, append);//"Content-Type: image/jpeg")

	char contentlen[64];
	sprintf(contentlen,"Content-Length: %ld", fsize);
	headers = curl_slist_append(headers, g_amazon_info.host);//"s3.amazonaws.com"); //bucket-1-20180131.s3.cn-northwest-1.amazonaws.com.cn
	//printf("host...................%s\n",g_amazon_info.host);


	char time_str1[64];
	strcpy(time_str1,"Date: ");
	strcat(time_str1,time_str);
	headers = curl_slist_append(headers, time_str1);
	
	char aws_date[64];
	strcpy(aws_date,"X-Amz-Date: ");//X-Amz-Date: 20150830T123600Z
	strcat(aws_date,aws_TimeStamp);
	headers = curl_slist_append(headers, aws_date);
	
	headers = curl_slist_append(headers, "x-amz-acl: public-read");
	headers = curl_slist_append(headers, "x-amz-content-sha256: UNSIGNED-PAYLOAD");
	//headers = curl_slist_append(headers, Authorization); 

	headers = curl_slist_append(headers, "Connection: Keep-Alive");
	headers = curl_slist_append(headers, "Expect: 100-continue");
	CURL *curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);   
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); 

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
	curl_easy_setopt(curl,  CURLOPT_INFILE, fp); 
	curl_easy_setopt(curl,  CURLOPT_INFILESIZE, fsize);  
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, amazonWriteData);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, str);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L); 
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
	int ret= curl_easy_perform(curl);
	curl_easy_cleanup(curl); 

	fclose(fp); 
	printf("==return:%d\n==file=%s\n==str:%s\n", ret,filename, str);
	free(str);

	return ret;// str;	
}



void ts_info_set(ts_info *ts, char flag, int tlen, char *fname)
{
	ts->ts_flag = flag;//-1 jgg; 0 6s; 1 start;  2 mid   3 end
	ts->ts_tlen = tlen; //recordtime
	strcpy(ts->file_name,fname);
	
	strncpy(ts->datetime,fname+strlen("/tmp/"),FRAND_LEN);
	ts->datetime[FRAND_LEN] = '\0';
	
	if(flag == TS_FLAG_JPG)
		strcpy(ts->jpg, fname);
	else if(flag == TS_FLAG_6S)//m3u8
	{
		strcpy(ts->m3u8_6s, fname);
		char *p = strstr(ts->m3u8_6s, ".mkv");
		strcpy(p, ".m3u8");
	}
	else if(flag == TS_FLAG_START || flag == TS_FLAG_ONE)
	{
		strcpy(ts->m3u8name, fname);
		char *p = strstr(ts->m3u8name, ".mkv");
		strcpy(p, ".m3u8");
	}

	if(flag>20 && flag<24)//start mid end
	{
		ts->record_oder++;
	}
}


void m3u8_file_write(ts_info *ts, char *putname, char ts_flag, char sendok)//sendok 1 ok;   0 fail
{		
	char  tsurl[256];
	char sub_dir[16];	
	if(ts_flag == TS_FLAG_JPG) strcpy(sub_dir,A_JPG_DIR);
	else if(ts_flag == TS_FLAG_6S) strcpy(sub_dir,A_VIDEO6s_DIR);
	else strcpy(sub_dir,A_VIDEO_DIR);

	if(gInDebug != 0)
		sprintf(tsurl,"https://%s/%s/%s/%s/%s/%s",g_amazon_info.host,g_amazon_info.bucket,g_amazon_info.deviceid,g_amazon_info.date,sub_dir,putname+strlen("/tmp/"));
	else
		sprintf(tsurl,"%s/%s/%s/%s/%s",AMAZON_QIANMIN,g_amazon_info.deviceid,g_amazon_info.date,sub_dir,putname+strlen("/tmp/"));
	//printf("....ts_url=%s\n", tsurl);
	

	//write m3u8
	char tmpstr[128];
	if(ts_flag == TS_FLAG_6S)
	{
		FILE* fn = fopen(ts->m3u8_6s, "a+");
		if(NULL == fn) return -1;		
		fputs("#EXTM3U\n", fn);
		fputs("#EXT-X-VERSION:3\n", fn);
		fputs("#EXT-X-TARGETDURATION:10\n", fn);
		fputs("#EXT-X-MEDIA-SEQUENCE:0\n", fn);
		fputs("#EXT-X-ALLOW-CACHE:YES\n\n", fn);//#EXT-X-DISCONTINUITY		
		fputs("#EXT-X-KEY:METHOD=NONE\n", fn);
		fputs("#EXT-X-DISCONTINUITY\n", fn);
		fputs("#EXTINF:6,\n", fn);
		fputs(tsurl, fn);fputs("\n", fn);
		fputs("#EXT-X-ENDLIST\n", fn);
		fclose(fn);
	}
	else
	{
		FILE* fn = fopen(ts->m3u8name, "a+");
		if(NULL == fn) return -1;
		
		if(ts_flag == TS_FLAG_START)
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
				sprintf(tmpstr, "#EXTINF:%d,\n", ts->ts_tlen); 
				fputs(tmpstr, fn);
				fputs(tsurl, fn);fputs("\n", fn);
			}

		}
		else if(ts_flag == TS_FLAG_MID)
		{
			if(sendok == A_PUT_SUCCESS)
			{
				sprintf(tmpstr, "#EXTINF:%d,\n", ts->ts_tlen); 
				fputs(tmpstr, fn);
				fputs(tsurl, fn);fputs("\n", fn);
			}
			else
			{
				fputs("#EXT-X-DISCONTINUITY\n", fn);
			}
		}
		
		else if(ts_flag == TS_FLAG_END)
		{
			if(sendok == A_PUT_SUCCESS)
			{
				sprintf(tmpstr, "#EXTINF:%d,\n", ts->ts_tlen); 
				fputs(tmpstr, fn);
				fputs(tsurl, fn);fputs("\n", fn);
			}
			fputs("#EXT-X-ENDLIST\n", fn);		
		}
		else if(ts_flag == TS_FLAG_ONE)
		{
			fputs("#EXTM3U\n", fn);
			fputs("#EXT-X-VERSION:3\n", fn);
			fputs("#EXT-X-TARGETDURATION:36\n", fn);
			fputs("#EXT-X-MEDIA-SEQUENCE:0\n", fn);
			fputs("#EXT-X-ALLOW-CACHE:YES\n\n", fn);
			fputs("#EXT-X-KEY:METHOD=NONE\n", fn);
			fputs("#EXT-X-DISCONTINUITY\n", fn);
			sprintf(tmpstr, "#EXTINF:%d,\n", ts->ts_tlen); 
			fputs(tmpstr, fn);
			fputs(tsurl, fn);fputs("\n", fn);
			fputs("#EXT-X-ENDLIST\n", fn);	
		}

		fclose(fn);

		//debug 
		char cmd[256];
		sprintf(cmd,"cat %s",ts->m3u8name);		
		printf("#########  start   ############### tsflag=%d, %s\n",ts_flag,ts->m3u8name);
		system(cmd);//test
		printf("#########   end    ############### \n");	
	}
}


void remove_tmp(ts_info *ts, char *putname, void *arg)
{
	//remov /tmp/	   ex. /tmp/121915.mkv
	char cmd[256];
	sprintf(cmd, "rm -rf /tmp/%s*", ts->datetime); 
	if(ts->ts_flag==TS_FLAG_JPG)remove(ts->file_name); 
	else if(ts->ts_flag==TS_FLAG_6S){system(cmd);} //.mkv .ts .m3u8
	else if(ts->ts_flag==TS_FLAG_START){remove(ts->file_name);remove(putname);}  
	else if(ts->ts_flag==TS_FLAG_MID){system(cmd);} 
	else if(ts->ts_flag==TS_FLAG_END){system(cmd);remove(ts->m3u8name);if(arg)free(arg);}
	else if(ts->ts_flag==TS_FLAG_ONE){system(cmd);if(arg)free(arg);}
	else {system(cmd);}
}


void* amazon_put_even(void *arg)
{	 
	static int failnum = 0;static int totalnum = 0;
	char cmd[256];
	char putname[64];
	char putm3u8[64];
	ts_info ts;
	ts_info *ts_p = (ts_info *)arg;
	char type = TYPE_JPG;//0 jpg, 1 mkv, 2 ts, 3 m3u8
	memcpy(&ts, (ts_info *)arg, sizeof(ts_info));
	strcpy(putname,ts.file_name);
	//printf("...................%d; %s ;%d,%d\n",ts.ts_flag,putname,ts.offset,  ts.ts_tlen);
	int ret;
	if(ts.ts_flag == TS_FLAG_JPG)	//jpg
	{
		type = TYPE_JPG;
		if(gInDebug != 0)
			ret = amazon_curl_send(putname, ts.ts_flag ,type);
		else //america
			ret = amazon_curl_send_am(putname, ts.ts_flag ,type);
		
		if(ret != 0)printf("curl_send_ts fail:%d,,,%s\n",++failnum,putname);		
	}
	else
	{	
		char *p=strstr(putname,".mkv");
		if(p != NULL)strcpy(p, ".ts");//printf("10000020 =%d \n",ts.ts_flag);
		//sprintf(cmd, "nohup ./ffmpeg -itsoffset %d -i %s -acodec copy -vcodec copy -f mpegts %s >/dev/null 2>&1",ts.offset, ts.file_name, putame); 
		//nohup ./ffmpeg -itsoffset 0 -i 200952.mkv -vcodec copy -acodec mp3 -ar 8000 -ac 1 -f mpegts 200952.ts >/dev/null 2>&1
		//printf("offset=%d,in=%s, out=%s\n",ts.offset, ts.file_name, putname);
		sprintf(cmd, "nohup ./ffmpeg -itsoffset %d -i %s -vcodec copy -acodec mp3 -ar 32000 -ac 1 -b:a 64k -f mpegts %s >/dev/null 2>&1",ts.offset, ts.file_name, putname); 
		system(cmd);
		sleep(2); 

		type = TYPE_TS;
		if(gInDebug != 0)
			ret = amazon_curl_send(putname, ts.ts_flag, type); 
		else
			ret = amazon_curl_send_am(putname, ts.ts_flag, type); 
		
		if(ret != 0)//put fail
		{
			if(ts.ts_flag>20 && ts.ts_flag<24)//start mid end
			{
				m3u8_file_write(&ts, putname, ts.ts_flag, A_PUT_FAIL);	
				ts_p->m3u8_oder++;
				//printf("-1----------------record_oder=%d, m3u8_oder=%d\n",ts.record_oder, ts_p->m3u8_oder);
			}
			printf("curl_send_ts fail:%d,,,%s\n",++failnum,putname);
		}
		else 
		{ 
			//if(ts.ts_flag == TS_FLAG_END){sleep(40);}
			if(ts.ts_flag>20 && ts.ts_flag<24)
			{
				int i = 0;
				while(ts.record_oder > (ts_p->m3u8_oder+1))
				{
					printf("wait to write m3u !\n");
					sleep(2);
					if(i++>100)break;
				}
			}
			
			//m3u
			m3u8_file_write(&ts, putname, ts.ts_flag, A_PUT_SUCCESS);//writ/send
			if(ts.ts_flag>20 && ts.ts_flag<24)//start mid end
			{
				ts_p->m3u8_oder++;
				//printf("0----------------record_oder=%d, m3u8_oder=%d\n",ts.record_oder, ts_p->m3u8_oder);
			}


			if(ts.ts_flag == TS_FLAG_6S)
				{strcpy(putm3u8,ts.m3u8_6s);} 
			else
				{strcpy(putm3u8,ts.m3u8name);} 
			type = TYPE_M3U;
			if(gInDebug != 0)
				ret = amazon_curl_send(putm3u8, ts.ts_flag, type);
			else
				ret = amazon_curl_send_am(putm3u8, ts.ts_flag, type);
			
			if(ret != 0)printf("curl_send_ts fail:%d,,,%s\n",++failnum,putname);
			else
			{   
				if(ts.ts_flag == TS_FLAG_START || ts.ts_flag == TS_FLAG_ONE || g_amazon_info.record_en != 0)
					amazon_send_even_info(&ts);
			}			
		} 
	}
	
#if 1 //test
	//remov /tmp/      ex. /tmp/121915.mkv
	sprintf(cmd, "rm -rf /tmp/%s*", ts.datetime); 
	if(ts.ts_flag==TS_FLAG_JPG)remove(ts.file_name); 
	else if(ts.ts_flag==TS_FLAG_6S){system(cmd);} //.mkv .ts .m3u8
	else if(ts.ts_flag==TS_FLAG_START){remove(ts.file_name);remove(putname);}  //printf("start__file_name=%s, putname=%s\n",ts.file_name,putname);
	else if(ts.ts_flag==TS_FLAG_MID){system(cmd);} //printf("mid__cmd=%s\n",cmd);
	else if(ts.ts_flag==TS_FLAG_END){system(cmd);remove(ts.m3u8name);if(arg)free(arg);}//printf("end__cmd=%s; m3u8name=%s\n",cmd,ts.m3u8name);
	else if(ts.ts_flag==TS_FLAG_ONE){system(cmd);if(arg)free(arg);}//printf("end__one=%s\n",cmd);
	else {system(cmd);}
#endif
}


void amazon_put_even_thread(ts_info *ts)
{ 
	pthread_t Thread_amazon_put_even_ID;
	if(pthread_create(&Thread_amazon_put_even_ID, NULL, amazon_put_even, (void *)ts) != 0)
	{
		printf("amazon_put_even_thread: create thread failed; ");
	}
	pthread_detach(Thread_amazon_put_even_ID);
}



