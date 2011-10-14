/* Copyright (C) 2011 by Chenguang Wang(wecing)
 *
 * This program is free software. It comes without any warranty, to
 * the extent permitted by applicable law. You can redistribute it
 * and/or modify it under the terms of the Do What The Fuck You Want
 * To Public License, Version 2, as published by Sam Hocevar. See
 * http://sam.zoy.org/wtfpl/COPYING for more details.
 */
#ifndef __SIMPLEAV_SAMUTEX_H__DEFINED__
#define __SIMPLEAV_SAMUTEX_H__DEFINED__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A simple cross-platform (sorry, *BSD...) mutex.
 *
 * usage:
 * SAMutex mutex;
 * SAMutex_init(&mutex);
 * SAMutex_lock(&mutex);
 * SAMutex_unlock(&mutex);
 * SAMutex_destroy(&mutex);
 *
 * all functions return 0 on success, -1 on error.
*/

#if defined(__unix__)
     #include <pthread.h>
     #define SAMutex pthread_mutex_t
#elif defined(_WIN32)
     #include <windows.h>
     #include <process.h>
     #define SAMutex HANDLE
#endif

int SAMutex_init(SAMutex *);
int SAMutex_lock(SAMutex *);
int SAMutex_unlock(SAMutex *);
int SAMutex_destroy(SAMutex *);

#ifdef __cplusplus
}
#endif

#endif
