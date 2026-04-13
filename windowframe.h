#pragma once

#include "opencv2/imgproc.hpp"

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

void onClick(int event, int x, int y, int /*flags*/);

static void OnClick(int event, int x, int y, int flags, void *frame);

bool isVisible() const;

void waitSingle(int frame_period = 30 /* millis */);

const std::string win_title;  // must be unique within app

// "high GUI" is helplessly single-threaded
std::list<int> key_codes;
std::list<Click> coord_events;

};

} // namespace ui
