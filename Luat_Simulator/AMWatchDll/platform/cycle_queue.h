/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    cycle_queue.h
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          cycle queue.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#ifndef __CYCLE_QUEUE_H__
#define __CYCLE_QUEUE_H__

typedef struct {
    UINT8 *buf;
    UINT32 size;        
    UINT32 head;
    UINT32 tail;
    unsigned empty: 1;
    unsigned full:  1;
    unsigned overflow:  1;  
}CycleQueue;

void QueueClean(CycleQueue *Q_ptr);

int QueueInsert(CycleQueue *Q_ptr, UINT8 *data, UINT32 len);

int QueueGetFreeSpace(CycleQueue *Q_ptr);

int QueueDelete(CycleQueue *Q_ptr, UINT8 *data, UINT32 len);
    
#endif //__CYCLE_QUEUE_H__