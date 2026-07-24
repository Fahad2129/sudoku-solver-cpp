#pragma once
#include "SudokuGrid.hpp"

/**
 * Solver
 * ------
 * Implements a backtracking algorithm with Minimum Remaining Values (MRV)
 * heuristic to solve any valid Sudoku puzzle.
 *
 * Usage:
 *   Solver s;
 *   bool solved = s.solve(grid);
 */
class Solver {
public:
    /**
     * Attempt to solve `grid` in-place.
     * @return true  if a solution was found (grid is now complete)
     *         false if no solution exists (grid is left in an intermediate state)
     */
    bool solve(SudokuGrid& grid);

    /**
     * Mark cells that violate Sudoku constraints by setting grid.conflict flags.
     * Clears all conflict flags first, then re-checks every filled cell.
     */
    void markConflicts(SudokuGrid& grid);

private:
    /**
     * Recursive backtracking core.
     * Returns true when the grid is fully and correctly filled.
     */
    bool backtrack(SudokuGrid& grid);

    /**
     * Find the empty cell with the fewest legal values (MRV heuristic).
     * Writes the chosen cell to (outRow, outCol).
     * Returns false if no empty cell exists (puzzle is complete).
     */
    bool findBestEmpty(const SudokuGrid& grid, int& outRow, int& outCol) const;

    /** Count how many values 1-9 are currently legal for cell (row, col). */
    int countOptions(const SudokuGrid& grid, int row, int col) const;
};
