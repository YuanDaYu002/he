#ifndef __GH_HTTP_H__
#define __GH_HTTP_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
*@ Description    :解析 URL 获得域名端口等信息
*@ Input          :<url>URL
*@ Output         :<host>域名
					<port>端口
					<file_name>文件名（如果有的话）
*@ Return         :
*@ attention      :host直接传入数组，注意长度要给足够
*******************************************************************************/
void http_parse_url(const char *url, char *host, int *port, char *file_name);



/*******************************************************************************
*@ Description    : HTTP上传文件
*@ Input          : <pServerPath> 服务端的 URL
                    <pMessageJson> 要推送的 字符数据
                    <content_type>http头content-type字段，
                    		传入NULL时默认填充"Content-Type: image/jpeg\r\n\r\n"
                    <pFile> 要上传的文件
*@ Output         :
*@ Return         :失败：小于0 ; 成功 ：0
*@ attention      :
*******************************************************************************/
char* http_upload_file(const char* pServerPath,const char* pMessageJson,const char* content_type, const char* pFile);


 /*******************************************************************************
 *@ Description    :http上传数据（字符串消息推送）
 *@ Input		   : <pServerPath> 服务端的 URL
					 <pMessageJson> 要推送的 字符数据
					 <content_type>http头content-type字段，
							 传入NULL时默认填充"Content-Type: application/json"
 *@ Output		   :<ret_code>服务端返回信息中的代码（错误码）
					 <ret_buffer>服务端返回的信息数据（使用后需要free）
 *@ Return		   :失败：小于0 ; 成功 ：0
 *@ attention	   :
 *******************************************************************************/
 int http_post_data(const char* pServerPath,const char* pMessageJson,char*content_type, int* ret_code, char** ret_buffer);
 




#ifdef __cplusplus
}
#endif

#endif


