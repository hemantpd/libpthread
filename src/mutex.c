/*
 * Copyright (c) 2011, Dongsheng Song <songdongsheng@live.cn>
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file mutex.c
 * @brief Implementation Code of Mutex Routines
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <winsock2.h>

#include "arch.h"
#include "misc.h"

/**
 * Create a mutex attribute object.
 * @param attr The pointer of the mutex attribute object.
 * @return Always return 0.
 * @remark We provide pthread_mutexattr_* functions only for compatibility,
 *         please use pthread_mutex_init(&mutex, NULL) for new applications.
 */
int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    arch_mutex_attr *pv = calloc(1, sizeof(arch_mutex_attr));
    if (pv == NULL)
        return set_errno(ENOMEM);

    pv->type = PTHREAD_MUTEX_DEFAULT;
    pv->pshared = PTHREAD_PROCESS_PRIVATE;

    *attr = pv;

    return 0;
}

/**
 * Get the mutex type attribute.
 * @param attr The pointer of the mutex attribute object.
 * @param type The mutex type.
 * @return Always return 0.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    arch_mutex_attr *pv = (arch_mutex_attr *) attr;
    *type = pv->type;
    return 0;
}

/**
 * Set the mutex type attribute.
 * @param attr The pointer of the mutex attribute object.
 * @param type The mutex type.
 * @return Always return 0.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    arch_mutex_attr *pv = (arch_mutex_attr *) attr;
    pv->type = type;
    return 0;
}

/**
 * Get the mutex process-shared attribute.
 * @param attr The pointer of the mutex attribute object.
 * @param pshared The process-shared attribute.
 * @return Always return 0.
 * @remark The only type we support is PTHREAD_PROCESS_PRIVATE.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *attr, int *pshared)
{
    arch_mutex_attr *pv = (arch_mutex_attr *) attr;
    *pshared = pv->pshared;
    return 0;
}

/**
 * Set the mutex process-shared attribute.
 * @param attr The pointer of the mutex attribute object.
 * @param pshared The process-shared attribute.
 * @return Always return 0.
 * @remark The only type we support is PTHREAD_PROCESS_PRIVATE.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared)
{
    if (pshared != PTHREAD_PROCESS_PRIVATE)
        return set_errno(EINVAL);

    arch_mutex_attr *pv = (arch_mutex_attr *) attr;
    pv->pshared = pshared;
    return 0;
}

/**
 * Get the mutex protocol attribute.
 * @param attr The pointer of the mutex attribute object.
 * @param protocol The mutex protocol.
 * @return Always return 0.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
    arch_mutex_attr *pv = (arch_mutex_attr *) attr;
    *protocol = pv->protocol;
    return 0;
}

/**
 * Set the mutex protocol attribute.
 * @param attr The pointer of the mutex attribute object.
 * @param protocol The mutex protocol.
 * @return Always return 0.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
    arch_mutex_attr *pv = (arch_mutex_attr *) attr;
    pv->protocol = protocol;
    return 0;
}

/**
 * Get the mutex prioceiling attribute.
 * @param attr The pointer of the mutex attribute object.
 * @param prioceiling The mutex prioceiling attribute.
 * @return Always return 0.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *attr, int *prioceiling)
{
    arch_mutex_attr *pv = (arch_mutex_attr *) attr;
    *prioceiling = pv->prioceiling;
    return 0;
}

/**
 * Set the mutex prioceiling attribute.
 * @param attr The pointer of the mutex attribute object.
 * @param prioceiling The mutex prioceiling attribute.
 * @return Always return 0.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int prioceiling)
{
    arch_mutex_attr *pv = (arch_mutex_attr *) attr;
    pv->prioceiling = prioceiling;
    return 0;
}

/**
 * Destroy a mutex attribute object.
 * @param attr The pointer of the mutex attribute object.
 * @return Always return 0.
 * @remark This function is provided for source code compatibility but no effect when called.
 */
int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    if (attr != NULL) {
        free(*attr);
        *attr = NULL;
    }

    return 0;
}

static int arch_mutex_init(pthread_mutex_t *m, int lock)
{
    arch_mutex *pv = calloc(1, sizeof(arch_mutex));
    if (pv == NULL)
        return set_errno(ENOMEM);

    InitializeCriticalSection(& pv->mutex);

    if (!lock) {
        *m = pv;
        return 0;
    }

    if (atomic_cmpxchg_ptr(m, pv, NULL) != NULL) {
        DeleteCriticalSection(& pv->mutex);
        free(pv);
    }

    return 0;
}

/**
 * Create a mutex object.
 * @param m The pointer of the mutex object.
 * @param a The pointer of the mutex attribute object.
 * @return If the function succeeds, the return value is 0.
 *         If the function fails, the return value is -1,
 *         with errno set to indicate the error (ENOMEM).
 * @remark We provide pthread_mutexattr_* functions only for compatibility,
 *         please use pthread_mutex_init(&mutex, NULL) for new applications.
 */
int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *a)
{
    *m = NULL;
    return arch_mutex_init(m, 0);
}

/**
 * Acquire a mutex lock.
 * @param m The pointer of the mutex object.
 * @return If the function succeeds, the return value is 0.
 *         If the function fails, the return value is -1,
 *         with errno set to indicate the error (ENOMEM).
 */
int pthread_mutex_lock(pthread_mutex_t *m)
{
    arch_mutex *pv;

    if (*m == NULL) {
        int rc = arch_mutex_init(m, 1);
        if (rc != 0) return rc;
    }

    pv = (arch_mutex *) *m;
    EnterCriticalSection(& pv->mutex);

    return 0;
}

/**
 * Try acquire a mutex lock.
 * @param m The pointer of the mutex object.
 * @return If the function succeeds, the return value is 0.
 *         If the function fails, the return value is -1,
 *         with errno set to indicate the error (ENOMEM or EBUSY).
 */
int pthread_mutex_trylock(pthread_mutex_t *m)
{
    arch_mutex *pv;

    if (*m == NULL) {
        int rc = arch_mutex_init(m, 1);
        if (rc != 0) return rc;
    }

    pv = (arch_mutex *) *m;
    if( 0 != TryEnterCriticalSection(& pv->mutex))
        return 0;

    return set_errno(EBUSY);
}

/**
 * Release a mutex lock.
 * @param m The pointer of the mutex object.
 * @return If the function succeeds, the return value is 0.
 *         If the function fails, the return value is -1,
 *         with errno set to indicate the error (EINVAL).
 */
int pthread_mutex_unlock(pthread_mutex_t *m)
{
    arch_mutex *pv = (arch_mutex *) *m;
    if (pv != NULL) {
        LeaveCriticalSection(& pv->mutex);
        return 0;
    }

    return set_errno(EINVAL);
}

/**
 * Destroy a mutex lock.
 * @param m The pointer of the mutex object.
 * @return Always return 0.
 */
int pthread_mutex_destroy(pthread_mutex_t *m)
{
    arch_mutex *pv = (arch_mutex *) *m;
    if (pv != NULL) {
        DeleteCriticalSection(& pv->mutex);
        free(pv);
    }

    return 0;
}
