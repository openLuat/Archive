/**************************************************************************
 *              Copyright (C), AirM2M Tech. Co., Ltd.
 *
 * Name:    list.cpp
 * Author:  panjun
 * Version: V0.1
 * Date:    2016/10/19
 *
 * Description:
 *
 *
 * History:
 *     panjun 16/10/19 Initially create file.
 **************************************************************************/

#include "stdafx.h"
#include "list.h"


void list_add_before(list_head *node, list_head *pos)
{
  node->prev = pos->prev;
  node->next = pos;
  pos->prev->next = node;
  pos->prev = node;
}

void list_add_after(list_head *node, list_head *pos)
{
  	if(pos->next==NULL && pos->prev == NULL) //¿ÕÁ´±í
	{
		pos->next = node;
		node->prev= pos;
		node->next =NULL;
	}
  	else
	{
		node->prev = pos;
		node->next = pos->next;
		pos->next->prev = node;
		pos->next = node;
	}
}

void list_del(list_head *node)
{
  node->prev->next = node->next;
  node->next->prev = node->prev;
}

