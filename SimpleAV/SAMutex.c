/*
 * SAMutex.c
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

#include "SAMutex.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * A simple cross-platform(sorry, *BSD...) mutex.
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

int SAMutex_init(SAMutex *mutex)
{
     if(mutex == NULL)
          return -1;
#if defined(__unix__)
     return pthread_mutex_init(mutex, NULL) == 0 ? 0 : -1;
#elif defined(_WIN32)
     *mutex = CreateMutex(0, FALSE, 0);
     return *mutex == NULL ? -1 : 0;
#endif
     return -1;
}

int SAMutex_lock(SAMutex *mutex)
{
     if(mutex == NULL)
          return -1;
#if defined(__unix__)
     return pthread_mutex_lock(mutex) == 0 ? 0 : -1;
#elif defined(_WIN32)
     return WaitForSingleObject(*mutex, INFINITE) == WAIT_FAILED ? -1 : 0;
#endif
     return -1;
}

int SAMutex_unlock(SAMutex *mutex)
{
     if(mutex == NULL)
          return -1;
#if defined(__unix__)
     return pthread_mutex_unlock(mutex) == 0 ? 0 : -1;
#elif defined(_WIN32)
     return ReleaseMutex(*mutex) != 0 ? 0 : -1;
#endif
     return -1;
}

int SAMutex_destroy(SAMutex *mutex)
{
     if(mutex == NULL)
          return -1;
#if defined(__unix__)
     return pthread_mutex_destroy(mutex) == 0 ? 0 : -1;
#elif defined(_WIN32)
     return CloseHandle(mutex) != 0 ? 0 : -1;
#endif
     return -1;
}

#ifdef __cplusplus
}
#endif
