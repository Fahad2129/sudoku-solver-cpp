#include "Solver.hpp"

bool Solver::solve(SudokuGrid& grid) {
    // Validate the starting position before attempting to solve
    if (!grid.isConsistent()) return false;
    return backtrack(grid);
}

bool Solver::backtrack(SudokuGrid& grid) {
    int row, col;

    // No more empty cells → puzzle complete
    if (!findBestEmpty(grid, row, col))
        return true;

    // Trying each digit 1–9
    for (int digit = 1; digit <= SudokuGrid::SIZE; ++digit) {
        if (grid.isValidPlacement(row, col, digit)) {
            grid.set(row, col, digit);

            if (backtrack(grid))
                return true;
            // Backtrack: undo the placement
            grid.set(row, col, 0);
        }
    }

    // No digit worked → signal failure upward
    return false;
}

bool Solver::findBestEmpty(const SudokuGrid& grid, int& outRow, int& outCol) const {
    int bestCount = SudokuGrid::SIZE + 1;
    bool found    = false;
    for (int r = 0; r < SudokuGrid::SIZE; ++r) {
        for (int c = 0; c < SudokuGrid::SIZE; ++c) {
            if (!grid.isEmpty(r, c)) continue;

            int opts = countOptions(grid, r, c);

            // If a cell has zero options the puzzle is already unsolvable;
            // returning it will cause all digits to fail quickly.
            if (opts < bestCount) {
                bestCount = opts;
                outRow    = r;
                outCol    = c;
                found     = true;

                // Can't do better than 1 option, stop scanning
                if (bestCount ==  1) return true;
            }
        }
    }
    return found;
}

int Solver::countOptions(const SudokuGrid& grid, int row, int col) const {
    int count = 0;
    for (int digit = 1; digit <= SudokuGrid::SIZE; ++digit)
        if (grid.isValidPlacement(row, col, digit))
            ++count;
    return count;
}

void Solver::markConflicts(SudokuGrid& grid) {
    // Reset all conflict flags
    for (int r = 0; r < SudokuGrid::SIZE; ++r)
        for (int c = 0; c < SudokuGrid::SIZE; ++c)
            grid.conflict[r][c] = false;

    // For each filled cell, check whether it conflicts with any peer
    for (int r = 0; r < SudokuGrid::SIZE; ++r) {
        for (int c = 0; c < SudokuGrid::SIZE; ++c) {
            int v = grid.get(r, c);
            if (v == 0) continue;

            // Temporarily clear so isValidPlacement tests against peers only
            grid.set(r, c, 0);
            if (!grid.isValidPlacement(r, c, v))
                grid.conflict[r][c] = true;
            grid.set(r, c, v);
        }
    }
}
