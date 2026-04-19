#ifndef LAB_H
#define LAB_H

/* =========================================================
 * lab.h  –  Data structures and prototypes for Lab entities
 * ========================================================= */

#define MAX_LABS      100
#define MAX_BATCHES   20
#define MAX_SUBJECTS  50
#define MAX_NAME_LEN  64

/* ----------------------------------------------------------
 * Batch: a group of students (e.g., "CS-A 2nd Year")
 * ---------------------------------------------------------- */
typedef struct {
    int  id;
    char name[MAX_NAME_LEN];
    int  year;          /* academic year: 1, 2, 3, or 4 */
} Batch;

/* ----------------------------------------------------------
 * Subject: a course that has lab sessions
 * ---------------------------------------------------------- */
typedef struct {
    int  id;
    char name[MAX_NAME_LEN];
} Subject;

/* ----------------------------------------------------------
 * Lab: a single lab session to be scheduled
 * ---------------------------------------------------------- */
typedef struct {
    int  id;
    int  subject_id;
    int  batch_id;
    int  student_count;     /* number of students in the lab */
    int  duration_slots;    /* how many consecutive slots needed */
    int  is_allocated;      /* 0 = not yet scheduled, 1 = scheduled */
    int  assigned_slot_id;  /* index into TimeSlot array (-1 if none) */
    int  assigned_room_id;  /* index into Room array (-1 if none) */
} Lab;

/* ----------------------------------------------------------
 * Function prototypes (implemented in lab.c)
 * ---------------------------------------------------------- */

/* Load batches from CSV; returns number read */
int load_batches(const char *filepath, Batch *batches, int max_count);

/* Load labs from CSV; returns number read */
int load_labs(const char *filepath, Lab *labs, int max_count);

/* Print all labs to stdout */
void print_labs(const Lab *labs, int count,
                const Batch *batches, int batch_count,
                const Subject *subjects, int subject_count);

/* Load subjects from a simple CSV; returns number read */
int load_subjects(const char *filepath, Subject *subjects, int max_count);

#endif /* LAB_H */
