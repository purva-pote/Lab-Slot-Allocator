# Design Document
## University Lab Slot Allocation System

---

### 1. Folder Structure

```
LabSlotAllocator/
├── src/            C source files (all algorithmic logic lives here)
├── include/        C header files (struct definitions + prototypes)
├── data/
│   ├── input/      CSV files read by the C backend
│   └── output/     schedule.txt written by the C backend
├── frontend/       Optional Python + Tkinter GUI
├── docs/           This documentation
└── Makefile        Build instructions
```

---

### 2. C Module Responsibilities

| File | Responsibility |
|------|---------------|
| `main.c` | Console menu loop; orchestrates loading, allocation, and display. |
| `lab.c / lab.h` | `Batch`, `Subject`, and `Lab` structs; CSV loaders; pretty-printer. |
| `room.c / room.h` | `Room` struct; CSV loader; helper to find room by ID. |
| `scheduler.c / scheduler.h` | `TimeSlot` struct; all four core algorithms (Merge Sort, Quick Sort, Graph Coloring, Greedy, Backtracking); CSV loader for time slots; schedule writer. |
| `utils.c / utils.h` | `trim()`, `split_csv()`, `print_banner()`, `print_separator()` — shared helper functions. |

---

### 3. Algorithm Flow

```
Load CSVs
    │
    ▼
Build Conflict Graph   ← O(n²) where n = number of labs
    │
    ▼
Graph Coloring (Backtracking)
    │  assigns each lab a time-slot "color"
    ▼
Greedy Allocation
    │  for each lab: pick first free room in its colored slot
    │
    ├─ All placed? ──── YES ──► Write schedule.txt
    │
    └─ NO ──► Quick Sort labs
                  │
                  ▼
            Backtracking Resolver
                  │
                  ├─ Solution found? ── YES ──► Write schedule.txt
                  └─ NO ──────────────────────► Write error to schedule.txt
```

---

### 4. Data Flow: GUI ↔ C Backend

```
Python GUI
  │
  ├── (user edits data in tabs)
  │       │
  │       ▼
  │   Write CSVs  ──►  data/input/*.csv
  │
  ├── (user clicks "Run Allocation")
  │       │
  │       ▼
  │   subprocess.run(lab_allocator)  ── stdin: "1\n2\n5\n"
  │                                        (load, allocate, exit)
  │       │
  │       ▼
  │   C backend reads data/input/*.csv
  │   C backend writes data/output/schedule.txt
  │       │
  │       ▼
  └── Python reads schedule.txt and displays it in the Schedule tab
```

Communication is **file-based only** — no sockets, pipes, or shared memory.

---

### 5. Key Data Structures

```c
// Conflict graph (adjacency matrix, MAX_LABS × MAX_LABS)
int conflict_graph[MAX_LABS][MAX_LABS];  // 1 = same-batch conflict

// Result
typedef struct {
    int lab_id;   // index into labs[]
    int slot_id;  // index into slots[]
    int room_id;  // index into rooms[]
} ScheduleEntry;
```

The conflict graph is a global matrix kept in `scheduler.c` and shared
across the graph-coloring and backtracking functions.
