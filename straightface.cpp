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

    const int thumb_layer = 5;  // -> config
    std::vector<cv::Mat> trg(thumb_layer + 1);
#if SF_SEPARATE_CHANNELS
    trg.at(0) = src;
#else
    cv::cvtColor(src, trg.at(0), cv::COLOR_BGR2HSV);
#endif
    for(int layer_id = 1; layer_id <= thumb_layer; ++layer_id) {
        skim(trg.at(layer_id), trg.at(layer_id-1));
    }

    const int max_hue = 180;
    const int max_val = 256;
    const int histogram_dims = 2;
    const int histogram_dim[] = {max_hue, max_val};
    const float color_range[] = {0, max_hue};
    const float light_range[] = {0, max_val};
    const float* ranges[] = {color_range, light_range};
    const int hsv_components[] = {0, 2};

    auto& layer = trg.at(thumb_layer);

    cv::Mat hist_hl;
    cv::calcHist(&layer, 1, hsv_components, cv::Mat(), hist_hl, histogram_dims, histogram_dim, ranges);
    assert(hist_hl.rows == max_hue);
    assert(hist_hl.cols == max_val);

    ui::Frame frame("sample display");
    frame.display(layer);
    frame.waitSingle();

    return 0;
}
