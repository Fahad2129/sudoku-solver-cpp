#include "GUIApp.hpp"
#include <cmath>
#include <iostream>
#include <sstream>

// ─────────────────────────────────────────────────────────────────────────────
// RoundedRect helper shape
// ─────────────────────────────────────────────────────────────────────────────

RoundedRect::RoundedRect(sf::Vector2f size, float radius, unsigned int ptCount)
    : size_(size), radius_(radius), ptCount_(ptCount) {
    rebuild();
}

void RoundedRect::setSize(sf::Vector2f size) {
    size_ = size;
    rebuild();
}

void RoundedRect::setRadius(float r) {
    radius_ = r;
    rebuild();
}

std::size_t RoundedRect::getPointCount() const {
    return ptCount_ * 4;
}

sf::Vector2f RoundedRect::getPoint(std::size_t index) const {
    // Which corner (0=TL, 1=TR, 2=BR, 3=BL) and angle within that corner
    unsigned int corner = static_cast<unsigned int>(index) / ptCount_;
    unsigned int local  = static_cast<unsigned int>(index) % ptCount_;

    // Angle sweeps π/2 per corner
    float angle = static_cast<float>(local) / static_cast<float>(ptCount_ - 1) * 90.f - 90.f;

    sf::Vector2f center;
    switch (corner) {
        case 0: center = { radius_,           radius_           }; break; // TL
        case 1: center = { size_.x - radius_, radius_           }; break; // TR
        case 2: center = { size_.x - radius_, size_.y - radius_ }; break; // BR
        case 3: center = { radius_,           size_.y - radius_ }; break; // BL
        default: center = {}; break;
    }
    float rad = angle * 3.14159265f / 180.f;
    return { center.x + radius_ * std::cos(rad),
             center.y + radius_ * std::sin(rad) };
}

void RoundedRect::rebuild() {
    update();
}

// ─────────────────────────────────────────────────────────────────────────────
// GUIApp
// ─────────────────────────────────────────────────────────────────────────────

namespace {
    // Hex colour helper
    sf::Color hex(uint32_t rgb, uint8_t a = 255) {
        return sf::Color(
            (rgb >> 16) & 0xFF,
            (rgb >>  8) & 0xFF,
             rgb        & 0xFF,
             a
        );
    }

    // Linear interpolation between two colours
    sf::Color lerpColor(sf::Color a, sf::Color b, float t) {
        auto lerp = [](uint8_t x, uint8_t y, float tt) -> uint8_t {
            return static_cast<uint8_t>(x + (y - x) * tt);
        };
        return { lerp(a.r,b.r,t), lerp(a.g,b.g,t),
                 lerp(a.b,b.b,t), lerp(a.a,b.a,t) };
    }
}

GUIApp::GUIApp() {
    initPalette();
    initLayout();

    // Create the window
    sf::VideoMode vm(static_cast<unsigned>(GRID_X * 2 + GRID_SIZE),
                     static_cast<unsigned>(BTN_Y + BTN_H + 60));
    window_.create(vm, "Sudoku", sf::Style::Close | sf::Style::Titlebar);
    window_.setFramerateLimit(60);

    if (!loadAssets()) {
        std::cerr << "[GUIApp] Failed to load one or more fonts.\n";
    }

    // Zero solver-filled tracker
    for (int r = 0; r < SudokuGrid::SIZE; ++r)
        for (int c = 0; c < SudokuGrid::SIZE; ++c)
            solverFilled_[r][c] = false;
}

void GUIApp::initPalette() {
    COL_BG               = hex(0xF5F5F7);
    COL_TEXT             = hex(0x1D1D1F);
    COL_ACCENT           = hex(0x0071E3);
    COL_ERROR            = hex(0xFF3B30);
    COL_GIVEN            = hex(0x1D1D1F);
    COL_CELL_BG          = sf::Color::White;
    COL_CELL_SELECTED    = hex(0xD6EAFF);   // soft blue tint
    COL_CELL_HIGHLIGHT   = hex(0xEAF4FF);   // very soft blue for same-number
    COL_LINE_THIN        = hex(0xC8C8CE);   // inner grid lines
    COL_LINE_BOLD        = hex(0x1D1D1F);   // 3×3 box lines
    COL_BTN              = hex(0x0071E3);
    COL_BTN_TEXT         = sf::Color::White;
    COL_SHADOW           = sf::Color(0, 0, 0, 25);
}

void GUIApp::initLayout() {
    CELL      = 56.f;
    GRID_SIZE = CELL * SudokuGrid::SIZE;  // 504
    GRID_X    = 48.f;
    GRID_Y    = 48.f;
    BTN_W     = 110.f;
    BTN_H     = 38.f;
    BTN_Y     = GRID_Y + GRID_SIZE + 32.f;
}

bool GUIApp::loadAssets() {
    // Try bundled assets first, then system fallbacks
    auto tryLoad = [](sf::Font& f, std::initializer_list<const char*> paths) -> bool {
        for (const char* p : paths)
            if (f.loadFromFile(p)) return true;
        return false;
    };

    bool ok = true;
    ok &= tryLoad(fontRegular_, {
        "assets/fonts/Inter-Regular.otf",
        "/usr/share/fonts/opentype/inter/Inter-Regular.otf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    });
    ok &= tryLoad(fontBold_, {
        "assets/fonts/Inter-Bold.otf",
        "/usr/share/fonts/opentype/inter/Inter-Bold.otf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    });
    ok &= tryLoad(fontMedium_, {
        "assets/fonts/Inter-Medium.otf",
        "/usr/share/fonts/opentype/inter/Inter-Medium.otf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf"
    });
    return ok;
}

// ─────────────────────────────────────────────────────────────────────────────
// Main loop
// ─────────────────────────────────────────────────────────────────────────────

void GUIApp::run() {
    sf::Clock dtClock;
    while (window_.isOpen()) {
        float dt = dtClock.restart().asSeconds();

        sf::Event event;
        while (window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window_.close();
            else
                handleEvent(event);
        }

        update(dt);
        render();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Events
// ─────────────────────────────────────────────────────────────────────────────

void GUIApp::handleEvent(const sf::Event& e) {
    if (e.type == sf::Event::MouseButtonPressed &&
        e.mouseButton.button == sf::Mouse::Left)
    {
        handleMouseClick({ static_cast<float>(e.mouseButton.x),
                           static_cast<float>(e.mouseButton.y) });
    }
    else if (e.type == sf::Event::KeyPressed) {
        handleKeyPress(e.key.code);
    }
    else if (e.type == sf::Event::TextEntered) {
        // Catch numpad / number input via TextEntered for reliability
        uint32_t ch = e.text.unicode;
        if (ch >= '1' && ch <= '9') {
            int digit = ch - '0';
            if (selected_) {
                int r = selected_->y, c = selected_->x;
                if (!grid_.isGiven(r, c)) {
                    grid_.set(r, c, digit);
                    status_ = Status::Idle;
                    animating_ = false;
                    triggerConflictCheck();
                }
            }
        }
    }
}

void GUIApp::handleMouseClick(sf::Vector2f pos) {
    // Button hit tests
    if (hitButton(pos, 0)) { actionSolve();  return; }
    if (hitButton(pos, 1)) { actionClear();  return; }
    if (hitButton(pos, 2)) { actionReset();  return; }

    // Cell selection
    auto cell = cellAt(pos);
    if (cell)
        selected_ = cell;
    else
        selected_.reset();
}

void GUIApp::handleKeyPress(sf::Keyboard::Key key) {
    using K = sf::Keyboard;

    // Arrow navigation
    if (selected_) {
        int dr = 0, dc = 0;
        if (key == K::Up)    dr = -1;
        if (key == K::Down)  dr =  1;
        if (key == K::Left)  dc = -1;
        if (key == K::Right) dc =  1;

        if (dr || dc) {
            int nr = (selected_->y + dr + SudokuGrid::SIZE) % SudokuGrid::SIZE;
            int nc = (selected_->x + dc + SudokuGrid::SIZE) % SudokuGrid::SIZE;
            selected_ = sf::Vector2i(nc, nr);
            return;
        }

        // Number keys (keyboard row)
        auto digitKey = [&]() -> int {
            if (key >= K::Num1 && key <= K::Num9) return key - K::Num1 + 1;
            if (key >= K::Numpad1 && key <= K::Numpad9) return key - K::Numpad1 + 1;
            return 0;
        };

        int digit = digitKey();
        if (digit) {
            int r = selected_->y, c = selected_->x;
            if (!grid_.isGiven(r, c)) {
                grid_.set(r, c, digit);
                status_ = Status::Idle;
                animating_ = false;
                triggerConflictCheck();
            }
            return;
        }

        // Delete / Backspace
        if (key == K::Delete || key == K::BackSpace) {
            int r = selected_->y, c = selected_->x;
            if (!grid_.isGiven(r, c)) {
                grid_.set(r, c, 0);
                status_ = Status::Idle;
                animating_ = false;
                triggerConflictCheck();
            }
        }
    }

    // Global shortcuts
    if (key == K::Return || key == K::S) actionSolve();
    if (key == K::C)                     actionClear();
    if (key == K::R)                     actionReset();
    if (key == K::Escape)                selected_.reset();
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

void GUIApp::update(float dt) {
    if (animating_) {
        animT_ = std::min(animT_ + dt * 1.2f, 1.f);
        if (animT_ >= 1.f) animating_ = false;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// Render
// ─────────────────────────────────────────────────────────────────────────────

void GUIApp::render() {
    window_.clear(COL_BG);

    drawBackground();
    drawGridShadow();
    drawCells();
    drawGridLines();
    drawNumbers();
    drawButtons();
    drawStatusText();

    window_.display();
}

void GUIApp::drawBackground() {
    sf::RectangleShape bg(sf::Vector2f(window_.getSize()));
    bg.setFillColor(COL_BG);
    window_.draw(bg);
}

void GUIApp::drawGridShadow() {
    // Layered soft shadow
    for (int i = 4; i >= 1; --i) {
        float offset = static_cast<float>(i * 2);
        float blur   = static_cast<float>(i * 3);
        sf::Color sc = sf::Color(0, 0, 0, static_cast<uint8_t>(12 - i * 2));
        drawRoundedRect(window_,
            GRID_X - blur + offset, GRID_Y - blur + offset,
            GRID_SIZE + blur * 2,   GRID_SIZE + blur * 2,
            8.f, sc);
    }
}

void GUIApp::drawCells() {
    // Which value is in the selected cell (for same-number highlighting)
    int selVal = 0;
    if (selected_)
        selVal = grid_.get(selected_->y, selected_->x);

    for (int r = 0; r < SudokuGrid::SIZE; ++r) {
        for (int c = 0; c < SudokuGrid::SIZE; ++c) {
            float x = GRID_X + c * CELL;
            float y = GRID_Y + r * CELL;

            sf::Color fill = COL_CELL_BG;

            bool isSel   = selected_ && selected_->x == c && selected_->y == r;
            bool isSame  = selVal && grid_.get(r, c) == selVal && !isSel;
            bool hasConflict = grid_.conflict[r][c];

            // Animation: pulse blue→white for solver-filled cells when solved
            if (animating_ && solverFilled_[r][c]) {
                float t = std::sin(animT_ * 3.14159f);
                fill = lerpColor(COL_CELL_BG, COL_CELL_SELECTED, t * 0.7f);
            }

            if (hasConflict)
                fill = hex(0xFF3B30, 30);  // soft red tint
            else if (isSel)
                fill = COL_CELL_SELECTED;
            else if (isSame)
                fill = COL_CELL_HIGHLIGHT;

            sf::RectangleShape cell(sf::Vector2f(CELL, CELL));
            cell.setPosition(x, y);
            cell.setFillColor(fill);
            window_.draw(cell);
        }
    }
}

void GUIApp::drawGridLines() {
    // Draw thin inner lines first, then bold box lines on top
    auto drawH = [&](int row, float thick, sf::Color col) {
        float y = GRID_Y + row * CELL;
        sf::RectangleShape line(sf::Vector2f(GRID_SIZE, thick));
        line.setPosition(GRID_X, y - thick * 0.5f);
        line.setFillColor(col);
        window_.draw(line);
    };
    auto drawV = [&](int col, float thick, sf::Color color) {
        float x = GRID_X + col * CELL;
        sf::RectangleShape line(sf::Vector2f(thick, GRID_SIZE));
        line.setPosition(x - thick * 0.5f, GRID_Y);
        line.setFillColor(color);
        window_.draw(line);
    };

    // Thin inner lines
    for (int i = 1; i < SudokuGrid::SIZE; ++i) {
        if (i % 3 != 0) {
            drawH(i, 1.f, COL_LINE_THIN);
            drawV(i, 1.f, COL_LINE_THIN);
        }
    }

    // Bold 3×3 box lines
    for (int i = 0; i <= SudokuGrid::SIZE; i += 3) {
        drawH(i, 2.2f, COL_LINE_BOLD);
        drawV(i, 2.2f, COL_LINE_BOLD);
    }
}

void GUIApp::drawNumbers() {
    for (int r = 0; r < SudokuGrid::SIZE; ++r) {
        for (int c = 0; c < SudokuGrid::SIZE; ++c) {
            int val = grid_.get(r, c);
            if (val == 0) continue;

            bool isGiven    = grid_.isGiven(r, c);
            bool hasConflict = grid_.conflict[r][c];

            sf::Color color;
            if (hasConflict)
                color = COL_ERROR;
            else if (isGiven)
                color = COL_GIVEN;
            else
                color = COL_ACCENT;

            // Pick font weight
            sf::Font& font = isGiven ? fontBold_ : fontRegular_;
            unsigned  size = isGiven ? 26u : 24u;

            sf::Text text;
            text.setFont(font);
            text.setString(std::to_string(val));
            text.setCharacterSize(size);
            text.setFillColor(color);

            // Centre in cell
            sf::FloatRect bounds = text.getLocalBounds();
            float cx = GRID_X + c * CELL + CELL * 0.5f - bounds.left - bounds.width  * 0.5f;
            float cy = GRID_Y + r * CELL + CELL * 0.5f - bounds.top  - bounds.height * 0.5f;
            text.setPosition(cx, cy);

            window_.draw(text);
        }
    }
}

void GUIApp::drawButtons() {
    // Three pill-shaped buttons: Solve | Clear | Reset
    static const char* labels[] = { "Solve", "Clear", "Reset" };
    int n = 3;

    for (int i = 0; i < n; ++i) {
        sf::FloatRect r = buttonRect(i);

        // Button shadow
        drawRoundedRect(window_, r.left + 1, r.top + 2, r.width, r.height,
                        BTN_H * 0.5f, sf::Color(0, 113, 227, 40));

        // Button body
        sf::Color btnFill = (i == 0) ? COL_BTN : hex(0xE8E8ED);
        sf::Color btnText = (i == 0) ? COL_BTN_TEXT : COL_TEXT;

        drawRoundedRect(window_, r.left, r.top, r.width, r.height,
                        BTN_H * 0.5f, btnFill);

        // Label
        sf::Text label;
        label.setFont(fontMedium_);
        label.setString(labels[i]);
        label.setCharacterSize(14);
        label.setFillColor(btnText);

        sf::FloatRect lb = label.getLocalBounds();
        label.setPosition(
            r.left + r.width  * 0.5f - lb.left - lb.width  * 0.5f,
            r.top  + r.height * 0.5f - lb.top  - lb.height * 0.5f
        );
        window_.draw(label);
    }
}

void GUIApp::drawStatusText() {
    std::string msg;
    sf::Color   color = COL_TEXT;

    switch (status_) {
        case Status::Idle:       msg = "Enter a puzzle and press Solve"; color = hex(0x86868B); break;
        case Status::Solving:    msg = "Solving…";                        color = COL_ACCENT;   break;
        case Status::Solved:     msg = "✓  Solved!";                      color = hex(0x34C759); break;
        case Status::NoSolution: msg = "No solution found";               color = COL_ERROR;    break;
    }

    sf::Text status;
    status.setFont(fontRegular_);
    status.setString(msg);
    status.setCharacterSize(13);
    status.setFillColor(color);

    // Centre below the grid
    sf::FloatRect bounds = status.getLocalBounds();
    float winW = static_cast<float>(window_.getSize().x);
    float textX = winW * 0.5f - bounds.left - bounds.width * 0.5f;
    float textY = BTN_Y + BTN_H + 14.f;
    status.setPosition(textX, textY);
    window_.draw(status);
}

// ─────────────────────────────────────────────────────────────────────────────
// Hit-testing
// ─────────────────────────────────────────────────────────────────────────────

std::optional<sf::Vector2i> GUIApp::cellAt(sf::Vector2f pos) const {
    float rx = pos.x - GRID_X;
    float ry = pos.y - GRID_Y;
    if (rx < 0 || ry < 0 || rx >= GRID_SIZE || ry >= GRID_SIZE)
        return std::nullopt;
    int c = static_cast<int>(rx / CELL);
    int r = static_cast<int>(ry / CELL);
    if (r < 0 || r >= SudokuGrid::SIZE || c < 0 || c >= SudokuGrid::SIZE)
        return std::nullopt;
    return sf::Vector2i(c, r);
}

sf::FloatRect GUIApp::buttonRect(int idx) const {
    // Three buttons centred under the grid
    float total = BTN_W * 3 + 16.f * 2;  // 2 gaps of 16px
    float startX = GRID_X + GRID_SIZE * 0.5f - total * 0.5f;
    float x = startX + idx * (BTN_W + 16.f);
    return { x, BTN_Y, BTN_W, BTN_H };
}

bool GUIApp::hitButton(sf::Vector2f pos, int idx) const {
    sf::FloatRect r = buttonRect(idx);
    return r.contains(pos);
}

// ─────────────────────────────────────────────────────────────────────────────
// Actions
// ─────────────────────────────────────────────────────────────────────────────

void GUIApp::actionSolve() {
    // Freeze current entries as givens so we can distinguish them
    grid_.freezeGivens();

    // Record which cells are currently empty (these will be solver-filled)
    for (int r = 0; r < SudokuGrid::SIZE; ++r)
        for (int c = 0; c < SudokuGrid::SIZE; ++c)
            solverFilled_[r][c] = grid_.isEmpty(r, c);

    // Clear conflict flags before solving
    for (int r = 0; r < SudokuGrid::SIZE; ++r)
        for (int c = 0; c < SudokuGrid::SIZE; ++c)
            grid_.conflict[r][c] = false;

    status_ = Status::Solving;
    render();   // show "Solving…" immediately

    if (solver_.solve(grid_)) {
        status_    = Status::Solved;
        animating_ = true;
        animT_     = 0.f;
        animClock_.restart();
    } else {
        status_    = Status::NoSolution;
        animating_ = false;
        // Restore empty cells so the user can try again
        grid_.resetToGivens();
        solver_.markConflicts(grid_);
    }
    selected_.reset();
}

void GUIApp::actionClear() {
    // Remove solver answers, keep user-entered givens
    grid_.resetToGivens();
    status_    = Status::Idle;
    animating_ = false;
    selected_.reset();
}

void GUIApp::actionReset() {
    grid_.clearAll();
    status_    = Status::Idle;
    animating_ = false;
    selected_.reset();

    for (int r = 0; r < SudokuGrid::SIZE; ++r)
        for (int c = 0; c < SudokuGrid::SIZE; ++c)
            solverFilled_[r][c] = false;
}

// ─────────────────────────────────────────────────────────────────────────────
// Utilities
// ─────────────────────────────────────────────────────────────────────────────

void GUIApp::triggerConflictCheck() {
    solver_.markConflicts(grid_);
}

void GUIApp::drawRoundedRect(sf::RenderTarget& rt,
                              float x, float y, float w, float h, float radius,
                              sf::Color fill, sf::Color outline, float outlineThick)
{
    RoundedRect shape(sf::Vector2f(w, h), std::min(radius, std::min(w, h) * 0.5f));
    shape.setPosition(x, y);
    shape.setFillColor(fill);
    if (outlineThick > 0.f) {
        shape.setOutlineColor(outline);
        shape.setOutlineThickness(outlineThick);
    }
    rt.draw(shape);
}
