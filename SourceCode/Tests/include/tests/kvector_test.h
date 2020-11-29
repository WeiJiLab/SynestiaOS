//
// Created by XingfengYang on 2020/7/6.
//

#ifndef __KERNEL_KVECTOR_TEST_H__
#define __KERNEL_KVECTOR_TEST_H__

#include "kernel/kvector.h"

extern uint32_t __HEAP_BEGIN;

void should_kvector_create() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);
}

void should_kvector_resize() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    kvector_resize(kernelVector, DEFAULT_VECTOR_SIZE * 2);
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE * 2);
}

void should_kvector_free() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    kvector_free(kernelVector);
}

void should_kvector_add() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    ListNode node;
    kvector_add(kernelVector, &node);
    kvector_add(kernelVector, &node);
    ASSERT_EQ(kernelVector->index, 2);
}

typedef struct TestElement {
    uint32_t id;
    ListNode node;
} TestElement;
TestElement t1;
TestElement t2;

void should_kvector_get() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    t1.id = 1;
    t1.node.next = nullptr;
    t1.node.prev = nullptr;

    t2.id = 2;
    t2.node.next = nullptr;
    t2.node.prev = nullptr;

    kvector_add(kernelVector, &(t1.node));
    kvector_add(kernelVector, &(t2.node));
    ASSERT_EQ(kernelVector->index, 2);

    ASSERT_EQ(getNode(kvector_get(kernelVector, 0), TestElement, node)->id, 1);
    ASSERT_EQ(getNode(kvector_get(kernelVector, 1), TestElement, node)->id, 2);
}

void should_kvector_remove_index() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    // TODO:
}

void should_kvector_remove() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    // TODO:
}

void should_kvector_is_empty() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    ASSERT_TRUE(kvector_is_empty(kernelVector));
}

void should_kvector_is_full() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    ListNode node;
    for (uint32_t i = 0; i < DEFAULT_VECTOR_SIZE; i++) {
        kvector_add(kernelVector, &node);
    }
    ASSERT_TRUE(kvector_is_full(kernelVector));
}

void should_kvector_clear() {
    KernelStatus heapInitStatus = heap_create(&testHeap, __HEAP_BEGIN, 64 * MB);

    KernelVector *kernelVector = kvector_allocate();
    ASSERT_NEQ(kernelVector, nullptr);
    ASSERT_EQ(kernelVector->index, 0);
    ASSERT_EQ(kernelVector->size, DEFAULT_VECTOR_SIZE);

    ListNode node;
    for (uint32_t i = 0; i < DEFAULT_VECTOR_SIZE; i++) {
        kvector_add(kernelVector, &node);
    }

    kvector_clear(kernelVector);
    ASSERT_TRUE(kvector_is_empty(kernelVector));
    ASSERT_EQ(kernelVector->index, 0);
}

#endif//__KERNEL_KVECTOR_TEST_H__
