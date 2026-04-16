#pragma once

#include "opencv2/imgproc.hpp"

#include <functional>
#include <list>
#include <string>
#include <utility>

namespace ui {

struct Frame;
struct Click { Frame* window; bool right; int x; int y; };

struct Frame{

    Frame(const std::string& title);
    void display(const cv::InputArray& img);

    bool keybdInputPending() const { return key_codes.empty(); }
    bool coordInputPending() const { return coord_events.empty(); }

    void onMouseEvent(int event, int x, int y, int /*flags*/);

    static void OnMouseEvent(int event, int x, int y, int flags, void *frame);

    bool isVisible() const;

    void loop(int frame_period = 30 /* millis */);

    const std::string win_title;  // must be unique within app

    // "high GUI" is helplessly single-threaded
    std::function<bool(int)> on_key                 = [](int){ return true; };  // close on keypress by default
    std::function<bool(bool, int, int)> on_click    = [](bool, int, int){ return true; }; // close on click by default
    std::function<bool(bool, int, int)> on_btnup    = [](bool, int, int){ return false; };
    std::function<void(int)> on_wheel               = [](int){};
    bool alive = false;
    // FIXME: replace with handlers
    std::list<int> key_codes;
    std::list<Click> coord_events;

};

} // namespace ui
