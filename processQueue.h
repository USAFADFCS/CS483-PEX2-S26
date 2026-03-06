/** processQueue.h
 * ===========================================================
 * Name: <Last Name, First Name>
 * Section: <Section>
 * Project: PEX2 - CPU Scheduling Simulator
 * Purpose: Defines the Process, Node, and Queue data structures
 *          used by the scheduling simulation's doubly-linked ready
 *          and finished queues, along with function prototypes for
 *          all queue manipulation operations.
 * =========================================================== */

#ifndef PROCESS_QUEUE_H
#define PROCESS_QUEUE_H

#include <stdbool.h>

// Tracks all scheduling attributes for one simulated process
typedef struct {
    int PID;             // unique process identifier
    int arrivalTime;     // simulation timestep at which the process joined the ready queue
    int priority;        // scheduling priority — lower number means higher priority
    int burstTotal;      // total CPU timesteps required to complete this process
    int burstRemaining;  // CPU timesteps still needed before this process finishes
    int initialWait;     // timesteps in the ready queue before being scheduled for the first time
    int totalWait;       // total timesteps spent waiting in the ready queue
    bool requeued;       // true once the process has been placed back in the ready queue
} Process;

// A single node in the doubly-linked list of processes
typedef struct Node {
    Process* data;       // pointer to the process stored at this node
    struct Node* prev;   // pointer to the previous node in the list
    struct Node* next;   // pointer to the next node in the list
} Node;

// Holds the head and tail pointers of a doubly-linked list of Nodes
typedef struct {
    Node* head;   // first node in the list (oldest entry)
    Node* tail;   // last node in the list (most recently inserted)
} Queue;

// ---- Functions used internally by main — students do not call these directly ----

/**
 * @brief  Initializes an empty queue by setting head and tail to NULL.
 * @param q Pointer to the Queue to initialize.
 */
void initQueue(Queue* q);

/**
 * @brief  Increments totalWait for every process in the queue by one timestep.
 *         Also increments initialWait for any process that has not yet been requeued.
 * @param q Pointer to the ready Queue whose wait-time counters should be updated.
 */
void incrementWaitTimes(Queue* q);

/**
 * @brief  Sorts the queue in ascending PID order using merge sort. Called before
 *         printing the finished queue at the end of the simulation.
 * @param q Pointer to the Queue to sort.
 */
void qSort(Queue* q);

// ---- Selector functions — return the queue position for use with qRemove ----

/**
 * @brief   Returns the 0-based position of the process with the shortest burstRemaining
 *          in the queue. Returns 0 when the queue is empty; callers must check that
 *          qRemove does not return NULL before using the result.
 *          Note: for unscheduled processes burstRemaining == burstTotal, so this
 *          function correctly selects the shortest job for SJF as well as SRTF.
 * @param q Pointer to the Queue to search.
 * @return  0-based index of the shortest-burst process, or 0 if the queue is empty.
 */
int qShortest(Queue* q);

/**
 * @brief   Returns the 0-based position of the process with the highest priority
 *          (lowest-numbered priority value) in the queue. Returns 0 when empty;
 *          callers must check that qRemove does not return NULL.
 * @param q Pointer to the Queue to search.
 * @return  0-based index of the highest-priority process, or 0 if the queue is empty.
 */
int qPriority(Queue* q);

// ---- Value helpers — return scalar values for preemption comparisons ----

/**
 * @brief   Returns the smallest burstRemaining value among all processes in the queue.
 *          Returns INT_MAX when the queue is empty, ensuring no spurious preemption.
 * @param q Pointer to the Queue to search.
 * @return  Minimum burstRemaining in the queue, or INT_MAX if empty.
 */
int qShortestBR(Queue* q);

/**
 * @brief   Returns the highest priority (lowest-numbered priority level) among all
 *          processes in the queue. Returns INT_MAX when the queue is empty, ensuring
 *          no spurious preemption.
 * @param q Pointer to the Queue to search.
 * @return  Minimum priority value in the queue, or INT_MAX if empty.
 */
int qGetPriority(Queue* q);

// ---- Debug / display helpers ----

/**
 * @brief  Prints all processes in the queue to stdout in tabular form, one per line.
 * @param q The Queue to print (passed by value — read only).
 */
void qPrint(Queue q);

/**
 * @brief  Prints all fields of a single process to stdout.
 * @param proc The Process to print (passed by value — read only).
 */
void processPrint(Process proc);

// ---- Core queue operations ----

/**
 * @brief   Appends a process to the tail of the queue.
 * @param q    Pointer to the Queue to insert into.
 * @param proc Pointer to the Process to append.
 */
void qInsert(Queue* q, Process* proc);

/**
 * @brief   Removes and returns the process at the 0-based position 'which' in the
 *          queue, freeing the node wrapper. Returns NULL if the position is out of
 *          bounds or the queue is empty. The caller takes ownership of the returned
 *          Process pointer and is responsible for eventually freeing it.
 * @param q     Pointer to the Queue to remove from.
 * @param which 0-based index of the node to remove.
 * @return Pointer to the removed Process, or NULL if not found.
 */
Process* qRemove(Queue* q, int which);

#endif // PROCESS_QUEUE_H
