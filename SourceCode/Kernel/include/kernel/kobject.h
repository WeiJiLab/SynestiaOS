//
// Created by XingfengYang on 2020/11/22.
//

#ifndef __SYNESTIAOS_KOBJECT_H__
#define __SYNESTIAOS_KOBJECT_H__

#include "libc/stdint.h"

typedef enum KernelObjectType {
    KERNEL_OBJECT_THREAD = 0,
    KERNEL_OBJECT_MUTEX,
    KERNEL_OBJECT_SEMAPHORE,
    KERNEL_OBJECT_FILE_DESCRIPTOR,
} KernelObjectType;


typedef enum KernelObjectStatus {
    USING,
    FREE,
} KernelObjectStatus;

typedef struct KernelObject {
    KernelObjectType type;
    KernelObjectStatus status;
    struct KernelObject *next;
} KernelObject;

#endif//__SYNESTIAOS_KOBJECT_H__
