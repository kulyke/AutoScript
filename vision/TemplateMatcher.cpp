#include "templatematcher.h"

#include <QDebug>
#include <cmath>
#include <vector>
#include <algorithm>

namespace {
struct MatchCandidate {
    cv::Rect rect;
    cv::Point maxLoc;
    double score = 0.0;
    double scale = 1.0;
};

double calcIoU(const cv::Rect& a, const cv::Rect& b)
{
    const cv::Rect inter = a & b;
    const int interArea = inter.area();
    if (interArea <= 0) {
        return 0.0;
    }

    const int unionArea = a.area() + b.area() - interArea;
    if (unionArea <= 0) {
        return 0.0;
    }
    return static_cast<double>(interArea) / static_cast<double>(unionArea);
}
}

bool TemplateMatcher::findTemplate(
        const cv::Mat& screen,
        const cv::Mat& tpl,
        QPoint& pt,
        double& score,
        double threshold)
{
    if(screen.empty() || tpl.empty()) {
        qDebug() << "TemplateMatcher::findTemplate invalid input: screen.empty=" << screen.empty()
                 << " tpl.empty=" << tpl.empty();
        score = 0.0;
        return false;
    }

    // qDebug()<<"TemplateMatcher::findTemplate screen="<<screen.cols<<"x"<<screen.rows
    //        <<" tpl="<<tpl.cols<<"x"<<tpl.rows;

    if (tpl.cols > screen.cols || tpl.rows > screen.rows) {
        qDebug() << "TemplateMatcher::findTemplate template larger than screen";
        score = 0.0;
        return false;
    }

    cv::Mat screenGray;
    cv::Mat tplGray;
    if (screen.channels() == 3) {
        cv::cvtColor(screen, screenGray, cv::COLOR_BGR2GRAY);
    } else if (screen.channels() == 4) {
        cv::cvtColor(screen, screenGray, cv::COLOR_BGRA2GRAY);
    } else {
        screenGray = screen;
    }

    if (tpl.channels() == 3) {
        cv::cvtColor(tpl, tplGray, cv::COLOR_BGR2GRAY);
    } else if (tpl.channels() == 4) {
        cv::cvtColor(tpl, tplGray, cv::COLOR_BGRA2GRAY);
    } else {
        tplGray = tpl;
    }

    // 多尺度匹配：0.95x ~ 1.05x
    constexpr double kMinScale = 0.95;
    constexpr double kMaxScale = 1.05;
    constexpr double kScaleStep = 0.05;
    constexpr int kTopPerScale = 8;
    constexpr double kNmsIouThreshold = 0.35;

    std::vector<MatchCandidate> candidates;
    double globalMax = -1.0;

    for (double s = kMinScale; s <= kMaxScale + 1e-9; s += kScaleStep) {
        const int scaledW = static_cast<int>(std::round(tplGray.cols * s));
        const int scaledH = static_cast<int>(std::round(tplGray.rows * s));
        if (scaledW < 4 || scaledH < 4) {
            continue;
        }
        if (scaledW > screenGray.cols || scaledH > screenGray.rows) {
            continue;
        }

        cv::Mat tplScaled;
        cv::resize(tplGray, tplScaled, cv::Size(scaledW, scaledH), 0, 0, cv::INTER_LINEAR);

        const int resultCols = screenGray.cols - tplScaled.cols + 1;
        const int resultRows = screenGray.rows - tplScaled.rows + 1;
        if (resultCols <= 0 || resultRows <= 0) {
            continue;
        }

        cv::Mat result;
        try {
            cv::matchTemplate(screenGray, tplScaled, result, cv::TM_CCOEFF_NORMED);
        } catch (const cv::Exception& e) {
            qDebug() << "TemplateMatcher::findTemplate OpenCV exception:" << e.what();
            continue;
        }

        // 每个尺度取 Top-K 局部峰值，避免收集过多重复点
        cv::Mat resultWork = result.clone();
        for (int k = 0; k < kTopPerScale; ++k) {
            double minVal = 0.0;
            double maxVal = 0.0;
            cv::Point minLoc;
            cv::Point maxLoc;
            cv::minMaxLoc(resultWork, &minVal, &maxVal, &minLoc, &maxLoc);

            if (!std::isfinite(maxVal) || maxVal < threshold) {
                break;
            }

            MatchCandidate c;
            c.maxLoc = maxLoc;
            c.rect = cv::Rect(maxLoc.x, maxLoc.y, tplScaled.cols, tplScaled.rows);
            c.score = maxVal;
            c.scale = s;
            candidates.push_back(c);
            globalMax = std::max(globalMax, maxVal);

            // 在结果图上抑制当前位置附近区域
            const int supHalfW = std::max(2, tplScaled.cols / 4);
            const int supHalfH = std::max(2, tplScaled.rows / 4);
            const int x0 = std::max(0, maxLoc.x - supHalfW);
            const int y0 = std::max(0, maxLoc.y - supHalfH);
            const int x1 = std::min(resultWork.cols, maxLoc.x + supHalfW + 1);
            const int y1 = std::min(resultWork.rows, maxLoc.y + supHalfH + 1);
            resultWork(cv::Rect(x0, y0, x1 - x0, y1 - y0)).setTo(-1.0f);
        }
    }

    if (candidates.empty()) {
        score = (globalMax < 0.0 ? 0.0 : globalMax);
        qDebug() << "TemplateMatcher::findTemplate no candidate over threshold=" << threshold
                 << " globalMax=" << score;
        return false;
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const MatchCandidate& a, const MatchCandidate& b) {
        return a.score > b.score;
    });

    // NMS 去重
    std::vector<MatchCandidate> kept;
    kept.reserve(candidates.size());
    for (const auto& c : candidates) {
        bool suppressed = false;
        for (const auto& k : kept) {
            if (calcIoU(c.rect, k.rect) > kNmsIouThreshold) {
                suppressed = true;
                break;
            }
        }
        if (!suppressed) {
            kept.push_back(c);
        }
    }

    const MatchCandidate& best = kept.front();
    score = best.score;
    if (score >= threshold) {
        pt.setX(best.rect.x + best.rect.width / 2);
        pt.setY(best.rect.y + best.rect.height / 2);
        qDebug() << "TemplateMatcher::findTemplate best=" << score
                 << " scale=" << best.scale
                 << " candidates=" << candidates.size()
                 << " kept=" << kept.size();
        return true;
    }

    qDebug() << "TemplateMatcher::findTemplate best score after NMS=" << score
             << " threshold=" << threshold
             << " candidates=" << candidates.size()
             << " kept=" << kept.size();
    return false;
}