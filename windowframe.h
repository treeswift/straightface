#pragma once

#include "opencv2/imgproc.hpp"

#include <functional>
#include <limits>
#include <map>
#include <string>
#include <utility>

namespace ui {

struct Frame;
struct Click { Frame* window; bool right; int x; int y; };

struct Param {
    Frame* owner;
    std::string name;
    int max_v;
    int value;
    std::function<void(int)> on = [](int){};
};

struct Frame{

    Frame(const std::string& title);
    void display(const cv::InputArray& img);
    void addKnob(const std::string& in_var, std::function<void(int)> on, int max, int def = std::numeric_limits<int>::min());

    void onMouseEvent(int event, int x, int y, int /*flags*/);
    static void OnMouseEvent(int event, int x, int y, int flags, void *frame);
    static void OnSliderEvent(int pos, void* param);

    bool isVisible() const;

    void loop(int frame_period = 30 /* millis */);

    const std::string win_title;  // must be unique within app

    // "high GUI" is helplessly single-threaded
    std::function<bool(int)> on_key                 = [](int){ return true; };  // close on keypress by default
    std::function<bool(bool, int, int)> on_click    = [](bool, int, int){ return true; }; // close on click by default
    std::function<bool(bool, int, int)> on_btnup    = [](bool, int, int){ return false; };
    std::function<void(int)> on_wheel               = [](int){};
    bool alive = false;

    std::map<std::string, Param> viewmodel;
};

} // namespace ui
