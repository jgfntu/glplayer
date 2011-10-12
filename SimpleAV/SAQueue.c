/*
 * SAQueue.c
 * Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This file is part of SimpleAV.
 *
 * SimpleAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SimpleAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SimpleAV. If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "SAQueue.h"

#include <stdlib.h>

SAQContext *SAQ_init(void)
{
     SAQContext *p = (SAQContext *)malloc(sizeof(SAQContext));
     if(p == NULL)
          return p;
     
     p->head = p->tail = NULL;
     p->nb = 0;
     return p;
}

int SAQ_push(SAQContext *saq_ptr, void *data)
{
     _SAQNode *node_ptr = (_SAQNode *)malloc(sizeof(_SAQNode));
     if(node_ptr == NULL)
          return -1;
     
     node_ptr->next = NULL;
     node_ptr->data = data;
     if(saq_ptr->tail != NULL)
          saq_ptr->tail->next = node_ptr;
     saq_ptr->tail = node_ptr;
     if(saq_ptr->head == NULL)
          saq_ptr->head = node_ptr;
     saq_ptr->nb++;
     return 0;
}

void *SAQ_pop(SAQContext *saq_ptr)
{
     if(saq_ptr->nb == 0)
          return NULL;
     
     _SAQNode *head_ptr = saq_ptr->head;
     void *data = head_ptr->data;

     if(saq_ptr->nb == 1)
          saq_ptr->tail = NULL;
     saq_ptr->head = head_ptr->next;

     saq_ptr->nb--;
     
     free(head_ptr);
     return data;
}

#ifdef __cplusplus
}
#endif
