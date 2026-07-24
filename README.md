# Sudoku Solver

Sudoku puzzle solver built in C++17 with SFML 2.5+.

---

## Features

- **Backtracking solver** with MRV (Minimum Remaining Values) heuristic
- **Live conflict detection** — violating cells highlight in red as you type
- **Same-number highlighting** — selecting a cell highlights all matching digits
- **Solve animation** — filled cells pulse blue when the puzzle is solved
- **Full keyboard support** — arrows navigate, digits input, Backspace clears
- Three buttons: **Solve**, **Clear** (keep puzzle), **Reset** (blank slate)

---

## Project Structure

```
sudoku/
├── CMakeLists.txt
├── README.md
├── assets/
│   └── fonts/
│       ├── Inter-Regular.otf
│       ├── Inter-Medium.otf
│       ├── Inter-SemiBold.otf
│       └── Inter-Bold.otf
└── src/
    ├── main.cpp          — Entry point
    ├── SudokuGrid.hpp/cpp — 9×9 board model + constraint checks
    ├── Solver.hpp/cpp     — MRV-enhanced backtracking solver
    └── GUIApp.hpp/cpp     — SFML window, events, rendering
```

---

## Class Overview

### `SudokuGrid`
Owns the 9×9 `int board_[9][9]` array (0 = empty) and a parallel `bool given_[9][9]` mask distinguishing pre-filled cells from solver-filled ones. Exposes `isValidPlacement(row, col, value)` for constraint checking and a public `conflict[9][9]` display-metadata array.

### `Solver`
Stateless. `solve(grid)` runs recursive backtracking; `findBestEmpty()` applies MRV to pick the most-constrained empty cell first, cutting the search tree significantly. `markConflicts(grid)` sweeps every filled cell to flag violations for the GUI.

### `GUIApp`
Manages the SFML `RenderWindow`, event loop, and all drawing. Separated into:
- `handleEvent / handleMouseClick / handleKeyPress` — input
- `update(dt)` — animation clock
- `drawBackground / drawGridShadow / drawCells / drawGridLines / drawNumbers / drawButtons / drawStatusText` — rendering pipeline

---

## Dependencies

| Library | Version  | Purpose        |
|---------|----------|----------------|
| SFML    | ≥ 2.5    | Window + 2-D graphics |
| Inter   | any      | UI font (bundled in `assets/fonts/`) |

---

## Build — Linux / macOS

```bash
# 1. Install SFML
#    Ubuntu/Debian:
sudo apt install libsfml-dev

#    macOS (Homebrew):
brew install sfml

# 2. Install Inter font (optional — falls back to DejaVu Sans)
#    Ubuntu/Debian:
sudo apt install fonts-inter
#    Then copy to assets/fonts/ (already done if cloned from repo):
cp /usr/share/fonts/opentype/inter/Inter-*.otf assets/fonts/

# 3. Configure & build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 4. Run (from the build directory)
./sudoku
```

---

## Build — Windows (MSVC)

```powershell
# Assumes SFML extracted to C:\SFML
cmake .. -DSFML_DIR="C:/SFML/lib/cmake/SFML" -DCMAKE_BUILD_TYPE=Release
cmake --build . --config Release
```

Copy the four SFML DLLs (`sfml-graphics-2.dll`, `sfml-window-2.dll`,
`sfml-system-2.dll`, `openal32.dll`) next to `sudoku.exe`.

---

## How to Use

1. **Click** any cell and **type a digit (1–9)** to enter your puzzle.
2. Use **arrow keys** to navigate between cells; **Backspace** to erase.
3. Press **Solve** (or `Enter`/`S`) to solve the puzzle.
   - Pre-filled numbers remain **dark/bold**.
   - Solver-filled numbers appear in **blue**.
4. Press **Clear** (`C`) to wipe the solution and restore your input.
5. Press **Reset** (`R`) to blank the entire board.

### Conflict highlighting
Any digit that violates a row, column, or 3×3 box constraint turns **red** in real time as you type — no need to wait until you press Solve.

---

## Colour Palette

| Role              | Hex       |
|-------------------|-----------|
| Background        | `#F5F5F7` |
| Text (dark)       | `#1D1D1F` |
| Accent blue       | `#0071E3` |
| Error red         | `#FF3B30` |
| Cell selected     | `#D6EAFF` |
| Same-num highlight| `#EAF4FF` |
| Inner grid lines  | `#C8C8CE` |

---

## Algorithm Notes

The solver uses **backtracking with MRV**:

1. Find the empty cell with the **fewest legal values** (most constrained).
2. Try each digit that doesn't violate any constraint.
3. Recurse; if no digit works, backtrack and try the next candidate in the parent call.

MRV typically reduces solve time from milliseconds to microseconds on human-grade puzzles, and handles "hardest" puzzles (e.g. the Arto Inkala puzzle) in < 1 ms on modern hardware.
