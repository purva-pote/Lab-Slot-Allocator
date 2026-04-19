#ifndef SCHEDULER_H
#define SCHEDULER_H

/* =========================================================
 * scheduler.h  –  Core scheduling algorithms
 *
 * Algorithms used:
 *   1. Merge Sort  – sort labs before scheduling
 *   2. Greedy      – assign labs to earliest valid slot + room
 *   3. Graph Coloring (Backtracking) – resolve conflicts
 *   4. Backtracking resolver – last-resort full search
 * ========================================================= */

#include "lab.h"
#include "room.h"

/* ---------------------------------------------------------- */
/* Time slot                                                   */
/* ---------------------------------------------------------- */
#define MAX_SLOTS    40
#define MAX_DAY_LEN  12
#define MAX_TIME_LEN 10

typedef struct {
    int  slot_id;
    char day[MAX_DAY_LEN];         /* e.g., "Monday" */
    char start_time[MAX_TIME_LEN]; /* e.g., "08:00" */
    char end_time[MAX_TIME_LEN];   /* e.g., "09:00" */
} TimeSlot;

/* ---------------------------------------------------------- */
/* Schedule entry: one successfully allocated lab              */
/* ---------------------------------------------------------- */
typedef struct {
    int lab_id;
    int slot_id;   /* index into TimeSlot array */
    int room_id;   /* index into Room array */
} ScheduleEntry;

/* ---------------------------------------------------------- */
/* Conflict graph (adjacency matrix for backtracking)          */
/* ---------------------------------------------------------- */
/* conflict_graph[i][j] == 1  means lab i and lab j share the
   same batch and therefore CANNOT be in the same time slot.  */
extern int conflict_graph[MAX_LABS][MAX_LABS];

/* ---------------------------------------------------------- */
/* Function prototypes (implemented in scheduler.c)            */
/* ---------------------------------------------------------- */

/* Load time slots from CSV; returns count */
int load_time_slots(const char *filepath, TimeSlot *slots, int max_count);

/* ── Sorting ─────────────────────────────────────────────── */

/* Merge Sort: sort labs[] in ascending order of student_count
   (larger labs get priority – they are sorted descending).   */
void merge_sort_labs(Lab *labs, int left, int right);

/* Quick Sort variant: sort labs by batch_id then student_count */
void quick_sort_labs(Lab *labs, int low, int high);

/* ── Conflict Graph ───────────────────────────────────────── */

/* Build conflict_graph from the lab array.
   Two labs conflict if they share the same batch_id.         */
void build_conflict_graph(const Lab *labs, int lab_count);

/* ── Graph Coloring (Backtracking) ───────────────────────── */

/* Assign a color (slot index) to each lab such that no two
   conflicting labs share the same color.
   colors[] output array: colors[i] = slot index for lab i.
   Returns 1 on success, 0 if impossible with num_slots.      */
int graph_color_labs(int lab_count, int num_slots, int *colors);

/* ── Greedy Allocation ───────────────────────────────────── */

/* Run greedy allocation:
     - Sort labs (merge sort) by student_count descending.
     - For each lab, pick the earliest slot (from colors[]) and
       a room with capacity >= student_count.
   Fills schedule[]; returns number of entries placed.         */
int greedy_allocate(Lab *labs, int lab_count,
                    Room *rooms, int room_count,
                    TimeSlot *slots, int slot_count,
                    int *colors,
                    ScheduleEntry *schedule);

/* ── Backtracking Resolver ───────────────────────────────── */

/* If greedy_allocate cannot place all labs, this function
   tries every valid (slot, room) combination recursively.
   Returns 1 if a complete assignment is found, 0 otherwise.  */
int backtrack_allocate(Lab *labs, int lab_count,
                       Room *rooms, int room_count,
                       TimeSlot *slots, int slot_count,
                       ScheduleEntry *schedule,
                       int *schedule_count,
                       int lab_index);

/* ── Output ─────────────────────────────────────────────── */

/* Write final schedule to a text file */
void write_schedule(const char *filepath,
                    const ScheduleEntry *schedule, int count,
                    const Lab *labs,   int lab_count,
                    const Room *rooms, int room_count,
                    const TimeSlot *slots, int slot_count,
                    const Batch *batches, int batch_count,
                    const Subject *subjects, int subject_count);

/* Print schedule to stdout as a table */
void print_schedule(const ScheduleEntry *schedule, int count,
                    const Lab *labs,   int lab_count,
                    const Room *rooms, int room_count,
                    const TimeSlot *slots, int slot_count,
                    const Batch *batches, int batch_count,
                    const Subject *subjects, int subject_count);

#endif /* SCHEDULER_H */
