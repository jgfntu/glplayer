/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#ifndef __SIMPLEAV_SAQUEUE_H__DEFINED__
#define __SIMPLEAV_SAQUEUE_H__DEFINED__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SAQNode {
     void *data;
     struct _SAQNode *next;
} _SAQNode;

typedef struct SAQContext {
     _SAQNode *head, *tail;
     int nb;
} SAQContext;

SAQContext *SAQ_init(void);

int SAQ_push(SAQContext *, void *);

void *SAQ_pop(SAQContext *);

#ifdef __cplusplus
}
#endif

#endif
