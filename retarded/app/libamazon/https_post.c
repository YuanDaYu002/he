/************************************************************************
    > File Name: https_post.c 
    > Author:
    > Mail: 
    > Blog: 
    > Created Time: 
 ***********************************************************************/

#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "in.h"
#include "my_inet.h"

#include <openssl/err.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/crypto.h>

//#include "https_post.h"


#define HTTP_HEADERS_MAXLEN 	512 	// Headers 的最大长度

/*
 * Headers 按需更改
 */
const char *HttpsPostHeaders = 	"User-Agent: Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)\r\n"
								"Cache-Control: no-cache\r\n"
								"Accept: */*\r\n"
								"Content-type: application/json\r\n";

/*
 * @Name 			- 创建TCP连接, 并建立到连接
 * @Parame *server 	- 字符串, 要连接的服务器地址, 可以为域名, 也可以为IP地址
 * @Parame 	port 	- 端口
 *
 * @return 			- 返回对应sock操作句柄, 用于控制后续通信
 */
int client_connect_tcp(char *server,int port)
{
	int sockfd;
	struct hostent *host;
	struct sockaddr_in cliaddr;

	sockfd=socket(AF_INET,SOCK_STREAM,0);
	if(sockfd < 0){
		perror("create socket error");
		return -1;
	}

	if(!(host=gethostbyname(server))){
		printf("gethostbyname(%s) error!\n", server);
		return -2;
	}

	//bzero(&cliaddr,sizeof(struct sockaddr));
	memset(&cliaddr,0,sizeof(struct sockaddr));
	cliaddr.sin_family=AF_INET;
	cliaddr.sin_port=t_htons(port);
	cliaddr.sin_addr=*((struct in_addr *)host->h_addr);

	if(connect(sockfd,(struct sockaddr *)&cliaddr,sizeof(struct sockaddr))<0){
		perror("[-] error");
		return -3;
	}

	return(sockfd);
}

/*
 * @Name 			- 封装post数据包括headers
 * @parame *host 	- 主机地址, 域名
 * @parame  port 	- 端口号
 * @parame 	page 	- url相对路径
 * @parame 	len 	- 数据内容的长度
 * @parame 	content - 数据内容
 * @parame 	data 	- 得到封装的数据结果
 *
 * @return 	int 	- 返回封装得到的数据长度
 */
int post_pack(const char* headers,const char *host, int port, const char *page, int len, const char *content, char *data)
{
	int re_len;
	
	if(NULL == headers)//无 header ，按照默认的来
		re_len = strlen(page) + strlen(host) + len + HTTP_HEADERS_MAXLEN;
	else
		re_len =  strlen(headers) + len + HTTP_HEADERS_MAXLEN;
	
	char *post = NULL;
	post = malloc(re_len);
	if(post == NULL)
	{
		printf("malloc failed !\n");
		return -1;
	}
	memset(post,0,re_len);
	if(NULL == headers)
	{
		snprintf(post,re_len, "POST %s HTTP/1.0\r\n", page);
		snprintf(post,re_len, "%sHost: %s:%d\r\n",post, host, port);
		//sprintf(post, "%s%s", post, headers);无该部分
		snprintf(post,re_len,"%sContent-Length: %d\r\n\r\n", post, len);
		int str_len = strlen(post);
		memcpy(post + str_len,content,len);
		//sprintf(post, "%s%s", post, content);该方式只能拷贝字符串，如果是非字符串数据会出问题 		
	}
	else //外部有传入的 headers
	{
		//sprintf(post, "POST %s HTTP/1.0\r\n", page);无该部分
		//sprintf(post, "%sHost: %s:%d\r\n",post, host, port);无该部分
		snprintf(post,re_len, "%s%s", post, headers);
		//sprintf(post, "%sContent-Length: %d\r\n\r\n", post, len);无该部分
		int str_len = strlen(post);
		memcpy(post + str_len,content,len);
		//sprintf(post, "%s%s", post, content); //该方式错误，当业务需要上传非字符串数据的时候, 会造成数据传输丢失或失败
	}
	

	re_len = strlen(post);
	memset(data, 0, re_len+1);
	memcpy(data, post, re_len);

	free(post);
	return re_len;
}


/*
 * @Name 		- 	初始化SSL, 并且绑定sockfd到SSL
 * 					此作用主要目的是通过SSL来操作sock
 * 					
 * @return 		- 	返回已完成初始化并绑定对应sockfd的SSL指针
 */
SSL *ssl_init(int sockfd)
{
	int re = 0;
	SSL *ssl;
	SSL_CTX *ctx;

	SSL_library_init();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_client_method());
	if (ctx == NULL){
		return NULL;
	}

	ssl = SSL_new(ctx);
	if (ssl == NULL){
		return NULL;
	}

	/* 把socket和SSL关联 */
	re = SSL_set_fd(ssl, sockfd);
	if (re == 0){
		SSL_free(ssl);
		return NULL;
	}

    /*
     * 经查阅, WIN32的系统下, 不能很有效的产生随机数, 此处增加随机数种子
     */
	RAND_poll();
	while (RAND_status() == 0)
	{
		unsigned short rand_ret = rand() % 65536;
		RAND_seed(&rand_ret, sizeof(rand_ret));
	}
	
	/*
     * ctx使用完成, 进行释放
     */
	SSL_CTX_free(ctx);
	
	return ssl;
}

/*
 * @Name 			- 通过SSL建立连接并发送数据
 * @Parame 	*ssl 	- SSL指针, 已经完成初始化并绑定了对应sock句柄的SSL指针
 * @Parame 	*data 	- 准备发送数据的指针地址
 * @Parame 	 size 	- 准备发送的数据长度
 *
 * @return 			- 返回发送完成的数据长度, 如果发送失败, 返回 -1
 */
int ssl_send(SSL *ssl, const char *data, int size)
{
	int re = 0;
	int count = 0;

	re = SSL_connect(ssl);

	if(re != 1){
		return -1;
	}

	while(count < size)
	{
		re = SSL_write(ssl, data+count, size-count);
		if(re == -1){
			return -2;
		}
		count += re;
	}

	return count;
}

/*
 * @Name 			- SSL接收数据, 需要已经建立连接
 * @Parame 	*ssl 	- SSL指针, 已经完成初始化并绑定了对应sock句柄的SSL指针
 * @Parame  *buff 	- 接收数据的缓冲区, 非空指针
 * @Parame 	 size 	- 准备接收的数据长度
 *
 * @return 			- 返回接收到的数据长度, 如果接收失败, 返回值 <0 
 */
int ssl_recv(SSL *ssl, char *buff, int size)
{
	int i = 0; 				// 读取数据取换行数量, 即判断headers是否结束 
	int re;
	int len = 0;
	char headers[HTTP_HEADERS_MAXLEN];

	if(ssl == NULL){
		return -1;
	}

	// Headers以换行结束, 此处判断头是否传输完成
	while((len = SSL_read(ssl, headers, 1)) == 1)
	{
		if(i < 4){
			if(headers[0] == '\r' || headers[0] == '\n'){
				i++;
				if(i>=4){
					break;
				}
			}else{
				i = 0;
			}
		}
		//printf("%c", headers[0]);		// 打印Headers
	}

	len = SSL_read(ssl, buff, size);
	return len;
}

/*
 * @Name 			- HTTPS的POST提交
 * @Parame 	*host 	- 主机地址, 即域名
 * @Parame 	 port 	- 端口号, 一般为443
 * @Parame 	*url 	- url相对路径
 * @Parame 	*headers- 要发送的 http 的头(传NULL则使用内部默认的头)
 * @Parame 	*data 	- 要提交的数据内容, 不包括Headers
 * @Parame 	 dsize 	- 需要发送的数据包大小, 由外部调用传入, 不包含头
 * @Parame 	*buff 	- 数据缓存指针, 非空数组或提前malloc
 * @Parame 	 bsize 	- 需要读取的返回结果长度, 可以尽量给大, 直到读取结束
 *
 * @return 			- 	返回结果长度, 如果读取失败, 则返回值 <0
 * 						-1 : 为POST数据申请内存失败
 * 						-2 : 建立TCP连接失败
 * 						-3 : SSL初始化或绑定sockfd到SSL失败
 *						-4 : POST提交失败
 *						-5 : 等待响应失败
 */
int https_post(char *host, int port, char *url,char* headers, const char *data, int dsize, char *buff, int bsize)
{
	SSL *ssl;
	int re = 0;
	int sockfd;
	int data_len = 0;
	int ssize = dsize + HTTP_HEADERS_MAXLEN; 	// 欲发送的数据包大小

	char *sdata = malloc(ssize);
	if(sdata == NULL){
		return -1;
	}

	// 1、建立TCP连接
	sockfd = client_connect_tcp(host, port);
	if(sockfd < 0){
		free(sdata);
		return -2;
	}

	// 2、SSL初始化, 关联Socket到SSL
	ssl = ssl_init(sockfd);
	if(ssl == NULL){
		free(sdata);
		close(sockfd);
		return -3;
	}

	// 3、组合POST数据
	data_len = post_pack(headers,host, port, url, dsize, data, sdata);

	// 4、通过SSL发送数据
	re = ssl_send(ssl, sdata, data_len);
	if(re < 0){
		free(sdata);
		close(sockfd);
		SSL_shutdown(ssl);
		return -4;
	}

	// 5、取回数据
	int r_len = 0;
	r_len = ssl_recv(ssl, buff, bsize);
	if(r_len < 0){
		free(sdata);
		close(sockfd);
		SSL_shutdown(ssl);
		return -5;
	}

	// 6、关闭会话, 释放内存
	free(sdata);
	close(sockfd);
	SSL_shutdown(ssl);
	ERR_free_strings();

	return r_len;
}




