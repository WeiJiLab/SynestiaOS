//
// Created by XingfengYang on 2020/6/29.
//

#include "arm/interrupt.h"
#include "kernel/ktimer.h"
#include "kernel/scheduler.h"
#include "arm/register.h"
#include "kernel/interrupt.h"
#include "kernel/log.h"
#include "kernel/percpu.h"
#include "kernel/spinlock.h"
#include "libc/stdlib.h"

extern InterruptManager genericInterruptManager;
extern KernelTimerManager kernelTimerManager;
extern Scheduler cfsScheduler;

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

void tick() {
    LogInfo("[Schd]: tick.\n");
    cfsScheduler.operation.switchNext(&cfsScheduler);
}

SpinLock spinlock = SpinLockCreate();

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
    int flag = cfsScheduler.switch_to_signal;
    cfsScheduler.switch_to_signal = 0;
    if (flag == 0) {
        return;
    } else if (flag == 1) {
        schd_save_context(cfsScheduler.prevThread);
        schd_restore_context(cfsScheduler.currentThread);
        schd_switch_mm(cfsScheduler.currentThread);
    } else if (flag == 2) {
        schd_restore_context(cfsScheduler.currentThread);
        schd_switch_mm(cfsScheduler.currentThread);
    }
}

KernelStatus scheduler_default_operation_init(struct Scheduler *scheduler) {
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

    LogInfo("[Scheduler]: Scheduler inited.\n");
    return OK;
}

KernelStatus scheduler_default_operation_add_thread(struct Scheduler *scheduler, Thread *thread, uint32_t priority) {
    thread->priority = priority;
    PerCpu *perCpu = percpu_min_priority(thread->cpuAffinity);
    KernelStatus threadAddStatus = perCpu->operations.insertThread(perCpu, thread);
    if (threadAddStatus != OK) {
        LogError("[Scheduler] thread %s add to scheduler failed.\n", thread->name);
        return ERROR;
    }
    return OK;
}

KernelStatus scheduler_default_operation_schedule(struct Scheduler *scheduler) {
    tick_init(&scheduler->schedulerTick, tick, "scheduler tick");
    genericInterruptManager.operation.registerTick(&genericInterruptManager, &scheduler->schedulerTick);
    LogInfo("[Scheduler]: Schd started.\n");
    arch_move_interrupt_table();
    genericInterruptManager.operation.enableInterrupt(&genericInterruptManager);
    return OK;
}

KernelStatus scheduler_default_operation_block(struct Scheduler *scheduler) {
// TODO:
}

KernelStatus scheduler_default_operation_yield(struct Scheduler *scheduler) {
// TODO:
}

KernelStatus scheduler_default_operation_preempt(struct Scheduler *scheduler) {
    // TODO:
}

KernelStatus scheduler_default_operation_switch_to(struct Scheduler *scheduler, Thread *thread) {
    if (thread == nullptr) {
        LogWarn("[Scheduler]: cant switch to nullptr thread.\n");
        return ERROR;
    }
    LogInfo("[Scheduler]: switch to: %s.\n", thread->name);

    // save current thread
    if (scheduler->currentThread == nullptr) {
        scheduler->switch_to_signal = 2;
    } else {
        scheduler->switch_to_signal = 1;
    }
    scheduler->prevThread = scheduler->currentThread;
    scheduler->currentThread = thread;
    percpu_get(read_cpuid())->currentThread = thread;
    return OK;
}

KernelStatus scheduler_default_operation_switch_next(struct Scheduler *scheduler) {
    uint32_t cpuid = read_cpuid();
    LogWarn("[Schd]: cpuId %d .\n", cpuid);
    PerCpu *perCpu = percpu_get(cpuid);
    Thread *thread = perCpu->operations.getNextThread(perCpu);

    spinlock.operations.acquire(&spinlock);

    thread->runtimeNs += TIMER_TICK_MS;
    thread->runtimeVirtualNs += (PRIORITY_DEFAULT_WEIGHT / PRIORITY_2_WEIGHT[thread->priority]) * thread->runtimeNs;
    scheduler->operation.switchTo(scheduler, thread);

    spinlock.operations.release(&spinlock);

    if (thread != perCpu->idleThread) {
        Thread *removedThread = perCpu->operations.removeThread(perCpu, thread);
        scheduler->operation.addThread(scheduler, removedThread, removedThread->priority);
    }
    return OK;
}

uint32_t scheduler_default_operation_get_current_pid(struct Scheduler *scheduler) {
    uint32_t cpuid = read_cpuid();
    PerCpu *perCpu = percpu_get(cpuid);
    return perCpu->currentThread->pid;
}

Thread *scheduler_default_operation_get_current_thread(struct Scheduler *scheduler) {
    return scheduler->currentThread;
}

Scheduler *scheduler_create(Scheduler *scheduler) {
    scheduler->operation.init = (SchedulerOperationInit) scheduler_default_operation_init;
    scheduler->operation.addThread = (SchedulerOperationAddThread) scheduler_default_operation_add_thread;
    scheduler->operation.schedule = (SchedulerOperationSchedule) scheduler_default_operation_schedule;
    scheduler->operation.block = (SchedulerOperationBlock) scheduler_default_operation_block;
    scheduler->operation.yield = (SchedulerOperationYield) scheduler_default_operation_yield;
    scheduler->operation.preempt = (SchedulerOperationPreempt) scheduler_default_operation_preempt;
    scheduler->operation.switchTo = (SchedulerOperationSwitchTo) scheduler_default_operation_switch_to;
    scheduler->operation.switchNext = (SchedulerOperationSwitchNext) scheduler_default_operation_switch_next;
    scheduler->operation.getCurrentPid = (SchedulerOperationGetCurrentPid) scheduler_default_operation_get_current_pid;
    scheduler->operation.getCurrentThread = (SchedulerOperationGetCurrentThread) scheduler_default_operation_get_current_thread;

    scheduler->switch_to_signal = 0;
    scheduler->currentThread = nullptr;
    scheduler->prevThread = nullptr;
    return scheduler;
}
