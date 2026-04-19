# University Lab Slot Allocation System

> **Core algorithms are implemented in C.**  
> An optional Python + Tkinter GUI is provided as a frontend.

---

## Quick Start

### 1. Compile the C Backend

```bash
cd LabSlotAllocator/
make
```

This produces the `lab_allocator` executable in the project root.

On Windows (MinGW):
```bash
gcc src/*.c -Iinclude -o lab_allocator.exe
```

### 2. Place / Edit CSV Input Files

All input files live in `data/input/`. Sample files are already provided.

| File | Columns |
|------|---------|
| `batches.csv` | `id, name, year` |
| `subjects.csv` | `id, name` |
| `labs.csv` | `id, subject_id, batch_id, students, duration_slots` |
| `rooms.csv` | `id, code, capacity, type` |
| `time_slots.csv` | `slot_id, day, start_time, end_time` |

### 3. Run the Console Application

```bash
./lab_allocator        # Linux / macOS
lab_allocator.exe      # Windows
```

Menu options:
```
1. Load data from CSV files
2. Run allocation
3. View schedule
4. View loaded data (labs + rooms)
5. Exit
```

The generated schedule is saved to `data/output/schedule.txt`.

---

## Optional: Python GUI

Requires Python 3 with Tkinter (usually bundled with standard Python).

```bash
python frontend/gui.py
```

The GUI lets you view and edit all input data, then click **Run Allocation**
to invoke the C backend and display the resulting schedule.

> **Note:** The C executable must be compiled before using the GUI.

---

## Algorithms (all in C)

| Algorithm | File | Purpose |
|-----------|------|---------|
| Merge Sort | `src/scheduler.c` | Sort labs by student count (descending) |
| Quick Sort | `src/scheduler.c` | Sort labs by batch then size for backtracking |
| Build Conflict Graph | `src/scheduler.c` | O(n²) adjacency matrix: edge = same batch |
| Graph Coloring (Backtracking) | `src/scheduler.c` | Assign conflict-free time-slot colors |
| Greedy Allocation | `src/scheduler.c` | Assign rooms using graph-coloring result |
| Backtracking Resolver | `src/scheduler.c` | Exhaustive fallback if greedy fails |

---

## Project Structure

```
LabSlotAllocator/
├── src/             C source files
├── include/         C headers
├── data/input/      CSV input files
├── data/output/     schedule.txt output
├── frontend/        Python + Tkinter GUI
├── docs/            SRS, Design doc, this README
└── Makefile
```

---

## Clean Build

```bash
make clean   # removes .o files and the executable
make         # recompile from scratch
```
