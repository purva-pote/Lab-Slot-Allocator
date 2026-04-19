/* =========================================================
 * main.c  -  University Lab Slot Allocation System
 *
 * Console menu:
 *   1. Load data from CSV files
 *   2. Run allocation (Greedy + Graph Coloring)
 *   3. View schedule
 *   4. View loaded data (labs + rooms)
 *   5. Exit
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lab.h"
#include "room.h"
#include "scheduler.h"
#include "utils.h"

/* -- File paths (relative to the project root) ------------ */
#define PATH_BATCHES    "data/input/batches.csv"
#define PATH_SUBJECTS   "data/input/subjects.csv"
#define PATH_LABS       "data/input/labs.csv"
#define PATH_ROOMS      "data/input/rooms.csv"
#define PATH_SLOTS      "data/input/time_slots.csv"
#define PATH_SCHEDULE   "data/output/schedule.txt"

/* -- Global data arrays ------------------------------------ */
/* NOTE: array is named "lab_list" (not "labs") to avoid
   name collision with the stdlib function labs() in stdlib.h */
static Batch    batches[MAX_BATCHES];
static Subject  subjects[MAX_SUBJECTS];
static Lab      lab_list[MAX_LABS];
static Room     rooms[MAX_ROOMS];
static TimeSlot slots[MAX_SLOTS];

static int batch_count   = 0;
static int subject_count = 0;
static int lab_count     = 0;
static int room_count    = 0;
static int slot_count    = 0;

static ScheduleEntry schedule[MAX_LABS];
static int           schedule_count = 0;

static int data_loaded = 0;

/* =========================================================
 * Option 1: Load CSV data
 * ========================================================= */
static void menu_load_data(void) {
    print_banner("LOADING DATA FROM CSV FILES");

    batch_count   = load_batches   (PATH_BATCHES,  batches,  MAX_BATCHES);
    subject_count = load_subjects  (PATH_SUBJECTS, subjects, MAX_SUBJECTS);
    lab_count     = load_labs      (PATH_LABS,     lab_list, MAX_LABS);
    room_count    = load_rooms     (PATH_ROOMS,    rooms,    MAX_ROOMS);
    slot_count    = load_time_slots(PATH_SLOTS,    slots,    MAX_SLOTS);

    if (lab_count == 0 || room_count == 0 || slot_count == 0) {
        printf("[WARN] Some essential data is missing. "
               "Check that CSV files exist and are correctly formatted.\n");
        data_loaded = 0;
    } else {
        printf("\n[OK] All data loaded successfully.\n");
        data_loaded = 1;
        schedule_count = 0;
        for (int i = 0; i < lab_count; i++) {
            lab_list[i].is_allocated     = 0;
            lab_list[i].assigned_slot_id = -1;
            lab_list[i].assigned_room_id = -1;
        }
    }
}

/* =========================================================
 * Option 2: Run Allocation
 * ========================================================= */
static void menu_run_allocation(void) {
    if (!data_loaded) {
        printf("[ERROR] Please load data first (Option 1).\n");
        return;
    }

    print_banner("RUNNING ALLOCATION ALGORITHM");
    schedule_count = 0;

    /* (a) Build conflict graph */
    build_conflict_graph(lab_list, lab_count);

    /* (b) Graph coloring to assign slot colors */
    int colors[MAX_LABS];
    int coloring_ok = graph_color_labs(lab_count, slot_count, colors);

    if (coloring_ok) {
        printf("\n--- Greedy Allocation ---\n");
        schedule_count = greedy_allocate(
            lab_list, lab_count,
            rooms,    room_count,
            slots,    slot_count,
            colors,   schedule
        );
    }

    /* (d) Full backtracking fallback if greedy missed any labs */
    if (schedule_count < lab_count) {
        printf("\n[INFO] Greedy placed only %d / %d labs. "
               "Attempting full backtracking search...\n",
               schedule_count, lab_count);

        schedule_count = 0;
        for (int i = 0; i < lab_count; i++) {
            lab_list[i].is_allocated     = 0;
            lab_list[i].assigned_slot_id = -1;
            lab_list[i].assigned_room_id = -1;
        }

        quick_sort_labs(lab_list, 0, lab_count - 1);

        int ok = backtrack_allocate(
            lab_list, lab_count,
            rooms,    room_count,
            slots,    slot_count,
            schedule, &schedule_count, 0
        );

        if (ok) {
            printf("[INFO] Backtracking succeeded: %d lab(s) allocated.\n",
                   schedule_count);
        } else {
            printf("[ERROR] Backtracking could not satisfy all constraints.\n"
                   "        Consider adding more rooms or time slots.\n");
        }
    }

    write_schedule(PATH_SCHEDULE,
                   schedule,  schedule_count,
                   lab_list,  lab_count,
                   rooms,     room_count,
                   slots,     slot_count,
                   batches,   batch_count,
                   subjects,  subject_count);

    printf("\nAllocation complete. %d / %d lab(s) scheduled.\n",
           schedule_count, lab_count);
}

/* =========================================================
 * Option 3: View Schedule
 * ========================================================= */
static void menu_view_schedule(void) {
    if (schedule_count == 0) {
        printf("[INFO] No schedule generated yet. Run allocation first (Option 2).\n");
        return;
    }
    print_schedule(schedule,  schedule_count,
                   lab_list,  lab_count,
                   rooms,     room_count,
                   slots,     slot_count,
                   batches,   batch_count,
                   subjects,  subject_count);
}

/* =========================================================
 * Option 4: View loaded data
 * ========================================================= */
static void menu_view_data(void) {
    if (!data_loaded) {
        printf("[ERROR] No data loaded yet. Use Option 1 first.\n");
        return;
    }
    print_labs(lab_list, lab_count, batches, batch_count, subjects, subject_count);
    print_rooms(rooms, room_count);
}

/* =========================================================
 * main()  -  Entry point and menu loop
 * ========================================================= */
int main(void) {
    int choice;

    print_banner("UNIVERSITY LAB SLOT ALLOCATION SYSTEM");
    printf("  Algorithms: Merge Sort | Quick Sort | "
           "Greedy | Graph Coloring | Backtracking\n\n");

    while (1) {
        printf("=== MAIN MENU ===\n");
        printf("  1. Load data from CSV files\n");
        printf("  2. Run allocation\n");
        printf("  3. View schedule\n");
        printf("  4. View loaded data (labs + rooms)\n");
        printf("  5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("[ERROR] Invalid input. Please enter a number 1-5.\n\n");
            continue;
        }
        printf("\n");

        switch (choice) {
            case 1: menu_load_data();      break;
            case 2: menu_run_allocation(); break;
            case 3: menu_view_schedule();  break;
            case 4: menu_view_data();      break;
            case 5: printf("Goodbye!\n"); return 0;
            default: printf("[ERROR] Invalid option. Choose 1-5.\n");
        }
        printf("\n");
    }
    return 0;
}
