/*
 * SAMutex.h
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
