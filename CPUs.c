/** CPUs.c
 * ===========================================================
 * Name: <Last Name, First Name>
 * Section: <Section>
 * Project: PEX2 - CPU Scheduling Simulator
 * Purpose: Implements six CPU scheduling algorithms as POSIX threads.
 *          Each thread follows the same pattern every timestep:
 *            1. Wait on its per-CPU semaphore (posted by main).
 *            2. Optionally preempt the current process back to readyQ.
 *            3. Select a new process from readyQ if idle.
 *            4. Decrement burstRemaining by one (one unit of execution).
 *            5. Move the process to finishedQ if it is complete.
 *            6. Post to mainSem to signal the clock thread.
 *          All accesses to readyQ and finishedQ are protected by their
 *          respective mutex locks.
 * ===========================================================
 * Documentation Statement: <describe any help received>
 * =========================================================== */

#include <stdio.h>
#include "CPUs.h"
#include "processQueue.h"

// ============================================================
// FIFO — First In First Out (non-preemptive)
// Runs each process to completion; always selects the head of
// the ready queue (the process that arrived earliest).
// ============================================================
void* FIFOcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    Process* p = NULL;  // currently running process; NULL when this CPU is idle

    // thread loops for the entire lifetime of the simulation
    while (1) {
        sem_wait(svars->cpuSems[threadNum]);    // block until main signals this timestep

        if (p == NULL) {                        // idle — select the next process
            pthread_mutex_lock(&(svars->readyQLock));
            p = qRemove(&(svars->readyQ), 0);   // FIFO: always take from the head
            if (p == NULL) {
                printf("No process to schedule\n");
            } else {
                printf("Scheduling PID %d\n", p->PID);
            }
            pthread_mutex_unlock(&(svars->readyQLock));
        }

        if (p != NULL) {                        // execute one timestep of burst
            p->burstRemaining--;
            if (p->burstRemaining == 0) {       // process finished — move to finishedQ
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));
                p = NULL;
            }
        }

        sem_post(svars->mainSem);               // notify main that this CPU is done
    }
}

// ============================================================
// SJF — Shortest Job First (non-preemptive)
// Runs each process to completion; selects the process with the
// smallest burstRemaining (equals burstTotal for unscheduled processes).
// ============================================================
void* SJFcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    Process* p = NULL;

    while (1) {
        sem_wait(svars->cpuSems[threadNum]);

        if (p == NULL) {
            pthread_mutex_lock(&(svars->readyQLock));

            // TODO: Select the process with the shortest remaining burst (burstRemaining).
            //       See processQueue.h for the available queue helpers.
            //       Print "Scheduling PID X\n" or "No process to schedule\n".

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        if (p != NULL) {
            p->burstRemaining--;
            if (p->burstRemaining == 0) {
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));
                p = NULL;
            }
        }

        sem_post(svars->mainSem);
    }
}

// ============================================================
// NPP — Non-Preemptive Priority
// Runs each process to completion; selects the process with the
// highest priority (lowest-numbered priority value) from the queue.
// Remember: lower priority number = higher priority.
// ============================================================
void* NPPcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    Process* p = NULL;

    while (1) {
        sem_wait(svars->cpuSems[threadNum]);

        if (p == NULL) {
            pthread_mutex_lock(&(svars->readyQLock));

            // TODO: Select the process with the highest priority (lowest number).
            //       See processQueue.h for the available queue helpers.
            //       Print "Scheduling PID X\n" or "No process to schedule\n".

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        if (p != NULL) {
            p->burstRemaining--;
            if (p->burstRemaining == 0) {
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));
                p = NULL;
            }
        }

        sem_post(svars->mainSem);
    }
}

// ============================================================
// RR — Round Robin (quantum-based preemption)
// Runs a process for at most 'quantum' timesteps before requeuing
// it; always selects from the head of the ready queue.
// ============================================================
void* RRcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    Process* p = NULL;
    // TODO: declare an integer counter (e.g. stepsWorked) here, outside the loop,
    //       to track how many timesteps the current process has run this quantum.

    while (1) {
        sem_wait(svars->cpuSems[threadNum]);

        // TODO: Before selecting, check whether the current process has used its
        //       full quantum (compare your counter to svars->quantum). If so,
        //       put it back in readyQ (set requeued=true first), reset the counter,
        //       and set p = NULL so the selection block picks a new process.
        //       Hold readyQLock while modifying the queue.

        if (p == NULL) {
            pthread_mutex_lock(&(svars->readyQLock));

            // TODO: Select from the head of the ready queue.
            //       Print "Scheduling PID X\n" or "No process to schedule\n".

            pthread_mutex_unlock(&(svars->readyQLock));
        }

        if (p != NULL) {
            p->burstRemaining--;
            if (p->burstRemaining == 0) {
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));
                p = NULL;
                // TODO: reset your step counter here (process finished early)
            } else {
                // TODO: increment your step counter here
            }
        }

        sem_post(svars->mainSem);
    }
}

// ============================================================
// SRTF — Shortest Remaining Time First (preemptive)
// Preempts the running process whenever a shorter job is in the
// ready queue; selects the process with the smallest burstRemaining.
//
// Note: hold readyQLock across both the preemption check AND the
//       new selection — this prevents another CPU thread from
//       modifying the queue in between those two steps.
// ============================================================
void* SRTFcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    Process* p = NULL;

    while (1) {
        sem_wait(svars->cpuSems[threadNum]);

        pthread_mutex_lock(&(svars->readyQLock));

        // TODO: Check whether a shorter job is waiting in readyQ.
        //       If so, preempt the running process (mark it requeued,
        //       put it back in readyQ, set p = NULL).
        //
        // TODO: If p is NULL (idle or just preempted), select the process
        //       with the shortest remaining burst.
        //       Print "Scheduling PID X\n" or "No process to schedule\n".

        pthread_mutex_unlock(&(svars->readyQLock));

        if (p != NULL) {
            p->burstRemaining--;
            if (p->burstRemaining == 0) {
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));
                p = NULL;
            }
        }

        sem_post(svars->mainSem);
    }
}

// ============================================================
// PP — Preemptive Priority
// Preempts the running process when a higher-priority (lower-
// numbered) process is in the ready queue. Use strict '>' so that
// equal priority keeps the running process (avoids unnecessary
// context switches).
//
// Note: hold readyQLock across both the preemption check AND the
//       new selection (same reason as SRTF).
// ============================================================
void* PPcpu(void* param) {
    int threadNum = ((CpuParams*) param)->threadNumber;
    SharedVars* svars = ((CpuParams*) param)->svars;

    Process* p = NULL;

    while (1) {
        sem_wait(svars->cpuSems[threadNum]);

        pthread_mutex_lock(&(svars->readyQLock));

        // TODO: Check whether a strictly higher-priority job is waiting in readyQ.
        //       If so, preempt the running process (mark it requeued,
        //       put it back in readyQ, set p = NULL).
        //
        // TODO: If p is NULL (idle or just preempted), select the process
        //       with the highest priority (lowest number).
        //       Print "Scheduling PID X\n" or "No process to schedule\n".

        pthread_mutex_unlock(&(svars->readyQLock));

        if (p != NULL) {
            p->burstRemaining--;
            if (p->burstRemaining == 0) {
                pthread_mutex_lock(&(svars->finishedQLock));
                qInsert(&(svars->finishedQ), p);
                pthread_mutex_unlock(&(svars->finishedQLock));
                p = NULL;
            }
        }

        sem_post(svars->mainSem);
    }
}
