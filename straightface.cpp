#include "windowframe.h"

#include "opencv2/core/optim.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include <stdio.h>  // I prefer fprintf to stream "<<" syntax

#include <cmath>
#include <utility>
#include <vector>

// extract to: geometry.h
inline cv::Point2d rot90(const cv::Point2d& vec) { return {vec.y, -vec.x}; }
double dot_p(const cv::Point2d& v1, const cv::Point2d& v2) { return v1.x * v2.x + v1.y * v2.x; }
double rot_p(const cv::Point2d& v1, const cv::Point2d& v2) { return dot_p(v1, rot90(v2)); }

// struct AppModel{}

void skim(cv::Mat& trg, const cv::Mat& src) {
    cv::resize(src, trg, cv::Size{}, 0.5, 0.5, cv::INTER_LINEAR);
}

struct LambdaFunction : public cv::MinProblemSolver::Function {
    LambdaFunction(int dims, std::function<double(const double*)> expr) : dimensions(dims), impl(expr) {}

    double calc(const double* x) const override {
        return impl(x);
    }

    int getDims() const override {
        return dimensions;
    }

    int dimensions;
    std::function<double(const double*)> impl;
};

// TODO expose the remaining value of `shrink_samples`
std::pair<int, int> shrink_linear(const cv::Mat& hist, float shrink_samples) {
    int i = 0;
    int j = hist.rows * hist.cols - 1;
    while(i < j) {
        float l = hist.at<float>(i);
        float r = hist.at<float>(j);
        if(l < r && l <= shrink_samples) {
            shrink_samples -= l;
            ++i;
        } else if(r <= shrink_samples) { // slightly favor contraction from the right
            shrink_samples -= r;
            --j;
        } else {
            break;
        }
    }
    return {i, j};
}

template<typename Pixel>
void dump_range(cv::Mat mat, int i, int j) {
    for(; i < j; ++i) {
        fprintf(stderr, "Bin %03d: %2.3f\n", i, mat.at<Pixel>(i));
    }
}

template<typename Pixel>
void dump_range(cv::Mat mat, const std::pair<int, int> &range) {
    dump_range<Pixel>(mat, range.first, range.second);
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
    const int lines_layer = 3;  // -> config
    std::vector<cv::Mat> trg(thumb_layer + 1);
#if SF_SEPARATE_CHANNELS
    trg.at(0) = src;
#else
    cv::cvtColor(src, trg.at(0), cv::COLOR_BGR2HSV);
#endif
    for(int layer_id = 1; layer_id <= thumb_layer; ++layer_id) {
        skim(trg.at(layer_id), trg.at(layer_id-1));
    }

    auto& thumb = trg.at(thumb_layer);

#if SF_COMPUTE_HISTOGRAM
    const int max_hue = 180;
    const int max_val = 256;
    const int histogram_dims = 2;
    const int histogram_dim[] = {max_hue, max_val};
    const float color_range[] = {0, max_hue};
    const float light_range[] = {0, max_val};
    const float* ranges[] = {color_range, light_range};
    const int hsv_components[] = {0, 2};

    cv::Mat hist_hl;
    cv::calcHist(&thumb, 1, hsv_components, cv::Mat(), hist_hl, histogram_dims, histogram_dim, ranges);
    assert(hist_hl.type() == CV_32F);
    assert(hist_hl.rows == max_hue);
    assert(hist_hl.cols == max_val);

    cv::Mat hist_h = cv::Mat::zeros(hist_hl.rows, 1, hist_hl.type());
    cv::Mat hist_l = cv::Mat::zeros(1, hist_hl.cols, hist_hl.type());
    hist_hl.forEach<float>([&](float& pixel, const int* pos) {
        hist_h.at<float>(pos[0]) += pixel;
        hist_l.at<float>(pos[1]) += pixel;
    });

#if SF_ANALYZE_HISTOGRAM
    // adjust brightness (we need a better approach for hues)
    const float shrinkerance = 0.02f;
    int total_count = thumb.rows * thumb.cols; // `thumb` same as in `cv::calcHist`
    const int shrink_count = shrinkerance * total_count; // floor
    // std::pair<int, int> hue_range = shrink_linear(hist_h, shrink_count);
    // dump_range<float>(hist_h, hue_range);
    std::pair<int, int> val_range = shrink_linear(hist_l, shrink_count);
    // dump_range<float>(hist_l, val_range);
    (void) val_range;  // note that we want a minimum lightness/saturation to keep hue differences visible
#endif // SF_ANALYZE_HISTOGRAM
#endif // SF_COMPUTE_HISTOGRAM

    ui::Frame frame("original image display");
    // here comes the model
    cv::Point2i anchor = {-1, -1};

    cv::Scalar fillco = {150, 0, 255};
    cv::Scalar lineco = {30, 128, 255};

    auto lines = trg.at(lines_layer).clone();

    std::function<double(const double*)> fitness = [&](const double* octopus) {
        const cv::Point2d* tetrapod = reinterpret_cast<const cv::Point2d*>(octopus);
        // testing for boundaries (TODO make compact):
        for(int i = 0; i < 4; ++i) {
            const cv::Point2d vert = tetrapod[i];
            if(vert.x < 0. || vert.y < 0. || vert.x >= lines.cols || vert.y >= lines.rows) {
                return 0.;
            }
        }

        // testing for convexity (TODO merge s0..s3 calculation with previous):
        cv::Point2d s0 = tetrapod[0] - tetrapod[1];
        cv::Point2d s1 = tetrapod[1] - tetrapod[2];
        cv::Point2d s2 = tetrapod[2] - tetrapod[3];
        cv::Point2d s3 = tetrapod[3] - tetrapod[0];

        // we want all the angles to have the same CW/CCW direction,
        // i.e. all the side "rot products" at the angles to have the same sign
        // (the sign inversion due to direction inversion is factored out by the comparison)
        bool a0 = std::signbit(rot_p(s0, s1));
        bool a1 = std::signbit(rot_p(s1, s2));
        bool a2 = std::signbit(rot_p(s2, s3));
        bool a3 = std::signbit(rot_p(s3, s0));
        if((a0 != a1) || (a2 != a3) || (a0 != a2)) {
            return 0.; // the quadrangle is self-intersecting or concave
        }

        cv::Point2d d0 = tetrapod[0] - tetrapod[2];
        cv::Point2d d1 = tetrapod[1] - tetrapod[3];
        // turn d1 by 90 degrees:
        //  cv::Point2d r1 = {d1.y, -d1.x};
        //  area == |d0| * |d1| * sin(d0, d1) == |d0| * |r1| * cos(d0, r1) == dot(d0, r1)
        // then, inlining:
        double area = abs(rot_p(d0, d1));

        auto isat = [&](const cv::Point2d& pt) { return lines.at<cv::Scalar>((int) pt.x, (int) pt.y) == fillco; };
        auto elen = [&](int side_index) {
            const cv::Point2d& v0 = tetrapod[side_index];
            const cv::Point2d& v1 = tetrapod[(side_index + 1) % 4];
            const cv::Point2d side = v1 - v0; // TOOD edges[side_index] // TODO rename sides into edges

            double elen = sqrt(v1.x * v1.x + v1.y * v1.y);
            if(elen < 1.) {
                return (int) isat(v0);
            }
            cv::Point2d step = side / elen;
            int ilen = (int) elen;
            int imin = std::numeric_limits<int>::max();
            int imax = 0;
            for(int i = 0; i < ilen; ++i) {
                bool atis = isat(v0 + step * i); // TODO change to search from both ends, slightly more optimal
                if(atis) {
                    imin = std::min(imin, i);
                    imax = std::max(imax, i);
                }
            }
            return (imin <= imax) ? (imax - imin + 1) : 0;
        };

        double edge = elen(0) + elen(1) + elen(2) + elen(3); // MOREINFO iterate along the actual perimeter, wrapping at fractions?
        return 0. - area * edge;
    };

    frame.display(lines); // intermediate LoD
    int greed = frame.addKnob("greed", [&](int val){
        greed = val;
    }, 30);
    frame.on_click = [&](bool right, int x, int y) {
        if(right) {
            lines = trg.at(lines_layer).clone();  // re-obtain the lines layer
        } else {
            anchor = {x, y};
            float tolerance = expf(0.1 * greed);  // MOREINFO consider int16 or fp32 pyramid? looks ok for now...
            // fprintf(stderr, "%d, %d tolerance=%f\n", x, y, tolerance);
            cv::Scalar tol3 = {tolerance, tolerance, tolerance};
            cv::floodFill(lines, anchor, fillco, nullptr, tol3, tol3, 0 * cv::FLOODFILL_FIXED_RANGE);
            // TODO use overload to update selection mask; suggest a contrasting color/choose from a palette
            
            // beginning with the seed point, write the solver:
            cv::Ptr<LambdaFunction> lf = new LambdaFunction(8, fitness); // barbarity
            cv::Mat anchvec = cv::Mat1d(1, 2);
            anchvec.at<double>(0) = anchor.x;
            anchvec.at<double>(1) = anchor.y;
            cv::Mat initvec = cv::repeat(anchvec, 1, 4);
            cv::Mat stepvec = initvec.clone();
            stepvec.at<double>(0) = stepvec.at<double>(1) =
            stepvec.at<double>(2) = stepvec.at<double>(5) = -1.;
            stepvec.at<double>(3) = stepvec.at<double>(4) =
            stepvec.at<double>(6) = stepvec.at<double>(7) = +1.;
            auto solver = cv::DownhillSolver::create(lf, stepvec);
            solver->minimize(initvec);
            cv::Point2i v0 = {initvec.at<double>(0), initvec.at<double>(1)};
            cv::Point2i v1 = {initvec.at<double>(2), initvec.at<double>(3)};
            cv::Point2i v2 = {initvec.at<double>(4), initvec.at<double>(5)};
            cv::Point2i v3 = {initvec.at<double>(6), initvec.at<double>(7)};
            cv::line(lines, v0, v1, lineco);
            cv::line(lines, v2, v1, lineco);
            cv::line(lines, v2, v3, lineco);
            cv::line(lines, v0, v3, lineco);
            for(int i = 0; i < 8; i += 2) {
                fprintf(stderr, "vertex: %lf, %lf\n", initvec.at<double>(i), initvec.at<double>(i + 1));
            }
        }
        frame.display(lines); // refresh
        return false;
    };
    frame.loop();

    return 0;
}
