/* =========================================================
 * room.c  –  Room I/O functions
 * ========================================================= */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "room.h"
#include "utils.h"

/* ----------------------------------------------------------
 * load_rooms()
 * Parse data/input/rooms.csv
 * Expected CSV columns: id,code,capacity,type
 * Returns number of rooms read.
 * ---------------------------------------------------------- */
int load_rooms(const char *filepath, Room *rooms, int max_count) {
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
        if (n < 4) continue;

        rooms[count].id       = atoi(tokens[0]);
        strncpy(rooms[count].code, tokens[1], MAX_CODE_LEN - 1);
        rooms[count].code[MAX_CODE_LEN - 1] = '\0';
        rooms[count].capacity = atoi(tokens[2]);
        strncpy(rooms[count].type, tokens[3], MAX_TYPE_LEN - 1);
        rooms[count].type[MAX_TYPE_LEN - 1] = '\0';
        count++;
    }

    fclose(fp);
    printf("[INFO] Loaded %d room(s) from %s\n", count, filepath);
    return count;
}

/* ----------------------------------------------------------
 * print_rooms()
 * Pretty-print rooms to stdout.
 * ---------------------------------------------------------- */
void print_rooms(const Room *rooms, int count) {
    print_banner("ROOM LIST");
    printf("%-5s %-12s %-10s %-20s\n", "ID", "Code", "Capacity", "Type");
    print_separator(50);
    for (int i = 0; i < count; i++) {
        printf("%-5d %-12s %-10d %-20s\n",
               rooms[i].id, rooms[i].code,
               rooms[i].capacity, rooms[i].type);
    }
    print_separator(50);
}

/* ----------------------------------------------------------
 * find_room_by_id()
 * Linear search for a room with the given id.
 * Returns index into rooms[] or -1.
 * ---------------------------------------------------------- */
int find_room_by_id(const Room *rooms, int count, int id) {
    for (int i = 0; i < count; i++) {
        if (rooms[i].id == id) return i;
    }
    return -1;
}
