"""
gui.py – University Lab Slot Allocation System
Optional Python + Tkinter Frontend

Communicates with the C backend via file-based I/O:
  - Writes CSVs to data/input/
  - Calls the compiled C executable
  - Reads data/output/schedule.txt and displays results

Usage:
    cd LabSlotAllocator/
    python frontend/gui.py
"""

import os
import sys
import subprocess
import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext

# ── Paths (relative to project root, one level up from frontend/) ──────────
SCRIPT_DIR   = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.dirname(SCRIPT_DIR)

DATA_IN      = os.path.join(PROJECT_ROOT, "data", "input")
DATA_OUT     = os.path.join(PROJECT_ROOT, "data", "output")

PATH_BATCHES  = os.path.join(DATA_IN,  "batches.csv")
PATH_SUBJECTS = os.path.join(DATA_IN,  "subjects.csv")
PATH_LABS     = os.path.join(DATA_IN,  "labs.csv")
PATH_ROOMS    = os.path.join(DATA_IN,  "rooms.csv")
PATH_SLOTS    = os.path.join(DATA_IN,  "time_slots.csv")
PATH_SCHEDULE = os.path.join(DATA_OUT, "schedule.txt")

# Executable name (adjust if on Windows: "lab_allocator.exe")
EXECUTABLE = os.path.join(PROJECT_ROOT, "lab_allocator")
if sys.platform.startswith("win"):
    EXECUTABLE += ".exe"


# ═══════════════════════════════════════════════════════════════════════════
# CSV helpers
# ═══════════════════════════════════════════════════════════════════════════

def read_csv(filepath, headers):
    """
    Read a CSV file; return list-of-dicts with given headers.
    Returns [] if the file doesn't exist or is empty.
    """
    rows = []
    if not os.path.exists(filepath):
        return rows
    with open(filepath, "r", newline="") as f:
        lines = [l.strip() for l in f if l.strip()]
    # skip header line
    for line in lines[1:]:
        parts = [p.strip() for p in line.split(",")]
        if len(parts) >= len(headers):
            rows.append(dict(zip(headers, parts)))
    return rows


def write_csv(filepath, headers, rows):
    """Write list-of-dicts as a CSV with the given header row."""
    os.makedirs(os.path.dirname(filepath), exist_ok=True)
    with open(filepath, "w", newline="") as f:
        f.write(",".join(headers) + "\n")
        for row in rows:
            f.write(",".join(str(row.get(h, "")) for h in headers) + "\n")


# ═══════════════════════════════════════════════════════════════════════════
# Generic CRUD frame (used for Batches, Rooms, Time Slots, etc.)
# ═══════════════════════════════════════════════════════════════════════════

class CRUDFrame(tk.Frame):
    """
    A reusable frame with:
      - A Treeview table showing current entries
      - Entry fields to add a new row
      - Remove Selected button
    """

    def __init__(self, parent, title, headers, filepath, **kwargs):
        super().__init__(parent, **kwargs)
        self.headers  = headers
        self.filepath = filepath
        self.rows     = []  # list of dicts

        # ── Title label ────────────────────────────────────────────────────
        tk.Label(self, text=title, font=("Helvetica", 13, "bold"),
                 bg="#1e1e2e", fg="#cdd6f4").pack(pady=(8, 4))

        # ── Treeview ───────────────────────────────────────────────────────
        frame_tree = tk.Frame(self, bg="#1e1e2e")
        frame_tree.pack(fill="both", expand=True, padx=10)

        style = ttk.Style()
        style.theme_use("clam")
        style.configure("Custom.Treeview",
                        background="#313244",
                        foreground="#cdd6f4",
                        fieldbackground="#313244",
                        rowheight=24,
                        font=("Consolas", 10))
        style.configure("Custom.Treeview.Heading",
                        background="#45475a",
                        foreground="#cdd6f4",
                        font=("Helvetica", 10, "bold"))

        self.tree = ttk.Treeview(frame_tree, columns=headers,
                                 show="headings", style="Custom.Treeview",
                                 height=8)
        for h in headers:
            self.tree.heading(h, text=h)
            self.tree.column(h, width=120, anchor="center")

        vsb = ttk.Scrollbar(frame_tree, orient="vertical",
                             command=self.tree.yview)
        self.tree.configure(yscrollcommand=vsb.set)
        self.tree.pack(side="left", fill="both", expand=True)
        vsb.pack(side="right", fill="y")

        # ── Entry row ──────────────────────────────────────────────────────
        frame_entry = tk.Frame(self, bg="#1e1e2e")
        frame_entry.pack(fill="x", padx=10, pady=6)

        self.entries = {}
        for i, h in enumerate(headers):
            tk.Label(frame_entry, text=h + ":", bg="#1e1e2e",
                     fg="#a6e3a1", font=("Helvetica", 9)).grid(
                row=0, column=i * 2, padx=(4, 1), sticky="e")
            e = tk.Entry(frame_entry, width=10,
                         bg="#313244", fg="#cdd6f4",
                         insertbackground="#cdd6f4",
                         relief="flat", font=("Consolas", 10))
            e.grid(row=0, column=i * 2 + 1, padx=(0, 6))
            self.entries[h] = e

        btn_add = tk.Button(frame_entry, text="Add", command=self.add_row,
                            bg="#a6e3a1", fg="#1e1e2e",
                            font=("Helvetica", 9, "bold"),
                            relief="flat", padx=8)
        btn_add.grid(row=0, column=len(headers) * 2, padx=4)

        btn_del = tk.Button(frame_entry, text="Remove Selected",
                            command=self.remove_selected,
                            bg="#f38ba8", fg="#1e1e2e",
                            font=("Helvetica", 9, "bold"),
                            relief="flat", padx=8)
        btn_del.grid(row=0, column=len(headers) * 2 + 1, padx=4)

        # Load existing data
        self.load()

    # ── Internal helpers ───────────────────────────────────────────────────

    def load(self):
        """Load rows from CSV and refresh the treeview."""
        self.rows = read_csv(self.filepath, self.headers)
        self._refresh_tree()

    def save(self):
        """Persist current rows to CSV."""
        write_csv(self.filepath, self.headers, self.rows)

    def _refresh_tree(self):
        """Clear treeview and repopulate from self.rows."""
        for item in self.tree.get_children():
            self.tree.delete(item)
        for row in self.rows:
            self.tree.insert("", "end",
                             values=[row.get(h, "") for h in self.headers])

    def add_row(self):
        """Read entry fields, validate, append row, save."""
        new_row = {}
        for h, e in self.entries.items():
            val = e.get().strip()
            if not val:
                messagebox.showwarning("Input Error",
                                       f"Field '{h}' cannot be empty.")
                return
            new_row[h] = val
        self.rows.append(new_row)
        self.save()
        self._refresh_tree()
        for e in self.entries.values():
            e.delete(0, "end")

    def remove_selected(self):
        """Remove the currently selected treeview row."""
        selected = self.tree.selection()
        if not selected:
            messagebox.showinfo("Select Row", "Please select a row to remove.")
            return
        idx = self.tree.index(selected[0])
        del self.rows[idx]
        self.save()
        self._refresh_tree()


# ═══════════════════════════════════════════════════════════════════════════
# Specialised Labs frame (5 columns)
# ═══════════════════════════════════════════════════════════════════════════

class LabsFrame(CRUDFrame):
    def __init__(self, parent, **kwargs):
        super().__init__(
            parent,
            title="Labs  (id, subject_id, batch_id, students, duration_slots)",
            headers=["id", "subject_id", "batch_id", "students",
                     "duration_slots"],
            filepath=PATH_LABS,
            **kwargs
        )


# ═══════════════════════════════════════════════════════════════════════════
# Schedule viewer
# ═══════════════════════════════════════════════════════════════════════════

class ScheduleFrame(tk.Frame):
    def __init__(self, parent, **kwargs):
        super().__init__(parent, **kwargs)

        tk.Label(self, text="Generated Schedule",
                 font=("Helvetica", 13, "bold"),
                 bg="#1e1e2e", fg="#cdd6f4").pack(pady=(8, 4))

        self.text = scrolledtext.ScrolledText(
            self,
            font=("Consolas", 10),
            bg="#181825", fg="#cdd6f4",
            insertbackground="#cdd6f4",
            relief="flat", state="disabled"
        )
        self.text.pack(fill="both", expand=True, padx=10, pady=6)

    def load_schedule(self):
        """Read schedule.txt and display in the text widget."""
        self.text.config(state="normal")
        self.text.delete("1.0", "end")
        if not os.path.exists(PATH_SCHEDULE):
            self.text.insert("end", "[No schedule file found. Run allocation first.]\n")
        else:
            with open(PATH_SCHEDULE, "r") as f:
                self.text.insert("end", f.read())
        self.text.config(state="disabled")


# ═══════════════════════════════════════════════════════════════════════════
# Main Application Window
# ═══════════════════════════════════════════════════════════════════════════

class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("University Lab Slot Allocation System")
        self.geometry("900x620")
        self.configure(bg="#1e1e2e")
        self.resizable(True, True)

        # ── Header bar ────────────────────────────────────────────────────
        header = tk.Frame(self, bg="#89b4fa", height=48)
        header.pack(fill="x")
        tk.Label(header,
                 text="  University Lab Slot Allocation System",
                 font=("Helvetica", 15, "bold"),
                 bg="#89b4fa", fg="#1e1e2e").pack(side="left",
                                                   pady=10, padx=8)
        tk.Label(header,
                 text="Greedy · Graph Coloring · Backtracking · Merge/Quick Sort",
                 font=("Helvetica", 9),
                 bg="#89b4fa", fg="#313244").pack(side="right",
                                                   pady=14, padx=12)

        # ── Notebook (tabs) ───────────────────────────────────────────────
        style = ttk.Style()
        style.theme_use("clam")
        style.configure("TNotebook",       background="#1e1e2e",
                         borderwidth=0)
        style.configure("TNotebook.Tab",   background="#313244",
                         foreground="#cdd6f4",
                         padding=[12, 5],
                         font=("Helvetica", 10))
        style.map("TNotebook.Tab",
                  background=[("selected", "#89b4fa")],
                  foreground=[("selected", "#1e1e2e")])

        nb = ttk.Notebook(self)
        nb.pack(fill="both", expand=True, padx=6, pady=6)

        # Tab 1: Batches
        self.tab_batches = CRUDFrame(
            nb, bg="#1e1e2e",
            title="Batches  (id, name, year)",
            headers=["id", "name", "year"],
            filepath=PATH_BATCHES
        )
        nb.add(self.tab_batches, text="Batches")

        # Tab 2: Subjects
        self.tab_subjects = CRUDFrame(
            nb, bg="#1e1e2e",
            title="Subjects  (id, name)",
            headers=["id", "name"],
            filepath=PATH_SUBJECTS
        )
        nb.add(self.tab_subjects, text="Subjects")

        # Tab 3: Labs
        self.tab_labs = LabsFrame(nb, bg="#1e1e2e")
        nb.add(self.tab_labs, text="Labs")

        # Tab 4: Rooms
        self.tab_rooms = CRUDFrame(
            nb, bg="#1e1e2e",
            title="Rooms  (id, code, capacity, type)",
            headers=["id", "code", "capacity", "type"],
            filepath=PATH_ROOMS
        )
        nb.add(self.tab_rooms, text="Rooms")

        # Tab 5: Time Slots
        self.tab_slots = CRUDFrame(
            nb, bg="#1e1e2e",
            title="Time Slots  (slot_id, day, start_time, end_time)",
            headers=["slot_id", "day", "start_time", "end_time"],
            filepath=PATH_SLOTS
        )
        nb.add(self.tab_slots, text="Time Slots")

        # Tab 6: Schedule
        self.tab_schedule = ScheduleFrame(nb, bg="#1e1e2e")
        nb.add(self.tab_schedule, text="Schedule")

        # Switch to schedule tab when it's selected
        nb.bind("<<NotebookTabChanged>>",
                lambda e: self._on_tab_change(nb))
        self._nb = nb

        # ── Bottom action bar ─────────────────────────────────────────────
        bar = tk.Frame(self, bg="#313244", height=50)
        bar.pack(fill="x", side="bottom")

        self.status_var = tk.StringVar(value="Ready.")
        tk.Label(bar, textvariable=self.status_var,
                 bg="#313244", fg="#a6e3a1",
                 font=("Consolas", 9)).pack(side="left", padx=12, pady=12)

        btn_run = tk.Button(
            bar, text="▶  Run Allocation",
            command=self.run_allocation,
            bg="#a6e3a1", fg="#1e1e2e",
            font=("Helvetica", 11, "bold"),
            relief="flat", padx=16, pady=6
        )
        btn_run.pack(side="right", padx=12, pady=8)

    def _on_tab_change(self, nb):
        """Reload schedule text when the Schedule tab is selected."""
        try:
            tab_id = nb.select()
            tab_name = nb.tab(tab_id, "text")
            if tab_name == "Schedule":
                self.tab_schedule.load_schedule()
        except Exception:
            pass

    def run_allocation(self):
        """
        1. Save all CSVs (already saved on each Add/Remove)
        2. Run the C executable with automated input (1=load, 2=run, 5=exit)
        3. Read schedule.txt and switch to the Schedule tab
        """
        self.status_var.set("Running C backend…")
        self.update()

        if not os.path.isfile(EXECUTABLE):
            messagebox.showerror(
                "Executable Not Found",
                f"Could not find:\n  {EXECUTABLE}\n\n"
                "Please compile the C code first:\n"
                "  cd LabSlotAllocator\n"
                "  make"
            )
            self.status_var.set("Error: executable not found.")
            return

        # Simulate the console menu: load (1), allocate (2), exit (5)
        stdin_input = "1\n2\n5\n"
        try:
            result = subprocess.run(
                [EXECUTABLE],
                input=stdin_input,
                capture_output=True,
                text=True,
                cwd=PROJECT_ROOT,
                timeout=30
            )
            output = result.stdout + result.stderr
            if result.returncode != 0:
                messagebox.showwarning("Allocation Warning",
                                       f"Backend exited with code {result.returncode}.\n\n"
                                       + output[-600:])
            else:
                self.status_var.set("Allocation complete. See Schedule tab.")
        except subprocess.TimeoutExpired:
            messagebox.showerror("Timeout",
                                 "The C backend took too long to respond.")
            self.status_var.set("Error: timeout.")
            return
        except Exception as exc:
            messagebox.showerror("Error", str(exc))
            self.status_var.set("Error running backend.")
            return

        # Reload the schedule tab and switch to it
        self.tab_schedule.load_schedule()
        self._nb.select(5)   # index 5 = Schedule tab
        self.status_var.set("Done – schedule loaded.")


# ═══════════════════════════════════════════════════════════════════════════
# Entry point
# ═══════════════════════════════════════════════════════════════════════════

if __name__ == "__main__":
    app = App()
    app.mainloop()
