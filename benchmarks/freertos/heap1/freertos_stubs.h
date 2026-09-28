/*
 * freertos_stubs.h
 *
 * Minimal FreeRTOS stub definitions for standalone ESBMC verification.
 * Replaces FreeRTOS.h and task.h so the heap files compile without the
 * full RTOS source tree.
 *
 * Design choices:
 *   - portBYTE_ALIGNMENT = 1  (disables the pointer-alignment code path,
 *     whose cast arithmetic is outside ESBMC's current pointer model)
 *   - vTaskSuspendAll / xTaskResumeAll are no-ops (sequential verification)
 *   - configASSERT is a no-op (vPortFree's assert does not abort)
 *   - Heap size is kept small (256 bytes) for bounded verification
 */

#ifndef FREERTOS_STUBS_H
#define FREERTOS_STUBS_H

#include <stdint.h>
#include <stddef.h>

/* ---- Heap configuration ---- */
#define configTOTAL_HEAP_SIZE              256U
#define configSUPPORT_DYNAMIC_ALLOCATION   1
#define configUSE_MALLOC_FAILED_HOOK       0
#define configAPPLICATION_ALLOCATED_HEAP   0
#define configHEAP_CLEAR_MEMORY_ON_FREE    0

/*
 * Byte alignment = 1 avoids the portPOINTER_SIZE_TYPE cast path in heap_1.c
 * (lines 85-104).  With alignment=1, portBYTE_ALIGNMENT_MASK=0, so the
 * condition (xWantedSize & portBYTE_ALIGNMENT_MASK) is always 0 and no
 * rounding is performed.
 */
#define portBYTE_ALIGNMENT                 1
#define portBYTE_ALIGNMENT_MASK            0

/* Large enough to hold a pointer; used only in the alignment cast path,
 * which is disabled when portBYTE_ALIGNMENT == 1. */
typedef uintptr_t portPOINTER_SIZE_TYPE;

/* ---- Scheduler stubs (no-ops for sequential verification) ---- */
#define vTaskSuspendAll()   do {} while (0)
static inline int xTaskResumeAll(void) { return 1; }

/* ---- Trace and assert stubs ---- */
#define traceMALLOC(p, s)   do {} while (0)
#define traceFREE(p, s)     do {} while (0)
#define configASSERT(x)     do {} while (0)

/* ---- Derived constant ---- */
/* configADJUSTED_HEAP_SIZE is defined inside heap_1.c; replicated here
 * for use in contracts and the harness. */
#define HEAP_ADJUSTED_SIZE  (configTOTAL_HEAP_SIZE - portBYTE_ALIGNMENT)

#endif /* FREERTOS_STUBS_H */
