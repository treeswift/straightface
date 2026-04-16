#include "windowframe.h"

#include "opencv2/highgui.hpp"

#include <stdio.h>

namespace ui {

// HighGUI -- TODO: re-test everything on Windows

Frame::Frame(const std::string& title) : win_title(title) {}

void Frame::display(const cv::InputArray& img) {
    cv::imshow(win_title, img);
    alive = true;
    cv::setMouseCallback(win_title, &OnMouseEvent, this);
}

void Frame::addKnob(const std::string& in_var, std::function<void(int)> on, int max, int def) {
    if(std::numeric_limits<int>::min() == def) {
        def = max / 2;
    }
    auto bookmark = viewmodel.emplace(std::make_pair(in_var, Param{this, in_var, max, def, on}));
    if(bookmark.second) {
        auto & param = bookmark.first->second;
        cv::createTrackbar(in_var, win_title, &param.value, param.max_v, &OnSliderEvent, &*bookmark.first);
    }
}

void Frame::OnSliderEvent(int pos, void* param) {
    const auto& slider = static_cast<std::pair<std::string, Param>*>(param);
    slider->second.on(pos);
}

// MOREINFO make private{}?
void Frame::onMouseEvent(int event, int x, int y, int flags) {
    if(event == cv::EVENT_MOUSEWHEEL) {
        on_wheel(cv::getMouseWheelDelta(flags));
    }
    if((event == cv::EVENT_LBUTTONUP) || (event == cv::EVENT_RBUTTONUP)) {
        alive &= !on_click(event == cv::EVENT_RBUTTONUP, x, y);
    }
    if((event == cv::EVENT_LBUTTONDOWN) || (event == cv::EVENT_RBUTTONDOWN)) {
        alive &= !on_click(event == cv::EVENT_RBUTTONDOWN, x, y);
    }
}

// MOREINFO make namespace{}?
void Frame::OnMouseEvent(int event, int x, int y, int flags, void *frame) {
    static_cast<Frame*>(frame)->onMouseEvent(event, x, y, flags);
}

bool Frame::isVisible() const {
    try
    {
        return cv::getWindowProperty(win_title, 0) >=0;
    }
    catch(const std::exception& e)
    {
        return false;
    }
}

void Frame::loop(int frame_period) {
    while(alive && isVisible()) {
        // poll_kbd_events(frame_period);
        const int key_code = cv::waitKey(frame_period);
        if(key_code >= 0) {
            alive &= !on_key(key_code);
        }
    }
}

} // namespace ui
