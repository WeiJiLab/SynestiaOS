//
// Created by XingfengYang on 2020/6/29.
//

#include "kernel/sched.h"
#include "arm/register.h"
#include "kernel/interrupt.h"
#include "kernel/log.h"
#include "kernel/percpu.h"
#include "kernel/spinlock.h"
#include "libc/stdlib.h"

extern InterruptManager genericInterruptManager;

uint32_t PRIORITY_2_WEIGHT[40] = {
        88761,
        71755,
        56483,
        46273,
        36291,
        29154,
        23254,
        18705,
        14949,
        11916,
        9548,
        7620,
        6100,
        4904,
        3906,
        3121,
        2501,
        1991,
        1586,
        1277,
        1024,
        820,
        655,
        526,
        423,
        335,
        272,
        215,
        172,
        137,
        110,
        87,
        70,
        56,
        45,
        36,
        29,
        23,
        18,
        15,
};

#define PRIORITY_DEFAULT_WEIGHT 1024

extern uint64_t ktimer_sys_runtime_tick(uint64_t tickInterval);

extern void cpu_save_context(Thread *thread, uint32_t offsetOfStack);

extern void cpu_restore_context(Thread *thread, uint32_t offsetOfStack);

extern void cpu_switch_mm(uint32_t pageTable);


#define TIMER_TICK_MS 50

uint32_t switch_to_signal = 0;

Thread *prevThread = nullptr;
Thread *currentThread = nullptr;

Tick schedulerTick;

void tick() {
    LogInfo("[Schd]: tick.\n");
    ktimer_sys_runtime_tick(TIMER_TICK_MS);
    schd_switch_next();
}

SpinLock spinlock = SpinLockCreate();

KernelStatus schd_switch_next(void) {
    uint32_t cpuid = read_cpuid();
    LogWarn("[Schd]: cpuId %d .\n", cpuid);
    PerCpu *perCpu = percpu_get(cpuid);
    Thread *thread = perCpu->operations.getNextThread(perCpu);

    spinlock.operations.acquire(&spinlock);

    thread->runtimeNs += TIMER_TICK_MS;
    thread->runtimeVirtualNs += (PRIORITY_DEFAULT_WEIGHT / PRIORITY_2_WEIGHT[thread->priority]) * thread->runtimeNs;
    schd_switch_to(thread);

    spinlock.operations.release(&spinlock);

    if (thread != perCpu->idleThread) {
        Thread *removedThread = perCpu->operations.removeThread(perCpu, thread);
        schd_add_thread(removedThread, removedThread->priority);
    }
    return OK;
}

KernelStatus schd_init() {
    if (percpu_create(SMP_MAX_CPUS) != ERROR) {
        for (CpuNum cpuId = 0; cpuId < SMP_MAX_CPUS; cpuId++) {
            Thread *idleThread = thread_create_idle_thread(cpuId);
            if (idleThread == nullptr) {
                return ERROR;
            }
            PerCpu *perCpu = percpu_get(cpuId);
            perCpu->operations.init(perCpu, cpuId, idleThread);
        }
    }

    LogInfo("[Schd]: Schd inited.\n");
    return OK;
}

KernelStatus schd_add_thread(Thread *thread, uint32_t priority) {
    thread->priority = priority;
    PerCpu *perCpu = percpu_min_priority(thread->cpuAffinity);
    KernelStatus threadAddStatus = perCpu->operations.insertThread(perCpu, thread);
    if (threadAddStatus != OK) {
        LogError("[Schd] thread %s add to scheduler failed.\n", thread->name);
        return ERROR;
    }
    return OK;
}

KernelStatus schd_schedule(void) {
    tick_init(&schedulerTick,tick);
    genericInterruptManager.operation.registerTick(&genericInterruptManager,&schedulerTick);
    LogInfo("[Schd]: Schd started.\n");
    genericInterruptManager.operation.enableInterrupt(&genericInterruptManager);
    return OK;
}


void schd_save_context(Thread *thread) {
    uint32_t offsetOfStack = offsetOf(Thread, stack);
    uint32_t offsetOfStackTop = offsetOf(KernelStack, top);

    cpu_save_context(thread, offsetOfStack + offsetOfStackTop);
}

void schd_restore_context(Thread *thread) {
    uint32_t offsetOfStack = offsetOf(Thread, stack);
    uint32_t offsetOfStackTop = offsetOf(KernelStack, top);

    cpu_restore_context(thread, offsetOfStack + offsetOfStackTop);
}

void schd_switch_mm(Thread *thread) {
    cpu_switch_mm((uint32_t) thread->memoryStruct.virtualMemory.pageTable);
}

void schd_switch_context() {
    int flag = switch_to_signal;
    switch_to_signal = 0;
    if (flag == 0) {
        return;
    } else if (flag == 1) {
        schd_save_context(prevThread);
        schd_restore_context(currentThread);
        schd_switch_mm(currentThread);
    } else if (flag == 2) {
        schd_restore_context(currentThread);
        schd_switch_mm(currentThread);
    }
}

KernelStatus schd_switch_to(Thread *thread) {
    if (thread == nullptr) {
        LogWarn("[Schd]: cant switch to nullptr thread.\n");
        return ERROR;
    }
    LogInfo("[Schd]: switch to: %s.\n", thread->name);

    // save current thread
    if (currentThread == nullptr) {
        switch_to_signal = 2;
    } else {
        switch_to_signal = 1;
    }
    prevThread = currentThread;
    currentThread = thread;
    percpu_get(read_cpuid())->currentThread = thread;
    return OK;
}

KernelStatus schd_block(void) {
    // TODO:
    return OK;
}

KernelStatus schd_yield(void) {
    // TODO:
    return OK;
}

KernelStatus schd_preempt(void) {
    // TODO:
    return OK;
}

uint32_t schd_getpid() {
    uint32_t cpuid = read_cpuid();
    PerCpu *perCpu = percpu_get(cpuid);
    return perCpu->currentThread->pid;
}

Thread *schd_get_current_thread(void) { return currentThread; }
