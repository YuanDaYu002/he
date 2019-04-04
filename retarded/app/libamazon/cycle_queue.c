/******************************************************************************
  文 件 名   : cycle_queue.c
  功能描述   : 循环队列的创建、销毁、入队、出队、判断队空、遍历输出
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cycle_queue.h"

/*****************************************************************************
 函 数 名  : CreateQueue
 功能描述  : 创建循环队列
返 回 值  : Queue*
*****************************************************************************/
Queue *CreateQueue(void)
{
    Queue *pQ = NULL;
    pQ = (Queue*)malloc(sizeof(Queue));
    if ( NULL == pQ )
    {
        return pQ;
    }
    memset(pQ,0,sizeof(Queue));
    return pQ;
}




int DestroyQueue(Queue *pQ)
{
    if ( NULL == pQ )
    {
        return QUEUE_ERR;
    }
    free(pQ);
    pQ = NULL;
    return QUEUE_OK;
}

/*******************************************************************************
*@ Description    :  循环队列 入队
*@ Input          : <Queue> 队列指针  
					<tdata> 输入数据
*@ Output         :
*@ Return         :队列错误码
*******************************************************************************/
int InQueue(Queue *pQ,data_t tdata)
{
    if (NULL == pQ)
    {
        return QUEUE_ERR;
    }
	
	if(QUEUE_SIZE == pQ->iNum)
	{
		return QUEUE_FULL;
	}
	
    pQ->data[pQ->Rear] = tdata;
    pQ->Rear++;
    if ( QUEUE_SIZE == pQ->Rear)//重新循环
    {
        pQ->Rear = 0;
    }
    pQ->iNum++;
    return QUEUE_OK;
}


int DeQueue(Queue *pQ,data_t *pdata)
{
    if ( (NULL == pQ) || (NULL == pdata) || (0 == pQ->iNum) )
    {
        return QUEUE_ERR;
    }
    *pdata = pQ->data[pQ->Front];
    pQ->Front++;
    pQ->iNum--;
    if ( QUEUE_SIZE == pQ->Front)
    {
        pQ->Front = 0;
    }
    return QUEUE_OK;
}

int QueueIsEmpty(Queue *pQ)
{
    if ( NULL == pQ)
    {
        return QUEUE_ERR;
    }
    if ( 0 == pQ->iNum)
    {
        return QUEUE_EMPTY;
    }
    return QUEUE_OK;
}

int QueueIsFull(Queue *pQ)
{
    if ( NULL == pQ)
    {
        return QUEUE_ERR;
    }
    if ( QUEUE_SIZE == pQ->iNum)
    {
        return QUEUE_FULL;
    }
    return QUEUE_OK;
}

/*
void ShowQueue(Queue *pQ)
{
    if ( (NULL == pQ) || (0 == pQ->iNum))
    {
        return;
    }
    int i = pQ->Front;
    int j = pQ->iNum;
    while ( j-- )
    {
        printf("%5d",pQ->data[i]);
        i++;
    }
    printf("\n");
    return ;
}
*/

