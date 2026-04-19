# Software Requirements Specification (SRS)
## University Lab Slot Allocation System

---

### 1. Introduction

The **University Lab Slot Allocation System** automatically assigns lab sessions
(each belonging to a batch and subject) to available time slots and physical rooms,
while enforcing scheduling constraints.

**Intended Users:** Academic administrators, timetable coordinators, and lab-in-charges
at engineering colleges.

---

### 2. Functional Requirements

| ID  | Requirement |
|-----|-------------|
| FR1 | The system shall read batches, subjects, labs, rooms, and time slots from CSV files. |
| FR2 | The system shall enforce the constraint that no two labs for the **same batch** are assigned to the **same time slot**. |
| FR3 | The system shall enforce the constraint that a room's **capacity ≥ number of students** in the assigned lab. |
| FR4 | The system shall assign each lab to a valid (slot, room) pair and write the result to `data/output/schedule.txt`. |
| FR5 | The system shall report clearly if any lab cannot be scheduled due to constraint violations. |
| FR6 | The GUI shall allow users to add, view, and remove batches, subjects, labs, rooms, and time slots. |
| FR7 | The GUI's "Run Allocation" button shall invoke the C backend and display the generated schedule. |

---

### 3. Non-Functional Requirements

| ID   | Requirement |
|------|-------------|
| NFR1 | The system shall handle up to 100 labs, 30 rooms, and 40 time slots without noticeable delay. |
| NFR2 | The C backend shall compile with `gcc -std=c99` and run on any POSIX-compatible system (Linux/macOS) or Windows with MinGW. |
| NFR3 | All file paths shall be relative so the project is portable across machines. |
| NFR4 | The console interface shall validate input and display meaningful error messages. |

---

### 4. Algorithms Used

| Algorithm | Role |
|-----------|------|
| **Merge Sort** | Sorts labs by `student_count` (descending) before greedy allocation — larger classes get priority for room selection. |
| **Quick Sort** | Sorts labs by `batch_id` then `student_count` before backtracking — grouping same-batch labs improves pruning. |
| **Conflict Graph Construction** | Builds an adjacency matrix where an edge between lab *i* and lab *j* means they share a batch (and therefore cannot share a time slot). |
| **Graph Coloring (Backtracking)** | Colors the conflict graph with the minimum number of "colors" (time-slot indices) so that adjacent labs always receive different colors. This is the classical graph-coloring problem solved via recursive backtracking. |
| **Greedy Allocation** | After graph coloring, assigns rooms greedily: for each lab's assigned color (slot), find the first room with sufficient capacity that is not yet occupied in that slot. |
| **Backtracking Resolver** | If greedy allocation fails to place all labs (e.g., multiple labs colored with the same slot but only one suitable room), a full recursive backtracking search tries every valid (slot, room) pair exhaustively until a complete, constraint-satisfying schedule is found — or reports infeasibility. |
