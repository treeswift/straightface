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

int main(int argc, char **argv) {
    if (argc <= 1) {
        fprintf(stderr, R"HELP(
            Usage: straightface image_path
)HELP");
        return -1;
    }
    const char* file_name = argv[1];
    cv::Mat src = cv::imread(file_name);

    std::vector<cv::Mat> trg;
    // TODO: cvtColor(image, hsv, COLOR_BGR2HSV);
    cv::buildPyramid(src, trg, 5);  // 1/32

    const char* win_title = "sample display";
    cv::imshow(win_title, trg.at(5));

    cv::setMouseCallback(win_title, &OnClick);
    while(!clicked) { (void) cv::waitKey(30); }  // millis (reciprocal of framerate)

    return 0;
}
