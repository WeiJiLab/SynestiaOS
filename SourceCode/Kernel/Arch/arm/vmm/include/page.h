//
// Created by XingfengYang on 2020/6/15.
//

#ifndef __KERNEL_PAGE_H__
#define __KERNEL_PAGE_H__

#include <stdint.h>

#define PAHE_TABLE_SIZE 0x805000

#define KERNEL_L1PT_NUMBER 4
#define KERNEL_L2PT_NUMBER 128
#define KERNEL_PTE_NUMBER 512

#define VA_OFFSET 12
#define KERNEL_PHYSICAL_START 0
#define PAGE_SIZE 4 * KB

typedef struct PageTableEntry {
  /* These are used in all kinds of entry. */
  uint64_t valid : 1; /* Valid mapping */
  uint64_t table : 1; /* == 1 in 4k map entries too */

  /* These ten bits are only used in Block entries and are ignored in Table entries. */
  uint64_t ai : 3;   /* Attribute Index */
  uint64_t ns : 1;   /* Not-Secure */
  uint64_t user : 1; /* User-visible */
  uint64_t ro : 1;   /* Read-Only */
  uint64_t sh : 2;   /* Shareability */
  uint64_t af : 1;   /* Access Flag */
  uint64_t ng : 1;   /* Not-Global */

  /* The base address must be appropriately aligned for Block entries */
  uint64_t base : 28; /* Base address of block or next table */
  uint64_t sbz : 12;  /* Must be zero */

  /* These seven bits are only used in Block entries and are ignored in Table entries. */
  uint64_t hint : 1;  /* In a block of 16 contiguous entries */
  uint64_t pxn : 1;   /* Privileged-XN */
  uint64_t xn : 1;    /* eXecute-Never */
  uint64_t avail : 4; /* Ignored by hardware */

  /* These 5 bits are only used in Table entries and are ignored in Block entries */
  uint64_t pxnt : 1; /* Privileged-XN */
  uint64_t xnt : 1;  /* eXecute-Never */
  uint64_t apt : 2;  /* Access Permissions */
  uint64_t nst : 1;  /* Not-Secure */
} __attribute__((packed)) PTE;

typedef struct PageTable {
  PTE pte[KERNEL_PTE_NUMBER * KERNEL_PTE_NUMBER];
} PT;

typedef struct Level2PageTable {
  PTE pte[KERNEL_PTE_NUMBER];
} L2PT;

typedef struct Level1PageTable {
  PTE pte[KERNEL_L1PT_NUMBER];
} L1PT;

typedef enum PhysicalPageType {
  PAGE_UNKNOWD = 0,
  PAGE_4K,
  PAGE_2M,
} PhysicalPageType;

typedef enum PhysicalPageUsage {
  USAGE_UNKNOWD = 0,
  USAGE_KERNEL,
  USAGE_KERNEL_HEAP,
  USAGE_USER,
  USAGE_PERIPHERAL,
  USAGE_FRAMEBUFFER,
  USAGE_PAGETABLE,
} PhysicalPageUsage;

typedef struct PhysicalPage {
  uint64_t ref_count : 8;
  PhysicalPageType type : 8;
  PhysicalPageUsage usage : 8;
  uint64_t reserved : 8;
} __attribute__((packed)) PhysicalPage;

uint64_t page_alloc(PhysicalPageUsage usage);

uint64_t page_free(uint64_t page);

uint64_t page_alloc_huge(PhysicalPageUsage usage);

uint64_t page_alloc_huge_at(PhysicalPageUsage usage, uint64_t page, uint64_t size);

uint64_t page_free_huge(uint64_t page, uint64_t size);

#endif // __KERNEL_PAGE_H__
