 
/***************************************************************************
* @file: link_queue.h
* @author:   
* @date:  4,23,2019
* @brief: 实现链式存储的队列的创建，销毁，入队，出队，遍历，判断是否为空队
* @attention:
***************************************************************************/

#ifndef _LINK_QUEUE_H_
#define _LINK_QUEUE_H_

#include "amazon_S3.h"
 
#define LQUEUE_SIZE 10


typedef struct _put_file_info_t data_t;
 
typedef struct QNODE
{
    data_t data;
    struct QNODE* pNext;
}QNode;
 
typedef struct QHEADER
{
    int iNum;
    QNode* pHeader; //队头
    QNode* pRear;	//队尾
}QHeader;
 
typedef enum LQUEUE_OP
{
	LQUEUE_FULL = -3,	//队列满
    LQUEUE_EMPTY = -2,	//队列空
    LQUEUE_ERR,
    LQUEUE_OK
}LQUEUE_OP_ENUM;
 
 
QHeader* CreateLQueue(void);
int DestroyLQueue(QHeader* qh);
int InLQueue(QHeader* qh,data_t *data);
int DeLQueue(QHeader* qh,data_t *pdata);
int LQueueIsEmpty(QHeader*qh);
//void ShowLQueue(QHeader *qh);
 
#endif /* _LINK_QUEUE_H_ */


