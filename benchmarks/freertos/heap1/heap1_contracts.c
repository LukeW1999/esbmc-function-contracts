/*
 * heap1_contracts.c
 *
 * Self-contained copy of FreeRTOS heap_1.c with ESBMC contracts added.
 * Based on FreeRTOS-Kernel (MIT licence).
 *
 * Changes from the original:
 *   1. Replaced #include "FreeRTOS.h" / "task.h" with freertos_stubs.h
 *   2. Added __ESBMC_requires / __ESBMC_assigns / __ESBMC_ensures contracts
 *   3. Moved pucAlignedHeap from function-local static to file-level static
 *      so that ESBMC's assigns tracking covers it
 *   4. All comments in English
 */

#include "freertos_stubs.h"

/* ---- Overflow-check macros (unchanged from original) ---- */
#define heapSIZE_MAX                  ( ~( ( size_t ) 0 ) )
#define heapADD_WILL_OVERFLOW( a, b ) ( ( a ) > ( heapSIZE_MAX - ( b ) ) )
#define configADJUSTED_HEAP_SIZE      ( configTOTAL_HEAP_SIZE - portBYTE_ALIGNMENT )

/* ---- Heap storage ---- */
static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];

/* Byte index of the next free location in ucHeap. */
static size_t xNextFreeByte = ( size_t ) 0U;

/*
 * Pointer to the first aligned byte of ucHeap.
 * Moved to file scope (was function-local static) so ESBMC's assigns
 * clause can track it.
 */
static uint8_t * pucAlignedHeap = NULL;

/* ------------------------------------------------------------------ */
/*  pvPortMalloc                                                        */
/* ------------------------------------------------------------------ */
void * pvPortMalloc( size_t xWantedSize )
{
    /*
     * Heap state invariant (preconditions on global state):
     *   - xNextFreeByte must already be within the heap; ESBMC's enforce
     *     mode makes globals nondeterministic, so we must state this
     *     invariant explicitly as a requires clause.
     *   - pucAlignedHeap is either uninitialised (NULL) or points to ucHeap.
     */
    __ESBMC_requires( xNextFreeByte <= configADJUSTED_HEAP_SIZE );
    __ESBMC_requires( pucAlignedHeap == NULL ||
                      pucAlignedHeap == ( uint8_t * ) ucHeap );

    /*
     * Contract:
     *   assigns  – only xNextFreeByte and pucAlignedHeap may change
     *   ensures  – heap boundary is never exceeded (invariant preserved)
     *   ensures  – NULL return leaves xNextFreeByte unchanged
     *   ensures  – non-NULL return means xNextFreeByte advanced
     *   ensures  – xNextFreeByte never decreases (monotonic bump allocator)
     *
     * Note: use != 0 / == 0 instead of != NULL / == NULL to avoid the
     * pointer-vs-integer type warning on __ESBMC_return_value.
     */
    __ESBMC_assigns( xNextFreeByte, pucAlignedHeap );
    /*
     * Postconditions:
     *   1. Heap boundary invariant: xNextFreeByte never exceeds usable size
     *   2. Monotonicity: bump allocator never moves the pointer backwards
     *   3. Conditional: non-NULL return implies xNextFreeByte advanced
     *      (__ESBMC_return_value is substituted with the actual return value
     *       in both enforce mode and replace mode by the contracts engine)
     */
    __ESBMC_ensures( xNextFreeByte <= configADJUSTED_HEAP_SIZE );
    __ESBMC_ensures( xNextFreeByte >= __ESBMC_old( xNextFreeByte ) );
    /* Exact semantics of the bump allocator:
     *   success: xNextFreeByte advances by exactly xWantedSize (strict bound)
     *   failure: xNextFreeByte unchanged, return value is NULL */
    __ESBMC_ensures( ((size_t) __ESBMC_return_value == 0) ||
                     (xNextFreeByte == __ESBMC_old( xNextFreeByte ) + xWantedSize) );
    __ESBMC_ensures( ((size_t) __ESBMC_return_value != 0) ||
                     (xNextFreeByte == __ESBMC_old( xNextFreeByte )) );
    __ESBMC_ensures( ((size_t) __ESBMC_return_value == 0) ||
                     (xNextFreeByte < configADJUSTED_HEAP_SIZE) );
    /* Return value is the address of the allocated block in the heap. */
    __ESBMC_ensures( ((size_t) __ESBMC_return_value == 0) ||
                     ((void *) __ESBMC_return_value ==
                      (void *)( ucHeap + __ESBMC_old( xNextFreeByte ) )) );
    /* pucAlignedHeap is initialised on first call and never changes. */
    __ESBMC_ensures( pucAlignedHeap == ( uint8_t * ) ucHeap );

    void * pvReturn = NULL;

    /* Alignment path disabled (portBYTE_ALIGNMENT == 1). */

    vTaskSuspendAll();
    {
        /* On first call: initialise the aligned heap pointer. */
        if( pucAlignedHeap == NULL )
        {
            /* With portBYTE_ALIGNMENT==1 the cast simplifies to &ucHeap[0]. */
            pucAlignedHeap = ( uint8_t * ) ( ( ( portPOINTER_SIZE_TYPE )
                &( ucHeap[ portBYTE_ALIGNMENT - 1 ] ) ) &
                ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) );
        }

        /* Allocate only if there is sufficient room without overflow. */
        if( ( xWantedSize > 0 ) &&
            ( heapADD_WILL_OVERFLOW( xNextFreeByte, xWantedSize ) == 0 ) &&
            ( ( xNextFreeByte + xWantedSize ) < configADJUSTED_HEAP_SIZE ) )
        {
            pvReturn = pucAlignedHeap + xNextFreeByte;
            xNextFreeByte += xWantedSize;
        }

        traceMALLOC( pvReturn, xWantedSize );
    }
    ( void ) xTaskResumeAll();

    return pvReturn;
}

/* ------------------------------------------------------------------ */
/*  vPortFree                                                           */
/* ------------------------------------------------------------------ */
void vPortFree( void * pv )
{
    /*
     * heap_1 does not support freeing memory.
     * Contract: assigns nothing, triggers configASSERT if pv != NULL.
     */
    ( void ) pv;
    configASSERT( pv == NULL );
}

/* ------------------------------------------------------------------ */
/*  vPortInitialiseBlocks                                               */
/* ------------------------------------------------------------------ */
void vPortInitialiseBlocks( void )
{
    /* Reset the allocator state (used before restarting the scheduler). */
    __ESBMC_assigns( xNextFreeByte );
    __ESBMC_ensures( xNextFreeByte == 0 );

    xNextFreeByte = ( size_t ) 0;
}

/* ------------------------------------------------------------------ */
/*  xPortGetFreeHeapSize                                                */
/* ------------------------------------------------------------------ */
size_t xPortGetFreeHeapSize( void )
{
    /*
     * Pure query: assigns nothing, result equals remaining capacity.
     * ESBMC note: no assigns clause means no globals should change;
     * the postcondition pins the return value.
     */
    __ESBMC_ensures( __ESBMC_return_value ==
                     configADJUSTED_HEAP_SIZE - xNextFreeByte );

    return ( configADJUSTED_HEAP_SIZE - xNextFreeByte );
}
