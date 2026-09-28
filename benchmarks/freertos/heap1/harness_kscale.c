/*
 * harness_kscale.c
 *
 * K-scaling harness for FreeRTOS heap_1 pvPortMalloc.
 *
 * Each verify_kN function calls pvPortMalloc N times and checks a basic
 * safety property: if all allocations succeed, the remaining free heap
 * equals the initial size minus the total allocated bytes.
 *
 * Run with:
 *   esbmc heap1_contracts.c harness_kscale.c --function verify_k1 --unwind 5
 *   esbmc heap1_contracts.c harness_kscale.c --replace-call-with-contract pvPortMalloc \
 *         --function verify_k1 --unwind 5
 */

#include "freertos_stubs.h"

void * pvPortMalloc( size_t xWantedSize );
void   vPortInitialiseBlocks( void );
size_t xPortGetFreeHeapSize( void );

/* Maximum allocation per call: keep small so K calls don't exhaust heap */
#define MAX_ALLOC 16

/* ------------------------------------------------------------------ */
/*  K = 1                                                               */
/* ------------------------------------------------------------------ */
void verify_k1( void )
{
    vPortInitialiseBlocks();

    size_t sz1;
    __ESBMC_assume( sz1 >= 1 && sz1 <= MAX_ALLOC );

    void *p1 = pvPortMalloc( sz1 );

    if( p1 != NULL )
    {
        __ESBMC_assert( xPortGetFreeHeapSize() == HEAP_ADJUSTED_SIZE - sz1,
                        "k1: free heap accounting correct" );
    }
}

/* ------------------------------------------------------------------ */
/*  K = 2                                                               */
/* ------------------------------------------------------------------ */
void verify_k2( void )
{
    vPortInitialiseBlocks();

    size_t sz1, sz2;
    __ESBMC_assume( sz1 >= 1 && sz1 <= MAX_ALLOC );
    __ESBMC_assume( sz2 >= 1 && sz2 <= MAX_ALLOC );

    void *p1 = pvPortMalloc( sz1 );
    void *p2 = pvPortMalloc( sz2 );

    if( p1 != NULL && p2 != NULL )
    {
        __ESBMC_assert( xPortGetFreeHeapSize() == HEAP_ADJUSTED_SIZE - sz1 - sz2,
                        "k2: free heap accounting correct" );
    }
}

/* ------------------------------------------------------------------ */
/*  K = 5                                                               */
/* ------------------------------------------------------------------ */
void verify_k5( void )
{
    vPortInitialiseBlocks();

    size_t sz1, sz2, sz3, sz4, sz5;
    __ESBMC_assume( sz1 >= 1 && sz1 <= MAX_ALLOC );
    __ESBMC_assume( sz2 >= 1 && sz2 <= MAX_ALLOC );
    __ESBMC_assume( sz3 >= 1 && sz3 <= MAX_ALLOC );
    __ESBMC_assume( sz4 >= 1 && sz4 <= MAX_ALLOC );
    __ESBMC_assume( sz5 >= 1 && sz5 <= MAX_ALLOC );

    void *p1 = pvPortMalloc( sz1 );
    void *p2 = pvPortMalloc( sz2 );
    void *p3 = pvPortMalloc( sz3 );
    void *p4 = pvPortMalloc( sz4 );
    void *p5 = pvPortMalloc( sz5 );

    if( p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL )
    {
        __ESBMC_assert(
            xPortGetFreeHeapSize() == HEAP_ADJUSTED_SIZE - sz1 - sz2 - sz3 - sz4 - sz5,
            "k5: free heap accounting correct" );
    }
}

/* ------------------------------------------------------------------ */
/*  K = 10                                                              */
/* ------------------------------------------------------------------ */
void verify_k10( void )
{
    vPortInitialiseBlocks();

    size_t sz1, sz2, sz3, sz4, sz5, sz6, sz7, sz8, sz9, sz10;
    __ESBMC_assume( sz1 >= 1 && sz1 <= MAX_ALLOC );
    __ESBMC_assume( sz2 >= 1 && sz2 <= MAX_ALLOC );
    __ESBMC_assume( sz3 >= 1 && sz3 <= MAX_ALLOC );
    __ESBMC_assume( sz4 >= 1 && sz4 <= MAX_ALLOC );
    __ESBMC_assume( sz5 >= 1 && sz5 <= MAX_ALLOC );
    __ESBMC_assume( sz6 >= 1 && sz6 <= MAX_ALLOC );
    __ESBMC_assume( sz7 >= 1 && sz7 <= MAX_ALLOC );
    __ESBMC_assume( sz8 >= 1 && sz8 <= MAX_ALLOC );
    __ESBMC_assume( sz9 >= 1 && sz9 <= MAX_ALLOC );
    __ESBMC_assume( sz10 >= 1 && sz10 <= MAX_ALLOC );

    void *p1  = pvPortMalloc( sz1 );
    void *p2  = pvPortMalloc( sz2 );
    void *p3  = pvPortMalloc( sz3 );
    void *p4  = pvPortMalloc( sz4 );
    void *p5  = pvPortMalloc( sz5 );
    void *p6  = pvPortMalloc( sz6 );
    void *p7  = pvPortMalloc( sz7 );
    void *p8  = pvPortMalloc( sz8 );
    void *p9  = pvPortMalloc( sz9 );
    void *p10 = pvPortMalloc( sz10 );

    if( p1 != NULL && p2 != NULL && p3 != NULL && p4 != NULL && p5 != NULL &&
        p6 != NULL && p7 != NULL && p8 != NULL && p9 != NULL && p10 != NULL )
    {
        __ESBMC_assert(
            xPortGetFreeHeapSize() ==
                HEAP_ADJUSTED_SIZE - sz1 - sz2 - sz3 - sz4 - sz5
                                   - sz6 - sz7 - sz8 - sz9 - sz10,
            "k10: free heap accounting correct" );
    }
}
