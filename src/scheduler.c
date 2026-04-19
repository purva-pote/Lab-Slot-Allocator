/* =========================================================
 * scheduler.c  –  Core Scheduling Algorithms
 *
 * Algorithms implemented:
 *   1. Merge Sort  – sort labs[] by student_count (desc) before allocation
 *   2. Quick Sort  – alternative sort by batch_id then student_count
 *   3. Build Conflict Graph – edge if two labs share same batch_id
 *   4. Graph Coloring (Backtracking) – color labs with slot indices
 *      so conflicting labs never share a color (time slot)
 *   5. Greedy Allocation – assign rooms using conflict graph colors
 *   6. Backtracking Resolver – exhaustive search if greedy fails
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scheduler.h"
#include "lab.h"
#include "room.h"
#include "utils.h"

/* Global conflict adjacency matrix */
int conflict_graph[MAX_LABS][MAX_LABS];

/* =========================================================
 * Section 1: Load Time Slots from CSV
 * ========================================================= */

int load_time_slots(const char *filepath, TimeSlot *slots, int max_count) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "[ERROR] Cannot open file: %s\n", filepath);
        return 0;
    }

    char line[256];
    int  count = 0;

    /* Skip header */
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return 0;
    }

    while (fgets(line, sizeof(line), fp) && count < max_count) {
        char *tokens[6];
        int n = split_csv(line, tokens, 6);
        if (n < 4) continue;

        slots[count].slot_id = atoi(tokens[0]);
        strncpy(slots[count].day,        tokens[1], MAX_DAY_LEN  - 1);
        strncpy(slots[count].start_time, tokens[2], MAX_TIME_LEN - 1);
        strncpy(slots[count].end_time,   tokens[3], MAX_TIME_LEN - 1);
        slots[count].day[MAX_DAY_LEN - 1]        = '\0';
        slots[count].start_time[MAX_TIME_LEN - 1] = '\0';
        slots[count].end_time[MAX_TIME_LEN - 1]   = '\0';
        count++;
    }

    fclose(fp);
    printf("[INFO] Loaded %d time slot(s) from %s\n", count, filepath);
    return count;
}

/* =========================================================
 * Section 2: Merge Sort  (sort by student_count descending)
 *
 * Larger labs are sorted first so they get priority when
 * picking rooms (a classic greedy heuristic).
 * ========================================================= */

/* Merge two sorted sub-arrays labs[left..mid] and labs[mid+1..right] */
static void merge(Lab *labs, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    /* Temporary arrays */
    Lab *L = (Lab *)malloc(n1 * sizeof(Lab));
    Lab *R = (Lab *)malloc(n2 * sizeof(Lab));
    if (!L || !R) {
        fprintf(stderr, "[ERROR] Memory allocation failed in merge()\n");
        free(L); free(R);
        return;
    }

    for (int i = 0; i < n1; i++) L[i] = labs[left + i];
    for (int j = 0; j < n2; j++) R[j] = labs[mid + 1 + j];

    int i = 0, j = 0, k = left;

    /* Merge in DESCENDING order of student_count */
    while (i < n1 && j < n2) {
        if (L[i].student_count >= R[j].student_count)
            labs[k++] = L[i++];
        else
            labs[k++] = R[j++];
    }
    while (i < n1) labs[k++] = L[i++];
    while (j < n2) labs[k++] = R[j++];

    free(L);
    free(R);
}

/* Recursive merge sort entry point */
void merge_sort_labs(Lab *labs, int left, int right) {
    if (left >= right) return; /* base case */
    int mid = left + (right - left) / 2;
    merge_sort_labs(labs, left, mid);
    merge_sort_labs(labs, mid + 1, right);
    merge(labs, left, mid, right);
}

/* =========================================================
 * Section 3: Quick Sort  (sort by batch_id, then student_count)
 *
 * Primary  key: batch_id ascending  (group same-batch labs together)
 * Secondary key: student_count descending (larger classes first)
 * ========================================================= */

static void swap_lab(Lab *a, Lab *b) {
    Lab tmp = *a; *a = *b; *b = tmp;
}

/* Compare function: returns negative if a should come before b */
static int compare_labs(const Lab *a, const Lab *b) {
    if (a->batch_id != b->batch_id)
        return a->batch_id - b->batch_id;       /* ascending batch */
    return b->student_count - a->student_count; /* descending size */
}

/* Lomuto partition scheme */
static int partition(Lab *labs, int low, int high) {
    Lab pivot = labs[high]; /* pivot element */
    int i = low - 1;

    for (int j = low; j < high; j++) {
        if (compare_labs(&labs[j], &pivot) <= 0) {
            i++;
            swap_lab(&labs[i], &labs[j]);
        }
    }
    swap_lab(&labs[i + 1], &labs[high]);
    return i + 1;
}

void quick_sort_labs(Lab *labs, int low, int high) {
    if (low < high) {
        int pi = partition(labs, low, high);
        quick_sort_labs(labs, low, pi - 1);
        quick_sort_labs(labs, pi + 1, high);
    }
}

/* =========================================================
 * Section 4: Build Conflict Graph
 *
 * An edge exists between lab i and lab j when:
 *   labs[i].batch_id == labs[j].batch_id   (same batch)
 * These labs cannot occupy the same time slot.
 * ========================================================= */

void build_conflict_graph(const Lab *labs, int lab_count) {
    /* Zero out the entire matrix first */
    memset(conflict_graph, 0, sizeof(conflict_graph));

    for (int i = 0; i < lab_count; i++) {
        for (int j = i + 1; j < lab_count; j++) {
            if (labs[i].batch_id == labs[j].batch_id) {
                conflict_graph[i][j] = 1;
                conflict_graph[j][i] = 1; /* undirected graph */
            }
        }
    }

    printf("[INFO] Conflict graph built for %d lab(s).\n", lab_count);
}

/* =========================================================
 * Section 5: Graph Coloring via Backtracking
 *
 * Assign a "color" (= time-slot index) to each lab vertex
 * such that no two adjacent (conflicting) labs share the
 * same color.
 *
 * This is a classic NP-hard problem; for small inputs
 * (dozens of labs) the backtracking is fast enough.
 * ========================================================= */

/* Check if assigning color `c` to lab `lab_idx` is safe */
static int is_safe_color(int lab_idx, int *colors,
                          int lab_count, int c) {
    for (int j = 0; j < lab_count; j++) {
        /* If j is adjacent (conflict) AND has same color -> not safe */
        if (conflict_graph[lab_idx][j] && colors[j] == c) {
            return 0;
        }
    }
    return 1;
}

/* Recursive helper */
static int color_util(int lab_idx, int lab_count,
                       int num_slots, int *colors) {
    /* Base case: all labs have been colored */
    if (lab_idx == lab_count) return 1;

    /* Try each slot (color) for this lab */
    for (int c = 0; c < num_slots; c++) {
        if (is_safe_color(lab_idx, colors, lab_count, c)) {
            colors[lab_idx] = c; /* tentative assignment */

            /* Recurse for the next lab */
            if (color_util(lab_idx + 1, lab_count, num_slots, colors))
                return 1; /* solution found */

            colors[lab_idx] = -1; /* backtrack */
        }
    }
    return 0; /* no valid coloring with num_slots colors */
}

int graph_color_labs(int lab_count, int num_slots, int *colors) {
    /* Initialize all colors to -1 (unassigned) */
    for (int i = 0; i < lab_count; i++) colors[i] = -1;

    if (!color_util(0, lab_count, num_slots, colors)) {
        fprintf(stderr,
            "[WARN] Graph coloring failed with %d slot(s). "
            "Try adding more time slots.\n", num_slots);
        return 0;
    }

    printf("[INFO] Graph coloring succeeded.\n");
    return 1;
}

/* =========================================================
 * Section 6: Greedy Allocation
 *
 * Uses the colors[] from graph coloring as the "target slot"
 * for each lab. Then finds a room in that slot that:
 *   - Has capacity >= lab.student_count
 *   - Is not already occupied in that slot by another lab
 * ========================================================= */

int greedy_allocate(Lab *labs, int lab_count,
                    Room *rooms, int room_count,
                    TimeSlot *slots, int slot_count,
                    int *colors,
                    ScheduleEntry *schedule) {

    /* slot_room_used[s][r] = 1 if room r is used in slot s */
    int slot_room_used[MAX_SLOTS][MAX_ROOMS];
    memset(slot_room_used, 0, sizeof(slot_room_used));

    int placed = 0;

    /* Sort labs by student_count descending before greedy loop */
    merge_sort_labs(labs, 0, lab_count - 1);

    for (int i = 0; i < lab_count; i++) {
        int target_slot = colors[i]; /* assigned time-slot color */
        if (target_slot < 0 || target_slot >= slot_count) {
            fprintf(stderr,
                "[WARN] Lab %d has invalid color %d; skipping.\n",
                labs[i].id, target_slot);
            continue;
        }

        /* Find a free room with enough capacity in target_slot */
        int found_room = -1;
        for (int r = 0; r < room_count; r++) {
            if (!slot_room_used[target_slot][r] &&
                rooms[r].capacity >= labs[i].student_count) {
                found_room = r;
                break; /* take the first suitable room */
            }
        }

        if (found_room == -1) {
            fprintf(stderr,
                "[WARN] No suitable room for Lab %d in slot %d.\n",
                labs[i].id, target_slot);
            continue;
        }

        /* Record allocation */
        slot_room_used[target_slot][found_room] = 1;
        labs[i].is_allocated      = 1;
        labs[i].assigned_slot_id  = target_slot;
        labs[i].assigned_room_id  = found_room;

        schedule[placed].lab_id  = i;        /* index in labs[] */
        schedule[placed].slot_id = target_slot;
        schedule[placed].room_id = found_room;
        placed++;
    }

    printf("[INFO] Greedy allocation placed %d / %d lab(s).\n",
           placed, lab_count);
    return placed;
}

/* =========================================================
 * Section 7: Backtracking Resolver
 *
 * If greedy allocation cannot place all labs, this function
 * performs a recursive exhaustive search.
 *
 * State: for each lab (in order), try every (slot, room) pair.
 * Constraints checked:
 *   (a) No conflicting lab (same batch) in the same slot.
 *   (b) Room capacity >= student_count.
 *   (c) Room not already used in that slot.
 * ========================================================= */

/* slot_room_used[][]: tracks room occupancy per slot.
   Declared static so the recursive function can share it. */
static int bt_slot_room_used[MAX_SLOTS][MAX_ROOMS];

/* Check if placing lab `lab_idx` in (slot s, room r) is valid */
static int bt_is_valid(const Lab *labs, int lab_count,
                       ScheduleEntry *schedule, int schedule_count,
                       int lab_idx, int s, int r, int room_capacity) {

    /* Capacity check */
    if (room_capacity < labs[lab_idx].student_count) return 0;

    /* Room already used in this slot */
    if (bt_slot_room_used[s][r]) return 0;

    /* Conflict check: no same-batch lab in the same slot */
    for (int k = 0; k < schedule_count; k++) {
        int prev_lab  = schedule[k].lab_id;
        int prev_slot = schedule[k].slot_id;
        if (prev_slot == s &&
            conflict_graph[lab_idx][prev_lab]) {
            return 0; /* same batch, same slot -> conflict */
        }
    }

    return 1;
}

int backtrack_allocate(Lab *labs, int lab_count,
                       Room *rooms, int room_count,
                       TimeSlot *slots, int slot_count,
                       ScheduleEntry *schedule,
                       int *schedule_count,
                       int lab_index) {

    /* Base case: all labs scheduled */
    if (lab_index == lab_count) return 1;

    for (int s = 0; s < slot_count; s++) {
        for (int r = 0; r < room_count; r++) {
            if (bt_is_valid(labs, lab_count, schedule, *schedule_count,
                            lab_index, s, r, rooms[r].capacity)) {

                /* Make a tentative placement */
                bt_slot_room_used[s][r] = 1;
                schedule[*schedule_count].lab_id  = lab_index;
                schedule[*schedule_count].slot_id = s;
                schedule[*schedule_count].room_id = r;
                (*schedule_count)++;

                labs[lab_index].is_allocated     = 1;
                labs[lab_index].assigned_slot_id = s;
                labs[lab_index].assigned_room_id = r;

                /* Recurse */
                if (backtrack_allocate(labs, lab_count, rooms, room_count,
                                       slots, slot_count, schedule,
                                       schedule_count, lab_index + 1))
                    return 1; /* solution found */

                /* Undo placement (backtrack) */
                (*schedule_count)--;
                bt_slot_room_used[s][r] = 0;
                labs[lab_index].is_allocated     = 0;
                labs[lab_index].assigned_slot_id = -1;
                labs[lab_index].assigned_room_id = -1;
            }
        }
    }

    return 0; /* no valid placement found for lab_index */
}

/* =========================================================
 * Section 8: Output  –  print and write schedule
 * ========================================================= */

/* Helper: find name by id */
static const char *b_name(const Batch *b, int n, int id) {
    for (int i = 0; i < n; i++) if (b[i].id == id) return b[i].name;
    return "?";
}
static const char *s_name(const Subject *s, int n, int id) {
    for (int i = 0; i < n; i++) if (s[i].id == id) return s[i].name;
    return "?";
}

/* Print a single schedule row */
static void print_row(const ScheduleEntry *e,
                      const Lab *labs, int lab_count,
                      const Room *rooms,
                      const TimeSlot *slots,
                      const Batch *batches, int batch_count,
                      const Subject *subjects, int subject_count) {
    if (e->lab_id < 0 || e->lab_id >= lab_count) return;
    const Lab  *l = &labs[e->lab_id];
    const Room *rm = &rooms[e->room_id];
    const TimeSlot *ts = &slots[e->slot_id];

    printf("%-5d %-20s %-15s %-10s %s–%s  %-12s %-5d\n",
           l->id,
           s_name(subjects, subject_count, l->subject_id),
           b_name(batches, batch_count, l->batch_id),
           ts->day, ts->start_time, ts->end_time,
           rm->code, l->student_count);
}

void print_schedule(const ScheduleEntry *schedule, int count,
                    const Lab *labs,   int lab_count,
                    const Room *rooms, int room_count,
                    const TimeSlot *slots, int slot_count,
                    const Batch *batches, int batch_count,
                    const Subject *subjects, int subject_count) {

    (void)room_count; (void)slot_count; /* suppress unused warnings */

    print_banner("FINAL SCHEDULE");
    printf("%-5s %-20s %-15s %-10s %-15s %-12s %-5s\n",
           "LabID", "Subject", "Batch", "Day", "Time", "Room", "Stud.");
    print_separator(85);

    for (int i = 0; i < count; i++) {
        print_row(&schedule[i], labs, lab_count, rooms, slots,
                  batches, batch_count, subjects, subject_count);
    }
    print_separator(85);
    printf("Total allocated: %d\n\n", count);
}

void write_schedule(const char *filepath,
                    const ScheduleEntry *schedule, int count,
                    const Lab *labs,   int lab_count,
                    const Room *rooms, int room_count,
                    const TimeSlot *slots, int slot_count,
                    const Batch *batches, int batch_count,
                    const Subject *subjects, int subject_count) {

    (void)room_count; (void)slot_count;

    FILE *fp = fopen(filepath, "w");
    if (!fp) {
        fprintf(stderr, "[ERROR] Cannot write schedule to %s\n", filepath);
        return;
    }

    fprintf(fp, "=== UNIVERSITY LAB SLOT ALLOCATION SCHEDULE ===\n\n");
    fprintf(fp, "%-5s %-20s %-15s %-10s %-15s %-12s %-5s\n",
            "LabID", "Subject", "Batch", "Day", "Time", "Room", "Stud.");
    for (int i = 0; i < 85; i++) fputc('-', fp);
    fputc('\n', fp);

    for (int i = 0; i < count; i++) {
        if (schedule[i].lab_id < 0 || schedule[i].lab_id >= lab_count) continue;
        const Lab      *l  = &labs[schedule[i].lab_id];
        const Room     *rm = &rooms[schedule[i].room_id];
        const TimeSlot *ts = &slots[schedule[i].slot_id];

        fprintf(fp, "%-5d %-20s %-15s %-10s %s-%s  %-12s %-5d\n",
                l->id,
                s_name(subjects, subject_count, l->subject_id),
                b_name(batches, batch_count, l->batch_id),
                ts->day, ts->start_time, ts->end_time,
                rm->code, l->student_count);
    }

    for (int i = 0; i < 85; i++) fputc('-', fp);
    fprintf(fp, "\nTotal allocated: %d\n", count);

    if (count == 0) {
        fprintf(fp, "\n[ERROR] No labs could be allocated. "
                    "Please check constraints and add more slots/rooms.\n");
    }

    fclose(fp);
    printf("[INFO] Schedule written to %s\n", filepath);
}
