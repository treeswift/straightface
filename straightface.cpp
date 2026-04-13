#include "windowframe.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include <stdio.h>  // I prefer fprintf to stream "<<" syntax

#include <vector>

// struct AppModel{}

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

    ui::Frame frame("sample display");
    frame.display(trg.at(5));
    frame.waitSingle();

    return 0;
}
