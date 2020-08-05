//
// Created by XingfengYang on 2020/7/17.
//

#ifndef __KERNEL_MUTEX_H__
#define __KERNEL_MUTEX_H__
#include "spinlock.h"
#include <atomic.h>
#include <kqueue.h>
#include <stdbool.h>
#include <stdint.h>

#define STATE_FREE 0u
#define STATE_CONTESTED 1u

#define MutexCreate()                                                                                                  \
  {                                                                                                                    \
    .val =                                                                                                             \
        {                                                                                                              \
            .counter = 0,                                                                                              \
        },                                                                                                             \
    .operations =                                                                                                      \
        {                                                                                                              \
            .acquire = mutex_default_acquire,                                                                          \
            .release = mutex_default_release,                                                                          \
        },                                                                                                             \
    .spinLock = SpinLockCreate(), .waitQueue = nullptr,                                                                \
  }

typedef void (*MutexAcquire)(struct Mutex *mutex);
typedef void (*MutexRelease)(struct Mutex *mutex);

typedef struct MutexOperations {
  MutexAcquire acquire;
  MutexRelease release;
} MutexOperations;

typedef struct Mutex {
  Atomic val;
  SpinLock spinLock;
  KQueue *waitQueue;
  MutexOperations operations;
} Mutex;

#endif // __KERNEL_MUTEX_H__
