
/******************************************************************************
文 件 名   : cycle_queue.c
函数功能：循环队列的创建、销毁、入队、出队、判断队空、遍历输出
******************************************************************************/
#ifndef _CYCLE_QUEUE_H_
#define _CYCLE_QUEUE_H_

#define QUEUE_SIZE 10   //循环队列的大小
typedef int data_t;  //单个元素的数据类型
typedef struct _QUEUE
{
    data_t data[QUEUE_SIZE];
    int iNum; 	//队列中总元素个数
    int Front;	//前驱
    int Rear;	//后继
}Queue;

typedef enum QUEUE_OP
{
    QUEUE_EMPTY = -3,
	QUEUE_FULL,
    QUEUE_ERR,
    QUEUE_OK
}QUEUE_err_code;

Queue *CreateQueue(void);
int DestroyQueue(Queue *pQ);
int InQueue(Queue *pQ,data_t tdata);
int DeQueue(Queue *pQ,data_t *pdata);
int QueueIsEmpty(Queue *pQ);
int QueueIsFull(Queue *pQ);

//void ShowQueue(Queue *pQ);
#endif /* _QUEUE_H_ */
