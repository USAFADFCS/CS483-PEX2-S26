/** CPUs.h
 * ===========================================================
 * Name: <Last Name, First Name>
 * Section: <Section>
 * Project: PEX2 - CPU Scheduling Simulator
 * Purpose: Declares the SharedVars and CpuParams structures that are
 *          shared between the main clock thread and all CPU scheduling
 *          threads, and provides prototypes for the six scheduling
 *          thread functions that students implement in CPUs.c.
 * =========================================================== */

#ifndef CPUS_H
#define CPUS_H

#include <pthread.h>
#include <semaphore.h>
#include "processQueue.h"

// All data shared between the main (clock) thread and every CPU scheduling thread
typedef struct {
    pthread_mutex_t readyQLock;    // protects readyQ from concurrent access
    pthread_mutex_t finishedQLock; // protects finishedQ from concurrent access
    sem_t** cpuSems;               // per-CPU semaphores: main posts, CPU waits (indexed by thread number)
    sem_t* mainSem;                // main waits on this after signaling all CPUs each timestep
    Queue readyQ;                  // processes waiting to be scheduled
    Queue finishedQ;               // processes that have completed execution
    int quantum;                   // round-robin time-slice length in timesteps (used by RRcpu)
} SharedVars;

// Parameters passed to each CPU scheduling thread at creation time
typedef struct {
    int threadNumber;    // index of this CPU; used to select the correct cpuSems entry
    SharedVars* svars;   // pointer to the data structure shared with all other threads
} CpuParams;

// ---- CPU scheduling thread functions (students implement these in CPUs.c) ----

/**
 * @brief  First-In First-Out CPU scheduling thread. Non-preemptive: runs each
 *         process to completion before selecting the next. Always picks the
 *         process at the head of the ready queue (earliest arrival).
 * @param param Pointer to a CpuParams struct for this thread.
 * @return NULL (required by the pthread thread-function signature).
 */
void* FIFOcpu(void* param);

/**
 * @brief  Shortest-Job-First CPU scheduling thread. Non-preemptive: runs each
 *         process to completion. Selects the process with the smallest burstTotal
 *         from the ready queue.
 * @param param Pointer to a CpuParams struct for this thread.
 * @return NULL (required by the pthread thread-function signature).
 */
void* SJFcpu(void* param);

/**
 * @brief  Non-Preemptive Priority CPU scheduling thread. Runs each process to
 *         completion. Selects the process with the highest priority (lowest-numbered
 *         priority value) from the ready queue.
 * @param param Pointer to a CpuParams struct for this thread.
 * @return NULL (required by the pthread thread-function signature).
 */
void* NPPcpu(void* param);

/**
 * @brief  Round-Robin CPU scheduling thread. Preempts the running process after
 *         SharedVars.quantum timesteps and requeues it; always selects from the
 *         head of the ready queue.
 * @param param Pointer to a CpuParams struct for this thread.
 * @return NULL (required by the pthread thread-function signature).
 */
void* RRcpu(void* param);

/**
 * @brief  Shortest Remaining Time First CPU scheduling thread. Preempts the running
 *         process whenever a process with a smaller burstRemaining value is in the
 *         ready queue; selects the shortest-remaining job each timestep.
 * @param param Pointer to a CpuParams struct for this thread.
 * @return NULL (required by the pthread thread-function signature).
 */
void* SRTFcpu(void* param);

/**
 * @brief  Preemptive Priority CPU scheduling thread. Preempts the running process
 *         when a higher-priority (lower-numbered) process is in the ready queue.
 *         Strict greater-than comparison means equal priority keeps the running
 *         process to avoid unnecessary context switches.
 * @param param Pointer to a CpuParams struct for this thread.
 * @return NULL (required by the pthread thread-function signature).
 */
void* PPcpu(void* param);

#endif // CPUS_H
