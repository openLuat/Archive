/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    cycle_queue.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/09/13
 *
 * Description:
 *          cycle queue.
 * History:
 *     panjun 16/09/13 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "cycle_queue.h"

/****************************************************************************
 *
 * Function: QueueClean
 *
 * Parameters: 
 *         CycleQueue *Q_ptr
 *
 * Returns: void 
 *
 * Description: 
 *
 ****************************************************************************/
void QueueClean(CycleQueue *Q_ptr)
{
    Q_ptr->head = Q_ptr->tail = 0;
    Q_ptr->empty = 1;
    Q_ptr->full = 0;
    Q_ptr->overflow = 0;    
}

int QueueGetFreeSpace(CycleQueue *Q_ptr)
{
    if(Q_ptr->empty)
    {
        return Q_ptr->size;
    }
    if(Q_ptr->full || Q_ptr->overflow)
    {
      return 0;
    }
    
    if(Q_ptr->head > Q_ptr->tail)
    {
        return Q_ptr->size -(Q_ptr->head - Q_ptr->tail);
    }
    else
    {
        return Q_ptr->tail - Q_ptr->head;
    }
}


/****************************************************************************
 *
 * Function: QueueInsert
 *
 * Parameters: 
 *         CycleQueue *Q_ptr
 *         uint8 *data
 *         uint32 len
 *
 * Returns: int 
 *
 * Description: 
 *
 ****************************************************************************/
int QueueInsert(CycleQueue *Q_ptr, UINT8 *data, UINT32 len)
{
    UINT32 ret = 0;

    for (; ret < len; ret++)
    {
        *(Q_ptr->buf + Q_ptr->head) = *(data + ret);
        
        if ((1==Q_ptr->full) && (Q_ptr->head==Q_ptr->tail))
        {
            Q_ptr->overflow = 1;               
        }
        
        Q_ptr->head = ++Q_ptr->head % Q_ptr->size;
        
        if (Q_ptr->head == Q_ptr->tail)
        {
            Q_ptr->full = 1;
        }
        
        if (1 == Q_ptr->empty)
        {
            Q_ptr->empty = 0;
        }
    }
    
    if (Q_ptr->overflow)
    {
        Q_ptr->tail = Q_ptr->head;
    }
   
    return ret;       
}

/****************************************************************************
 *
 * Function: QueueDelete
 *
 * Parameters: 
 *         CycleQueue *Q_ptr
 *         uint8 *data
 *         uint32 len
 *
 * Returns: int 
 *
 * Description: 
 *
 ****************************************************************************/
int QueueDelete(CycleQueue *Q_ptr, UINT8 *data, UINT32 len)
{
    UINT32 ret = 0;
    
    if (!Q_ptr->empty)
    {
        while(ret < len) 
        {
            *(data + ret) = *(Q_ptr->buf + Q_ptr->tail);
            Q_ptr->tail = ++Q_ptr->tail % Q_ptr->size;
            ret++;
            
            if (Q_ptr->tail == Q_ptr->head)
            {
                Q_ptr->empty = 1;
                break;
            }
        }
    }
   
    if ((ret>0) && (1==Q_ptr->full))   
    {
        Q_ptr->full = 0;
        Q_ptr->overflow = 0;
    }
    
    return ret;    
}
