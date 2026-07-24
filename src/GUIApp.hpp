#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <optional>
#include "SudokuGrid.hpp"
#include "Solver.hpp"

/**
 * GUIApp
 * ------
 * Manages the SFML window, event loop, rendering, and all UI state.
 *
 * Apple-inspired design language:
 *   - Background  : #F5F5F7  (near-white)
 *   - Text dark   : #1D1D1F  (deep charcoal)
 *   - Accent blue : #0071E3  (iOS blue)
 *   - Error red   : #FF3B30
 *   - Given text  : #1D1D1F bold
 *   - Solver text : #0071E3 regular
 */
class GUIApp {
public:
    GUIApp();
    void run();

private:
    // ── Window & timing ──────────────────────────────────────────────────
    sf::RenderWindow window_;
    sf::Clock        animClock_;   // for solve-complete animation
    bool             animating_ = false;
    float            animT_     = 0.f;  // 0..1

    // ── Model ─────────────────────────────────────────────────────────────
    SudokuGrid grid_;
    Solver     solver_;

    // ── Selection state ───────────────────────────────────────────────────
    std::optional<sf::Vector2i> selected_;  // currently highlighted cell

    // ── UI state ──────────────────────────────────────────────────────────
    enum class Status { Idle, Solving, Solved, NoSolution };
    Status      status_    = Status::Idle;

    // ── Layout constants (computed in constructor) ────────────────────────
    float CELL;          // cell size in px
    float GRID_X;        // grid top-left X
    float GRID_Y;        // grid top-left Y
    float GRID_SIZE;     // total grid size in px
    float BTN_W;
    float BTN_H;
    float BTN_Y;

    // ── Assets ────────────────────────────────────────────────────────────
    sf::Font fontRegular_;
    sf::Font fontBold_;
    sf::Font fontMedium_;

    // ── Palette ───────────────────────────────────────────────────────────
    sf::Color COL_BG;
    sf::Color COL_TEXT;
    sf::Color COL_ACCENT;
    sf::Color COL_ERROR;
    sf::Color COL_GIVEN;
    sf::Color COL_CELL_BG;
    sf::Color COL_CELL_SELECTED;
    sf::Color COL_CELL_HIGHLIGHT;  // same-number highlight
    sf::Color COL_LINE_THIN;
    sf::Color COL_LINE_BOLD;
    sf::Color COL_BTN;
    sf::Color COL_BTN_TEXT;
    sf::Color COL_SHADOW;

    // ── Internal helpers ──────────────────────────────────────────────────
    void initLayout();
    void initPalette();
    bool loadAssets();

    void handleEvent(const sf::Event& event);
    void handleMouseClick(sf::Vector2f pos);
    void handleKeyPress(sf::Keyboard::Key key);

    void update(float dt);
    void render();

    // Drawing subroutines
    void drawBackground();
    void drawGridShadow();
    void drawCells();
    void drawGridLines();
    void drawNumbers();
    void drawButtons();
    void drawStatusText();

    // Hit-testing helpers
    std::optional<sf::Vector2i> cellAt(sf::Vector2f pos) const;
    sf::FloatRect buttonRect(int idx) const;   // 0=Solve,1=Clear,2=Reset
    bool hitButton(sf::Vector2f pos, int idx) const;

    // Actions
    void actionSolve();
    void actionClear();
    void actionReset();

    // Utility
    void triggerConflictCheck();
    void drawRoundedRect(sf::RenderTarget& rt, float x, float y,
                         float w, float h, float radius,
                         sf::Color fill, sf::Color outline = sf::Color::Transparent,
                         float outlineThick = 0.f);

    // For the solve animation: which cells were filled by the solver
    bool solverFilled_[SudokuGrid::SIZE][SudokuGrid::SIZE];
};

// ── Simple rounded-rectangle shape helper ────────────────────────────────────
class RoundedRect : public sf::Shape {
public:
    RoundedRect(sf::Vector2f size, float radius, unsigned int ptCount = 20);
    void setSize(sf::Vector2f size);
    void setRadius(float r);
    std::size_t getPointCount() const override;
    sf::Vector2f getPoint(std::size_t index) const override;
private:
    sf::Vector2f size_;
    float        radius_;
    unsigned int ptCount_;
    void rebuild();
};
