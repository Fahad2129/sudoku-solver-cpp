#include "SudokuGrid.hpp"
#include <stdexcept>

SudokuGrid::SudokuGrid() {
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c) {
            board_[r][c]    = 0;
            given_[r][c]    = false;
            conflict[r][c]  = false;
        }
}

int SudokuGrid::get(int row, int col) const {
    return board_[row][col];
}

void SudokuGrid::set(int row, int col, int value) {
    board_[row][col] = value;
}

bool SudokuGrid::isGiven(int row, int col) const {
    return given_[row][col];
}

void SudokuGrid::setGiven(int row, int col, bool given) {
    given_[row][col] = given;
}

void SudokuGrid::freezeGivens() {
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c)
            given_[r][c] = (board_[r][c] != 0);
}

void SudokuGrid::resetToGivens() {
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c) {
            conflict[r][c] = false;
            if (!given_[r][c])
                board_[r][c] = 0;
        }
}

void SudokuGrid::clearAll() {
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c) {
            board_[r][c]   = 0;
            given_[r][c]   = false;
            conflict[r][c] = false;
        }
}

bool SudokuGrid::isValidPlacement(int row, int col, int value) const {
    if (value < 1 || value > 9) return false;

    // Check row
    for (int c = 0; c < SIZE; ++c)
        if (c != col && board_[row][c] == value)
            return false;

    // Check column
    for (int r = 0; r < SIZE; ++r)
        if (r != row && board_[r][col] == value)
            return false;

    // Check 3×3 box
    int boxRow = (row / BOX) * BOX;
    int boxCol = (col / BOX) * BOX;
    for (int r = boxRow; r < boxRow + BOX; ++r)
        for (int c = boxCol; c < boxCol + BOX; ++c)
            if ((r != row || c != col) && board_[r][c] == value)
                return false;

    return true;
}

bool SudokuGrid::isConsistent() const {
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c) {
            int v = board_[r][c];
            if (v != 0 && !isValidPlacement(r, c, v))
                return false;
        }
    return true;
}

bool SudokuGrid::isFull() const {
    for (int r = 0; r < SIZE; ++r)
        for (int c = 0; c < SIZE; ++c)
            if (board_[r][c] == 0)
                return false;
    return true;
}
