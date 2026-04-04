#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"

#include <stdio.h>  // I prefer fprintf to stream "<<" syntax

#include <vector>

// struct AppModel{}
bool clicked = false;  // "high GUI" is helplessly single-threaded

void OnClick(int event, int x, int y, int/*flags*/, void *userdata) {
    (void) userdata;  // -> struct AppModel
    if(event == cv::EVENT_LBUTTONDOWN) {
        fprintf(stdout, "Click coordinates: <%d, %d>\n", x, y);
        clicked = true;
    }
}

// HighGUI -- TODO: re-test on Windows
bool isVisible(const char* win_title) {
    try
    {
        return cv::getWindowProperty(win_title, 0) >=0;
    }
    catch(const std::exception& e)
    {
        return false;
    }
}

void skim(cv::Mat& trg, const cv::Mat& src) {
    cv::resize(src, trg, cv::Size{}, 0.5, 0.5, cv::INTER_LINEAR);
}

int main(int argc, char **argv) {
    if (argc <= 1) {
        fprintf(stderr, R"HELP(
            Usage: straightface image_path
)HELP");
        return -1;
    }
    const char* file_name = argv[1];
    cv::Mat src = cv::imread(file_name);   

    std::vector<cv::Mat> trg(6);
#if SF_SEPARATE_CHANNELS
    trg.at(0) = src;
#else
    cv::cvtColor(src, trg.at(0), cv::COLOR_BGR2HSV);
#endif
    skim(trg.at(1), trg.at(0));
    skim(trg.at(2), trg.at(1));
    skim(trg.at(3), trg.at(2));
    skim(trg.at(4), trg.at(3));
    skim(trg.at(5), trg.at(4));

    const char* win_title = "sample display";
    cv::imshow(win_title, trg.at(5));

    cv::setMouseCallback(win_title, &OnClick);
    while(isVisible(win_title) && !clicked) { (void) cv::waitKey(30); }  // millis (reciprocal of framerate)

    return 0;
}
