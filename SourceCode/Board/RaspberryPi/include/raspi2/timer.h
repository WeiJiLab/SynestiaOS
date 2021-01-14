#ifndef __BOARD_RASP_TIMER_H__
#define __BOARD_RASP_TIMER_H__

#include "libc/stdint.h"
#include "raspi2/raspi.h"

#define SYSTEM_TIMER_BASE (SYSTEM_TIMER_OFFSET + PERIPHERAL_BASE)
#define CORE0_TIMER_IRQCNTL 0x40000040
#define CORE0_IRQ_SOURCE 0x40000060

/**
 * timer init
 */
void system_timer_init(void);

void timer_set(uint32_t usecs);

extern void io_writel(uint32_t val, void *addr);

void generic_timer_init(void);

typedef struct {
    uint8_t timer0_matched : 1;
    uint8_t timer1_matched : 1;
    uint8_t timer2_matched : 1;
    uint8_t timer3_matched : 1;
    uint32_t reserved : 28;
} timer_control_reg_t;

typedef struct {
    timer_control_reg_t control;
    uint32_t counter_low;
    uint32_t counter_high;
    uint32_t timer0;
    uint32_t timer1;
    uint32_t timer2;
    uint32_t timer3;
} timer_registers_t;

#endif// __BOARD_RASP_TIMER_H__
