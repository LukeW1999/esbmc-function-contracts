/*
 * heap1_plain.c
 *
 * Contract-free copy of FreeRTOS heap_1 for use in direct (inlined)
 * harness verification.  No __ESBMC_return_value or __ESBMC_ensures
 * clauses; ESBMC verifies the harness assertions by fully inlining
 * this implementation.
 *
 * Based on FreeRTOS-Kernel (MIT licence).
 */

#include "freertos_stubs.h"

#define heapSIZE_MAX                  ( ~( ( size_t ) 0 ) )
#define heapADD_WILL_OVERFLOW( a, b ) ( ( a ) > ( heapSIZE_MAX - ( b ) ) )
#define configADJUSTED_HEAP_SIZE      ( configTOTAL_HEAP_SIZE - portBYTE_ALIGNMENT )

static uint8_t ucHeap[ configTOTAL_HEAP_SIZE ];
static size_t  xNextFreeByte = ( size_t ) 0U;

/* pucAlignedHeap at file scope (was function-local static in original). */
static uint8_t * pucAlignedHeap = NULL;

void * pvPortMalloc( size_t xWantedSize )
{
    void * pvReturn = NULL;

    vTaskSuspendAll();
    {
        if( pucAlignedHeap == NULL )
        {
            /* portBYTE_ALIGNMENT == 1 so this simplifies to &ucHeap[0]. */
            pucAlignedHeap = ( uint8_t * ) ( ( ( portPOINTER_SIZE_TYPE )
                &( ucHeap[ portBYTE_ALIGNMENT - 1 ] ) ) &
                ( ~( ( portPOINTER_SIZE_TYPE ) portBYTE_ALIGNMENT_MASK ) ) );
        }

        if( ( xWantedSize > 0 ) &&
            ( heapADD_WILL_OVERFLOW( xNextFreeByte, xWantedSize ) == 0 ) &&
            ( ( xNextFreeByte + xWantedSize ) < configADJUSTED_HEAP_SIZE ) )
        {
            pvReturn       = pucAlignedHeap + xNextFreeByte;
            xNextFreeByte += xWantedSize;
        }

        traceMALLOC( pvReturn, xWantedSize );
    }
    ( void ) xTaskResumeAll();

    return pvReturn;
}

void vPortFree( void * pv )
{
    ( void ) pv;
    configASSERT( pv == NULL );
}

void vPortInitialiseBlocks( void )
{
    xNextFreeByte = ( size_t ) 0;
    pucAlignedHeap = NULL;   /* reset so next alloc re-initialises it */
}

size_t xPortGetFreeHeapSize( void )
{
    return ( configADJUSTED_HEAP_SIZE - xNextFreeByte );
}
