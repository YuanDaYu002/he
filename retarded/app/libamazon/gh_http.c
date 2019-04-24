#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "gh_http.h"
#include "tcpclient.h"
#include "typeport.h"


extern unsigned long get_file_size(const char *path); 
void http_parse_url(const char *url, char *host, int *port, char *file_name)
{
     /*通过url解析出域名, 端口, 以及文件名*/
    int j = 0;
    int i;
    int start = 0;
    *port = 80;
    char *patterns[] = {"http://", "https://", NULL};

    for (i = 0; patterns[i]; i++)//分离下载地址中的http协议
        if (strncmp(url, patterns[i], strlen(patterns[i])) == 0)
            start = strlen(patterns[i]);

    //解析域名, 这里处理时域名后面的端口号会保留
    for (i = start; url[i] != '/' && url[i] != '\0'; i++, j++)
        host[j] = url[i];
    host[j] = '\0';

    //解析端口号, 如果没有, 那么设置端口为80
    char *pos = strstr(host, ":");
    if (pos)
        sscanf(pos, ":%d", port);

    //删除域名端口号
    for (i = 0; i < (int)strlen(host); i++)
    {
        if (host[i] == ':')
        {
            host[i] = '\0';
            break;
        }
    }

    //获取下载文件名
    j = 0;
    for (i = start; url[i] != '\0'; i++)
    {
        if (url[i] == '/')
        {
            if (i !=  strlen(url) - 1)
                j = 0;
            continue;
        }
        else
            file_name[j++] = url[i];
    }
    file_name[j] = '\0';

}


/*******************************************************************************
*@ Description    :
*@ Input          :<pclient>:post 本地客户端的描述信息
                    <page>: URL
                    <message_json>要推送的字符数据（http头的数据部分）
                    <>
                    <filepath>文件的路径
                    
*@ Output         :<response>服务器返回的响应包数据
*@ Return         :成功：0 失败: <0的数
*@ attention      : 返回参数 response 需要上层free才能释放
*******************************************************************************/
static int http_post_file(tcpclient *pclient, const char *page,
     const char* message_json, const char* content_type, const char *filepath,
     char **response)
{

    //check if the file is valid or not
    struct stat stat_buf;
    if(lstat(filepath,&stat_buf)<0)//获取文件的相关信息
    {
        ERROR_LOG("lstat %s fail", filepath);
        return -1;
    }
    if(!S_ISREG(stat_buf.st_mode)) //判断该文件是否是一个常规文件
    {
        ERROR_LOG("%s is not a regular file!",filepath);
        return  -1;
    }
 
    char *filename;
    filename=strrchr((char*)filepath,'/'); //查找filepath中最后一次出现字符‘/’的位置
    if(filename==NULL)
    {
 
    }
    filename+=1;
    if(filename>=filepath+strlen(filepath))
    {
        //'/' is the last character
        ERROR_LOG("%s is not a correct file!",filepath);
        return  -1;
    }
 
    //DEBUG_LOG("filepath=%s,filename=%s",filepath,filename);
 
    char content_type_buf[4096];  //用于构造 http post 的头部 content_type 信息
    memset(content_type_buf, 0, 4096);
 
    char post[512]; //http 头 POST 字段
    char host[256]; //http 头 HOST 字段
    char *lpbuf;//指向服务器返回的数据
    char *ptmp;
    int len=0;
 
    lpbuf = NULL;
    const char *header2="User-Agent: Is Http 1.1\r\nCache-Control: no-cache\r\nAccept: */*\r\n";
 
    snprintf(post,sizeof(post),"POST %s HTTP/1.1\r\n",page);
    snprintf(host,sizeof(host),"HOST: %s:%d\r\n",pclient->remote_ip,pclient->remote_port);
    strcpy(content_type_buf,post);
    strcat(content_type_buf,host);
 
 
    char *boundary = (char *)"-----------------------8d9ab1c50066a";
 
    strcat(content_type_buf, "Content-Type: multipart/form-data; boundary=");
    strcat(content_type_buf, boundary);
    strcat(content_type_buf, "\r\n");

    //--Construct request data {filePath, file}
    char content_before[8192];
    memset(content_before, 0, 8192);

    //GH_LOG_TEXT(message_json);
    //附加数据。
    strcat(content_before, "--");
    strcat(content_before, boundary);
    strcat(content_before, "\r\n");
    strcat(content_before, "Content-Disposition: form-data; name=\"consite_message\"\r\n\r\n");
    strcat(content_before, message_json);
	int i = strlen(message_json);
	if(message_json[i-1] != '\n' && message_json[i-1] != '\r')//没有加换行符
	{
		 strcat(content_before, "\r\n");
	}

    strcat(content_before, "--");
    strcat(content_before, boundary);
    strcat(content_before, "\r\n");
    strcat(content_before, "Content-Disposition: attachment; name=\"file\"; filename=\"");
    strcat(content_before, filename);
    strcat(content_before, "\"\r\n");
    if(NULL == content_type)
        strcat(content_before, "Content-Type: image/jpeg\r\n\r\n");
    else
    {
        strcat(content_before, content_type);
        strcat(content_before,"\r\n\r\n");
    }
        

    char content_end[2048];
    memset(content_end, 0, 2048);
    strcat(content_end, "\r\n");
    strcat(content_end, "--");
    strcat(content_end, boundary);
    strcat(content_end, "--\r\n");

    int max_cont_len = 50*1024;  //用来缓存文件-----------
    char* content = (char*)malloc(max_cont_len);
    if(NULL == content)
    {
        ERROR_LOG("malloc failed !\n");
        return -1;
    }

    int fd;
    fd = open(filepath,O_RDONLY,0666);
    if(!fd)
    {
        ERROR_LOG("fail to open file : %s",filepath);
        return -1;
    }
    len = get_file_size(filepath);
 

    char *lenstr;//字符串总长度
    lenstr = (char*)malloc(256);
    snprintf(lenstr,256, "%d", (int)(strlen(content_before)+len+strlen(content_end)));
 
    strcat(content_type_buf, "Content-Length: ");
    strcat(content_type_buf, lenstr);
    strcat(content_type_buf, "\r\n\r\n");
    free(lenstr);

    //send
    if (!pclient->connected && tcpclient_conn(pclient) == -1)
    {
        return -1;
    }
 
    //content-type
    tcpclient_send(pclient,content_type_buf,strlen(content_type_buf));
 
    //content-before
    tcpclient_send(pclient,content_before,strlen(content_before));
    
    //content
    unsigned int count_len = 0;
    unsigned int ret = 0;
    while(count_len != len)
    {
        ret = read(fd,content,max_cont_len);
        if(ret < 0)
        {
            ERROR_LOG("read fialed !\n");
            close(fd);
            free(content);
            return -1;
        }
        
        if(0 == ret && count_len != len)//读到文件的实际总长度和原本长度不一致
        {
            ERROR_LOG("the actualy data length  is not equal the file length !\n ");
            close(fd);
            free(content);
            return -1;
        }
        
        count_len += ret;
        int ret_tmp = tcpclient_send(pclient,content,ret);
        if(ret_tmp != ret)
        {
            ERROR_LOG("tcpclient_send failed ! \n");
            close(fd);
            free(content);
            return -1;
        }
        
    }
    close(fd);
    free(content);
 
    //content-end
    tcpclient_send(pclient,content_end,strlen(content_end));

/*---#进入服务器响应包解析过程------------------------------------------------------------*/
    /*it's time to recv from server*/
    if(tcpclient_recv(pclient,&lpbuf,0) <= 0)
    {
        if(lpbuf) free(lpbuf);
        return -2;
    }

    /*---#对返回数据进行合法检验------------------------------------------------------------*/
    //GH_LOG_TEXT(lpbuf);
 
        /*响应代码,|HTTP/1.0 200 OK|
         *从第10个字符开始,第3位
         * */
        //DEBUD_LOG("responsed:\n%s",lpbuf);
        memset(post,0,sizeof(post));
        strncpy(post,lpbuf+9,3); //????????????????????? 注意这个 9 ？
        if(atoi(post)!=200)
        {
            ERROR_LOG("atoi(post)!=200");
            if(lpbuf) free(lpbuf);
            return atoi(post);
        }
 
        ptmp = (char*)strstr(lpbuf,"\r\n\r\n");
        if(ptmp == NULL)
        {
            free(lpbuf);
            return -3;
        }
        ptmp += 4;//跳过\r\n
        
        /*---#前边为http的头部分------------------------------------------------------------*/
        /*---#拷贝真实数据给返回参数------------------------------------------------------------*/
        
        len = strlen(ptmp)+1;
        *response=(char*)malloc(len);
        if(*response == NULL)
        {
            if(lpbuf) free(lpbuf);
            return -1;
        }
        memset(*response,0,len);
        memcpy(*response,ptmp,len-1);

        //从头域找到内容长度,如果没有找到则不处理
        ptmp = (char*)strstr(lpbuf,"Content-Length:");
        if(ptmp != NULL)
        {
            char *ptmp2;
            ptmp += 15;
            ptmp2 = (char*)strstr(ptmp,"\r\n");
            if(ptmp2 != NULL)//将内容长度部分的字符串摘出来并转换成int型数据
            {
                memset(post,0,sizeof(post));
                strncpy(post,ptmp,ptmp2-ptmp);
                if(atoi(post)<len)
                    (*response)[atoi(post)] = '\0';
            }
        }

        if(lpbuf) free(lpbuf);
        return 0;
 
}

/*******************************************************************************
*@ Description    : http推送(字符串)函数
*@ Input          :<pclient>本地客户端描述信息
                    <page> 服务端的URL
                    <message_json>http头要捎带的客户端数据（JSON字符串）
                    <ret_code>服务端的反馈结果代码（一般成功为200，失败为其他错误码）
*@ Output         :<response>服务端的反馈数据(使用后需要释放)
*@ Return         :失败： 小于0 ; 成功 :0
*@ attention      :
*******************************************************************************/
static int http_post_string(tcpclient *pclient, const char *page,
 const const char* message_json,const char* content_type, int* ret_code, char **response)
{
    unsigned int content_buf_size = 1024*2;
    char* content_buffer = (char*)malloc(content_buf_size);
    if(NULL == content_buffer)
    {
        ERROR_LOG("malloc failed \n");
        return -1;
    }
    memset(content_buffer,0,content_buf_size);

 
    char post[512]={0};
    char host[256]={0};
    char* lpbuf = NULL;
    char* ptmp  = NULL;

    const char *header2="Connection: keep-alive\r\nAccept-Encoding: gzip, deflate\r\nAccept: */*\r\nUser-Agent: python-requests/2.18.4\r\n";

    snprintf(post,sizeof(post),"POST %s HTTP/1.1\r\n",page);
    strcpy(content_buffer, post);

    snprintf(host,sizeof(host),"HOST: %s:%d\r\n",pclient->remote_ip,pclient->remote_port);
    strcat(content_buffer, host);
    strcat(content_buffer, header2);

    char lenstr[6] = {0};
   // lenstr = (char*)malloc(256);
    snprintf(lenstr,sizeof(lenstr), "%d", (int)(strlen(message_json)));
    DEBUG_LOG("strlen(message_json)= (%d)  lenstr = %s\n",(int)strlen(message_json),lenstr);
    strcat(content_buffer, "Content-Length: ");
    strcat(content_buffer, lenstr);
    strcat(content_buffer, "\r\n");

    char content_type_buf[2048] = {0};
    if(NULL == content_type)
        strcat(content_type_buf, "Content-Type: application/json");
    else
        strcat(content_type_buf, content_type);
    strcat(content_type_buf, "\r\n\r\n");
    strcat(content_type_buf, message_json);

    strcat(content_buffer, content_type_buf);
    

    
    /*---#send------------------------------------------------------------*/
    if (!pclient->connected)
    {
        if (tcpclient_conn(pclient) == -1)
        {
            ERROR_LOG("tcpclient_conn failed !\n");
            return -1;
        }
    }

    //content-type
    int ret = tcpclient_send(pclient, content_buffer, strlen(content_buffer));
    if(ret != strlen(content_buffer))
    {
        ERROR_LOG("tcpclient_send failed!\n");
         if(content_buffer) free(content_buffer);
         return -1;
    }
    DEBUG_LOG("tcpclient_send success!\n");
    
    /*---# recv feedbak from server------------------------------------------------------------*/
    /*it's time to recv from server*/
    if(tcpclient_recv(pclient,&lpbuf,0) <= 0)
    {
        ERROR_LOG("tcpclient_recv failed!\n");
        if(lpbuf) free(lpbuf);
        return -2;
    }
    DEBUG_LOG("tcpclient_recv success!\n");
    /*---# 对返回的结果进行解析------------------------------------------------------------*/
    /*响应代码,|HTTP/1.0 200 OK|
     *从第10个字符开始,第3位
     * */
    DEBUG_LOG("responsed:\n %s\n",lpbuf);
    memset(post,0,sizeof(post));
    strncpy(post,lpbuf+9,3); //注意这个 9  有可能导致不兼容 ！！！！！
    *ret_code = atoi(post);
    if(*ret_code!=200)
    {
        ERROR_LOG("ret_code = (%d)\n",*ret_code);
        if(lpbuf) free(lpbuf);
        return *ret_code;
    }

    ptmp = (char*)strstr(lpbuf,"\r\n\r\n");
    if(ptmp == NULL)
    {
        ERROR_LOG("find \\r\\n\\r\\n failed!\n");
        free(lpbuf);
        return -3;
    }
    ptmp += 4;//跳过\r\n

    int len = strlen(ptmp)+1;
    DEBUG_LOG("response len = (%d)\n",len);
    *response=(char*)malloc(len);
    if(*response == NULL)
    {
        if(lpbuf) free(lpbuf);
        return -1;
    }
    memset(*response,0,len);
    memcpy(*response,ptmp,len-1);

    //从头域找到内容长度,如果没有找到则不处理(为了加个结束符)
    ptmp = (char*)strstr(lpbuf,"Content-Length:");
    if(ptmp != NULL)
    {
        char *ptmp2;
        ptmp += 15;  //注意这个15也可能导致不兼容！！！！
        ptmp2 = (char*)strstr(ptmp,"\r\n");
        if(ptmp2 != NULL)
        {
            memset(post,0,sizeof(post));
            strncpy(post,ptmp,ptmp2-ptmp);
            if(atoi(post)<len)
                (*response)[atoi(post)] = '\0';
        }
    }

    if(lpbuf) free(lpbuf);
    if(content_buffer) free(content_buffer);
    return 0;
}

/*******************************************************************************
*@ Description    :
*@ Input          :<pHost>服务端的域名
                    <nPort> TCP通信的端口
                    <pServerPath> 服务端的 URL
                    <pMessageJson> 要推送的 字符数据
                    <content_type>http头content-type字段，
                             传入NULL时默认填充"Content-Type: application/json"            
*@ Output         :<ret_code>服务端返回信息中的代码（错误码）
                    <ret_buffer>服务端返回的信息数据（使用后需要free）
*@ Return         : 失败：小于0 ; 成功 ：0
*@ attention      :
*******************************************************************************/
int http_post_data(const char* pServerPath,const char* pMessageJson,char*content_type, int* ret_code, char** ret_buffer)

{
 
    tcpclient client;

	char host[128] = {0};
	char file_name[64] = {0};
	int port;
	http_parse_url(pServerPath,host,&port,file_name);
	printf("---url = %s\n",pServerPath);
	printf("---host = %s\n",host);
	printf("---port = %d file_name = %s\n",port,file_name);
	
    tcpclient_create(&client, host, port);
 
//    if(http_post(&client,"/recv_file.php","f1=hello",&response)){
//        GH_LOG_INFO("失败!");
//        exit(2);
//    }
    int ret = http_post_string(&client, pServerPath, pMessageJson,content_type, ret_code, ret_buffer);
 
 
    tcpclient_close(&client);
    return ret;
}

/*******************************************************************************
*@ Description    : HTTP上传文件
*@ Input          :<pHost>服务端的域名
                    <nPort> TCP通信的端口
                    <pServerPath> 服务端的 URL
                    <pMessageJson> 要推送的 字符数据
                    <content_type>http头content-type字段，
                            传入NULL时默认填充"Content-Type: image/jpeg\r\n\r\n"
                    <pFile> 要上传的文件(绝对路径)
*@ Output         :
*@ Return         :失败：NULL ; 成功 :服务器的响应数据
*@ attention      :返回值 response 需要上层 free 才能释放
*******************************************************************************/
char* http_upload_file(const char* pServerPath,const char* pMessageJson,const char* content_type, const char* pFile)
{
 
    tcpclient client;
    char *response = NULL;

	char host[128] = {0};
	char file_name[64] = {0};
	int port;
	http_parse_url(pServerPath,host,&port,file_name);
	printf("---url = %s\n",pServerPath);
	printf("---host = %s\n",host);
	printf("---port = %d file_name = %s\n",port,file_name);

	
    tcpclient_create(&client, host, port);
 
//    if(http_post(&client,"/recv_file.php","f1=hello",&response)){
//        GH_LOG_INFO("失败!");
//        exit(2);
//    }
    int ret= http_post_file(&client, pServerPath, pMessageJson,content_type, pFile, &response);
    
    tcpclient_close(&client);
    return response;
}







