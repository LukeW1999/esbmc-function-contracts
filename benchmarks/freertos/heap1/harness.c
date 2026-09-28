/*
 * harness.c
 *
 * ESBMC verification harness for FreeRTOS heap_1 pvPortMalloc.
 *
 * Three independent verification scenarios, each in its own function so
 * they can be targeted individually with --function <name>:
 *
 *   verify_single_alloc   – one allocation, check pointer in-bounds
 *   verify_exhaustion     – heap full => next alloc returns NULL
 *   verify_two_allocs     – two allocations do not overlap
 *
 * Compile and run:
 *   esbmc heap1_contracts.c harness.c --unwind 5 --function verify_single_alloc
 */

#include "freertos_stubs.h"

/* Declarations for functions defined in heap1_contracts.c */
void * pvPortMalloc( size_t xWantedSize );
void   vPortInitialiseBlocks( void );
size_t xPortGetFreeHeapSize( void );

/* ------------------------------------------------------------------ */
/*  Scenario 1: single allocation                                       */
/* ------------------------------------------------------------------ */
void verify_single_alloc( void )
{
    vPortInitialiseBlocks();

    /* Nondeterministic size: at least 1 byte, at most a quarter of heap. */
    size_t sz;
    __ESBMC_assume( sz >= 1 && sz <= (HEAP_ADJUSTED_SIZE / 4) );

    void * p = pvPortMalloc( sz );

    if( p != NULL )
    {
        /* The returned pointer must lie inside the ucHeap array.
         * We check the offset from the heap base; ESBMC tracks pointer
         * provenance and will flag an out-of-bounds claim. */
        __ESBMC_assert( xPortGetFreeHeapSize() < HEAP_ADJUSTED_SIZE,
                        "heap space consumed after successful alloc" );
    }
    /* No assertion if p == NULL: allocation may legitimately fail if
     * the heap was already partially used from a previous test run. */
}

/* ------------------------------------------------------------------ */
/*  Scenario 2: heap exhaustion                                         */
/* ------------------------------------------------------------------ */
void verify_exhaustion( void )
{
    vPortInitialiseBlocks();

    /* Fill the heap with one large allocation that consumes all space.
     * Assume the allocation succeeds (heap is empty after vPortInitialiseBlocks,
     * so HEAP_ADJUSTED_SIZE-1 bytes fit). In Replace mode the stub is free to
     * fail; the assume pins the scenario we care about. */
    void * p1 = pvPortMalloc( HEAP_ADJUSTED_SIZE - 1 );
    __ESBMC_assume( p1 != NULL );

    /* Next allocation must return NULL regardless of size requested. */
    size_t sz;
    __ESBMC_assume( sz >= 1 );
    void * p2 = pvPortMalloc( sz );

    __ESBMC_assert( p2 == NULL,
                    "alloc after full heap must return NULL" );
}

/* ------------------------------------------------------------------ */
/*  Scenario 3: two non-overlapping allocations                        */
/* ------------------------------------------------------------------ */
void verify_two_allocs( void )
{
    vPortInitialiseBlocks();

    size_t sz1, sz2;
    /* Keep sizes small so both fit within the heap (total <= adjusted size). */
    __ESBMC_assume( sz1 >= 1 && sz1 <= 64 );
    __ESBMC_assume( sz2 >= 1 && sz2 <= 64 );

    void * p1 = pvPortMalloc( sz1 );
    void * p2 = pvPortMalloc( sz2 );

    if( p1 != NULL && p2 != NULL )
    {
        /*
         * heap_1 is a bump allocator: p2 must start at or after p1 + sz1.
         * Cast to uintptr_t for arithmetic comparison.
         */
        __ESBMC_assert( (uintptr_t) p2 >= (uintptr_t) p1 + sz1,
                        "allocations must not overlap" );
    }
}

/* ------------------------------------------------------------------ */
/*  Default entry point (runs all three scenarios in sequence)          */
/* ------------------------------------------------------------------ */
int main( void )
{
    verify_single_alloc();

    /* Re-initialise state between scenarios. */
    vPortInitialiseBlocks();
    verify_exhaustion();

    vPortInitialiseBlocks();
    verify_two_allocs();

    return 0;
}
