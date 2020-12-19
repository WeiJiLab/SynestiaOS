//
// Created by XingfengYang on 2020/11/23.
//
#include "arm/page.h"
#include "kernel/slab.h"
#include "kernel/log.h"
#include "kernel/kobject.h"
#include "kernel/mutex.h"
#include "kernel/semaphore.h"
#include "kernel/Thread.h"
#include "kernel/type.h"
#include "kernel/kheap.h"

extern PhysicalPageAllocator kernelPageAllocator;
extern Heap kernelHeap;

void slab_default_alloc_callback(struct Slab *slab, KernelObjectType type, void *ptr) {
}

void slab_default_free_callback(struct Slab *slab, KernelObjectType type, void *ptr) {
}

void *slab_default_alloc(struct Slab *slab, KernelObjectType type) {
    // can not found any kernel object from kernel object lists
    if (slab->kernelObjects[type] == nullptr) {
        // alloc a new kernel object from heap and link it to kernel object list
        switch (type) {
            case KERNEL_OBJECT_THREAD: {
                Thread *thread = kernelHeap.operations.alloc(&kernelHeap, sizeof(Thread));
                slab->kernelObjects[type] = &thread->object;
                thread->object.status = USING;
                return thread;
            }
            case KERNEL_OBJECT_MUTEX: {
                Mutex *mutex = kernelHeap.operations.alloc(&kernelHeap, sizeof(Mutex));
                slab->kernelObjects[type] = &mutex->object;
                mutex->object.status = USING;
                return mutex;
            }
            case KERNEL_OBJECT_SEMAPHORE: {
                Semaphore *semaphore = kernelHeap.operations.alloc(&kernelHeap, sizeof(Semaphore));
                slab->kernelObjects[type] = &semaphore->object;
                semaphore->object.status = USING;
                return semaphore;
            }
            case KERNEL_OBJECT_FILE_DESCRIPTOR: {
                FileDescriptor *fileDescriptor = kernelHeap.operations.alloc(&kernelHeap, sizeof(FileDescriptor));
                slab->kernelObjects[type] = &fileDescriptor->object;
                fileDescriptor->object.status = USING;
                return fileDescriptor;
            }
        }
    } else {
        KernelObject *kernelObject = slab->kernelObjects[type];

        // the first of kernel object list is free
        if (kernelObject->status == FREE) {
            // just use it, and mark it as used
            kernelObject->status = USING;
            return kernelObject->operations.getObject(&kernelObject);
        } else {
            // let find the free kernel object from list
            while (kernelObject->next != nullptr) {
                if (kernelObject->next->status == FREE) {
                    kernelObject->status = USING;
                    return kernelObject->operations.getObject(&kernelObject);
                }
                kernelObject = kernelObject->next;
            }
            // oops, not found free kernel object, so let's alloc from heap.
            switch (type) {
                case KERNEL_OBJECT_THREAD: {
                    Thread *thread = kernelHeap.operations.alloc(&kernelHeap, sizeof(Thread));
                    slab->kernelObjects[type] = &thread->object;
                    thread->object.status = USING;
                    return thread;
                }
                case KERNEL_OBJECT_MUTEX: {
                    Mutex *mutex = kernelHeap.operations.alloc(&kernelHeap, sizeof(Mutex));
                    slab->kernelObjects[type] = &mutex->object;
                    mutex->object.status = USING;
                    return mutex;
                }
                case KERNEL_OBJECT_SEMAPHORE: {
                    Semaphore *semaphore = kernelHeap.operations.alloc(&kernelHeap, sizeof(Semaphore));
                    slab->kernelObjects[type] = &semaphore->object;
                    semaphore->object.status = USING;
                    return semaphore;
                }
                case KERNEL_OBJECT_FILE_DESCRIPTOR: {
                    FileDescriptor *fileDescriptor = kernelHeap.operations.alloc(&kernelHeap, sizeof(FileDescriptor));
                    slab->kernelObjects[type] = &fileDescriptor->object;
                    fileDescriptor->object.status = USING;
                    return fileDescriptor;
                }
            }
        }
    }
}

KernelStatus slab_default_free(struct Slab *slab, void *ptr) {
}

void slab_default_set_alloc_callback(struct Slab *slab, SlabAllocCallback callback) {
}

void slab_default_set_free_callback(struct Slab *slab, SlabFreeCallback callback) {
}

KernelStatus slab_create(Slab *slab, uint32_t addr, uint32_t size) {
    slab->allocCallback = slab_default_alloc_callback;
    slab->freeCallback = slab_default_free_callback;

    slab->address = addr;
    LogInfo("[KSlab] at: %d. \n", slab->address);

    PhysicalPageAllocator *physicalPageAllocator = &kernelPageAllocator;

    // allocate physical page for slab
    uint32_t slabPhysicalPage = (uint32_t) physicalPageAllocator->operations.allocHugeAt(
            physicalPageAllocator, USAGE_KERNEL_HEAP, (slab->address | 4 * KB) >> VA_OFFSET, size - slab->address);
    LogInfo("[KSlab] alloc slab page: %d. \n", (uint32_t) slabPhysicalPage);

    slab->address = KERNEL_PHYSICAL_START + slabPhysicalPage * PAGE_SIZE;
    LogInfo("[KHeap] kheap at: %d. \n", slab->address);


    slab->operations.setFreeCallback = slab_default_set_free_callback;
    slab->operations.setAllocCallback = slab_default_set_alloc_callback;

    slab->operations.alloc = slab_default_alloc;
    slab->operations.free = slab_default_free;


    LogInfo("[KSlab] kslab created. \n");
    return OK;
}
