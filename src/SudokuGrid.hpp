#pragma once
#include <array>
#include <cstdint>

/**
 * SudokuGrid
 * ----------
 * Encapsulates the 9×9 Sudoku board state.
 * Tracks which cells were pre-filled by the user ("given" cells) vs.
 * cells filled in by the solver.
 *
 * Coordinates: (row, col) both 0-indexed, row=0 is top.
 * Value 0 means "empty".
 */
class SudokuGrid {
public:
    static constexpr int SIZE = 9;
    static constexpr int BOX  = 3;

    SudokuGrid();

    // --- Cell access ---
    int  get(int row, int col) const;
    void set(int row, int col, int value);

    bool isGiven(int row, int col) const;
    void setGiven(int row, int col, bool given);

    // --- Convenience ---
    bool isEmpty(int row, int col) const { return get(row, col) == 0; }

    /** Freeze the current non-zero cells as "given" cells. */
    void freezeGivens();

    /** Remove solver-filled values, keeping only givens. */
    void resetToGivens();

    /** Clear everything (givens and values). */
    void clearAll();

    /**
     * Check whether placing `value` at (row, col) respects Sudoku rules.
     * Does NOT require the cell to currently be empty.
     */
    bool isValidPlacement(int row, int col, int value) const;

    /**
     * Validate every filled cell and return true if no conflicts exist.
     * Empty cells are ignored.
     */
    bool isConsistent() const;

    /** Returns true when all 81 cells are filled. */
    bool isFull() const;

    /**
     * Per-cell conflict flag set by the GUI layer after a validation sweep.
     * Not managed by this class's own logic — purely display metadata.
     */
    bool conflict[SIZE][SIZE];

private:
    int  board_[SIZE][SIZE];
    bool given_[SIZE][SIZE];
};
