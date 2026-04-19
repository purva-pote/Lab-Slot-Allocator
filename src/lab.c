/* =========================================================
 * lab.c  –  Lab, Batch, and Subject I/O functions
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lab.h"
#include "utils.h"

/* ----------------------------------------------------------
 * load_batches()
 * Parse data/input/batches.csv
 * Expected CSV columns: id,name,year
 * Returns number of batches read.
 * ---------------------------------------------------------- */
int load_batches(const char *filepath, Batch *batches, int max_count) {
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        fprintf(stderr, "[ERROR] Cannot open file: %s\n", filepath);
        return 0;
    }

    char line[256];
    int  count = 0;

    /* Skip header line */
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return 0;
    }

    while (fgets(line, sizeof(line), fp) && count < max_count) {
        char *tokens[8];
        int n = split_csv(line, tokens, 8);
        if (n < 3) continue; /* skip malformed lines */

        batches[count].id   = atoi(tokens[0]);
        strncpy(batches[count].name, tokens[1], MAX_NAME_LEN - 1);
        batches[count].name[MAX_NAME_LEN - 1] = '\0';
        batches[count].year = atoi(tokens[2]);
        count++;
    }

    fclose(fp);
    printf("[INFO] Loaded %d batch(es) from %s\n", count, filepath);
    return count;
}

/* ----------------------------------------------------------
 * load_subjects()
 * Parse data/input/subjects.csv
 * Expected CSV columns: id,name
 * Returns number of subjects read.
 * ---------------------------------------------------------- */
int load_subjects(const char *filepath, Subject *subjects, int max_count) {
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
        char *tokens[4];
        int n = split_csv(line, tokens, 4);
        if (n < 2) continue;

        subjects[count].id = atoi(tokens[0]);
        strncpy(subjects[count].name, tokens[1], MAX_NAME_LEN - 1);
        subjects[count].name[MAX_NAME_LEN - 1] = '\0';
        count++;
    }

    fclose(fp);
    printf("[INFO] Loaded %d subject(s) from %s\n", count, filepath);
    return count;
}

/* ----------------------------------------------------------
 * load_labs()
 * Parse data/input/labs.csv
 * Expected CSV columns: id,subject_id,batch_id,students,duration_slots
 * Returns number of labs read.
 * ---------------------------------------------------------- */
int load_labs(const char *filepath, Lab *labs, int max_count) {
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
        char *tokens[8];
        int n = split_csv(line, tokens, 8);
        if (n < 5) continue;

        labs[count].id             = atoi(tokens[0]);
        labs[count].subject_id     = atoi(tokens[1]);
        labs[count].batch_id       = atoi(tokens[2]);
        labs[count].student_count  = atoi(tokens[3]);
        labs[count].duration_slots = atoi(tokens[4]);
        labs[count].is_allocated   = 0;
        labs[count].assigned_slot_id = -1;
        labs[count].assigned_room_id = -1;
        count++;
    }

    fclose(fp);
    printf("[INFO] Loaded %d lab(s) from %s\n", count, filepath);
    return count;
}

/* ----------------------------------------------------------
 * Helper: find name by id in a Batch array
 * ---------------------------------------------------------- */
static const char *batch_name(const Batch *batches, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (batches[i].id == id) return batches[i].name;
    }
    return "Unknown";
}

/* Helper: find name by id in a Subject array */
static const char *subject_name(const Subject *subjects, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (subjects[i].id == id) return subjects[i].name;
    }
    return "Unknown";
}

/* ----------------------------------------------------------
 * print_labs()
 * Pretty-print all labs in a table to stdout.
 * ---------------------------------------------------------- */
void print_labs(const Lab *labs, int count,
                const Batch *batches, int batch_count,
                const Subject *subjects, int subject_count) {

    print_banner("LAB LIST");
    printf("%-5s %-20s %-15s %-10s %-14s %-12s\n",
           "ID", "Subject", "Batch", "Students", "Duration(slots)", "Allocated?");
    print_separator(80);

    for (int i = 0; i < count; i++) {
        printf("%-5d %-20s %-15s %-10d %-14d %-12s\n",
               labs[i].id,
               subject_name(subjects, subject_count, labs[i].subject_id),
               batch_name(batches, batch_count, labs[i].batch_id),
               labs[i].student_count,
               labs[i].duration_slots,
               labs[i].is_allocated ? "Yes" : "No");
    }
    print_separator(80);
}
