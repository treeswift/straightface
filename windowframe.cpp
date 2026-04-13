#include "windowframe.h"

#include "opencv2/highgui.hpp"

#include <stdio.h>

namespace ui {

// HighGUI -- TODO: re-test everything on Windows

Frame::Frame(const std::string& title) : win_title(title) {
    cv::setMouseCallback(win_title, &OnClick);
}

void Frame::display(const cv::InputArray& img) {
    cv::imshow(win_title, img);
}

// MOREINFO make private{}?
void Frame::onClick(int event, int x, int y, int /*flags*/) {
    if(event == cv::EVENT_LBUTTONDOWN || event == cv::EVENT_RBUTTONDOWN) {
        fprintf(stdout, "Click coordinates: <%d, %d>\n", x, y);
        coord_events.push_back({this, event == cv::EVENT_RBUTTONDOWN, x, y});
    }
}

// MOREINFO make namespace{}?
void Frame::OnClick(int event, int x, int y, int flags, void *frame) {
    static_cast<Frame*>(frame)->onClick(event, x, y, flags);
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

void Frame::waitSingle(int frame_period) {
    while(isVisible() && coordInputPending() && keybdInputPending()) {
        // poll_kbd_events(frame_period);
        const int key_code = cv::waitKey(frame_period);
        if(key_code >= 0) {
            key_codes.push_back(key_code);
        }
    }
}

} // namespace ui
