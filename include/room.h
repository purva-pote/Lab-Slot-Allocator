#ifndef ROOM_H
#define ROOM_H

/* =========================================================
 * room.h  –  Data structures and prototypes for Rooms
 * ========================================================= */

#define MAX_ROOMS    30
#define MAX_CODE_LEN 16
#define MAX_TYPE_LEN 32

/* ----------------------------------------------------------
 * Room: a physical lab room
 * ---------------------------------------------------------- */
typedef struct {
    int  id;
    char code[MAX_CODE_LEN];     /* e.g., "LAB-101" */
    int  capacity;               /* max students the room holds */
    char type[MAX_TYPE_LEN];     /* e.g., "Computer", "Electronics" */
} Room;

/* ----------------------------------------------------------
 * Function prototypes (implemented in room.c)
 * ---------------------------------------------------------- */

/* Load rooms from CSV; returns number of rooms read */
int load_rooms(const char *filepath, Room *rooms, int max_count);

/* Print all rooms to stdout */
void print_rooms(const Room *rooms, int count);

/* Find a room by id; returns index or -1 if not found */
int find_room_by_id(const Room *rooms, int count, int id);

#endif /* ROOM_H */
