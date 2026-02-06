#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <stdint.h>

#define configUSE_PREEMPTION                    1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      ( ( unsigned long ) 125000000 )
#define configTICK_RATE_HZ                      ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                ( ( uint16_t ) 256 )
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 64 * 1024 ) )
#define configMAX_TASK_NAME_LEN                 16
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configSUPPORT_STATIC_ALLOCATION         0

#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_EVENT_GROUPS                  1
#define configUSE_TIME_SLICING                  1
#define configUSE_NEWLIB_REENTRANT              1

#define INCLUDE_xEventGroupSetBits              1
#define INCLUDE_xEventGroupSetBitsFromISR       1
#define INCLUDE_xEventGroupWaitBits             1
#define INCLUDE_xEventGroupClearBits            1
#define INCLUDE_xEventGroupGetBits              1
#define INCLUDE_xEventGroupSync                 1

#define INCLUDE_xTimerPendFunctionCall          1

#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               2
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            1024

#define configKERNEL_INTERRUPT_PRIORITY         0xff
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    0x1f
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY 15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configCHECK_FOR_STACK_OVERFLOW          0
#define configUSE_MALLOC_FAILED_HOOK            0
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0

#define configQUEUE_REGISTRY_SIZE               0

#define configUSE_STATS_FORMATTING_FUNCTIONS    0

#define INCLUDE_vTaskDelay                      1
#define INCLUDE_vTaskDelayUntil                 1
#define INCLUDE_vTaskDelete                     1

#endif // FREERTOS_CONFIG_H
