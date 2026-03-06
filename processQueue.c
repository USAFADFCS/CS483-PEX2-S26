/** processQueue.c
 * ===========================================================
 * Name: <Last Name, First Name>
 * Section: <Section>
 * Project: PEX2 - CPU Scheduling Simulator
 * Purpose: Implements queue operations for the doubly-linked ready
 *          and finished process queues used by the CPU scheduler.
 *          Provides insertion, removal, search, sort, and display
 *          functions operating on Process nodes.
 * =========================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "processQueue.h"

void initQueue(Queue* q) {
    q->head = NULL;
    q->tail = NULL;
}

void qInsert(Queue* q, Process* proc) {
    if (!(q->head)) {   // empty queue — new node becomes both head and tail
        q->head = malloc(sizeof(Node));
        q->tail = q->head;
        q->head->data = proc;
        q->head->next = NULL;
        q->head->prev = NULL;
    } else {            // non-empty — append new node at the tail
        Node* oldTail = q->tail;
        Node* newTail = malloc(sizeof(Node));
        newTail->next = NULL;
        newTail->prev = oldTail;
        newTail->data = proc;
        oldTail->next = newTail;
        q->tail = newTail;
    }
}

// Frees the node wrapper but NOT the Process struct; caller owns the returned pointer
Process* qRemove(Queue* q, int which) {
    // walk forward to the requested position
    Node* curr = q->head;
    while (curr && which > 0) {
        curr = curr->next;
        which--;
    }

    if (!curr) {                                // position is out of bounds or queue is empty
        return NULL;
    } else if (curr == q->head && curr == q->tail) {   // only node in the list
        q->head = NULL;
        q->tail = NULL;
    } else if (curr == q->head) {              // removing the head
        q->head = curr->next;
        q->head->prev = NULL;
    } else if (curr == q->tail) {              // removing the tail
        q->tail = curr->prev;
        q->tail->next = NULL;
    } else {                                   // removing an interior node
        curr->prev->next = curr->next;
        curr->next->prev = curr->prev;
    }

    Process* removed = curr->data;
    free(curr);
    return removed;
}

int qShortestBR(Queue* q) {
    Node* curr = q->head;
    int minBurst = INT_MAX;

    while (curr) {
        if (curr->data->burstRemaining < minBurst) {
            minBurst = curr->data->burstRemaining;
        }
        curr = curr->next;
    }
    return minBurst;
}

int qGetPriority(Queue* q) {
    Node* curr = q->head;
    int minPriority = INT_MAX;

    while (curr) {
        if (curr->data->priority < minPriority) {
            minPriority = curr->data->priority;
        }
        curr = curr->next;
    }
    return minPriority;
}

int qShortest(Queue* q) {
    Node* curr = q->head;
    int position = 0;
    int minBurst = INT_MAX;
    int selection = 0;

    while (curr) {
        if (curr->data->burstRemaining < minBurst) {
            minBurst = curr->data->burstRemaining;
            selection = position;
        }
        curr = curr->next;
        position++;
    }
    return selection;
}

int qPriority(Queue* q) {
    Node* curr = q->head;
    int position = 0;
    int minPriority = INT_MAX;
    int selection = 0;

    while (curr) {
        if (curr->data->priority < minPriority) {
            minPriority = curr->data->priority;
            selection = position;
        }
        curr = curr->next;
        position++;
    }
    return selection;
}

void qPrint(Queue q) {
    printf("\t%s: %s, %s, %s, %s, %s, %s, %s\n",
           "PID", "arrivalTime", "priority", "burstTotal",
           "burstRemaining", "initialWait", "totalWait", "requeued");
    Node* curr = q.head;
    while (curr) {
        processPrint(*(curr->data));
        curr = curr->next;
    }
}

void incrementWaitTimes(Queue* q) {
    Node* curr = q->head;
    while (curr) {
        curr->data->totalWait++;
        if (!(curr->data->requeued)) {
            curr->data->initialWait++;
        }
        curr = curr->next;
    }
}

void processPrint(Process proc) {
    printf("\t%d: %d, %d, %d, %d, %d, %d, ",
           proc.PID, proc.arrivalTime, proc.priority, proc.burstTotal,
           proc.burstRemaining, proc.initialWait, proc.totalWait);
    if (proc.requeued) {
        printf("TRUE\n");
    } else {
        printf("FALSE\n");
    }
}

// Sorts the queue in ascending PID order using recursive merge sort.
// The split step uses two inward-stepping pointers to find the midpoint.
void qSort(Queue* q) {
    // base case: zero or one element requires no sorting
    if (q->head == q->tail) {
        if (q->head) {
            q->head->next = NULL;
            q->head->prev = NULL;
        }
        return;
    }

    // split: step pointers inward from both ends to locate the midpoint
    Queue left;
    Queue right;
    left.head  = q->head;
    left.tail  = q->head;
    right.head = q->tail;
    right.tail = q->tail;

    while (left.tail != right.head && left.tail != right.head->next) {
        left.tail  = left.tail->next;
        right.head = right.head->prev;
    }

    // if they met at the same node, advance right so the halves don't overlap
    if (left.tail == right.head) {
        right.head = right.head->next;
    } else {
        // they crossed — swap so left ends just before right begins
        Node* tmp  = left.tail;
        left.tail  = right.head;
        right.head = tmp;
    }

    // recursively sort each half
    qSort(&left);
    qSort(&right);

    // merge the two sorted halves back into q in ascending PID order
    q->head = NULL;
    q->tail = NULL;
    Node* leftCurr  = left.head;
    Node* rightCurr = right.head;

    // choose the first node for the merged result
    if (leftCurr->data->PID < rightCurr->data->PID) {
        q->head = leftCurr;
        q->tail = leftCurr;
        leftCurr = leftCurr->next;
    } else {
        q->head = rightCurr;
        q->tail = rightCurr;
        rightCurr = rightCurr->next;
    }
    q->head->prev = NULL;
    q->head->next = NULL;

    // append the node with the smaller PID at each step
    while (leftCurr && rightCurr) {
        if (leftCurr->data->PID < rightCurr->data->PID) {
            q->tail->next   = leftCurr;
            leftCurr->prev  = q->tail;
            q->tail         = leftCurr;
            leftCurr        = leftCurr->next;
        } else {
            q->tail->next   = rightCurr;
            rightCurr->prev = q->tail;
            q->tail         = rightCurr;
            rightCurr       = rightCurr->next;
        }
    }

    // append any remaining nodes from whichever half is not yet exhausted
    if (leftCurr) {
        q->tail->next  = leftCurr;
        leftCurr->prev = q->tail;
        q->tail        = left.tail;
    } else {
        q->tail->next   = rightCurr;
        rightCurr->prev = q->tail;
        q->tail         = right.tail;
    }
}
