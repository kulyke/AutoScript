#include "templatematcher.h"

bool TemplateMatcher::findTemplate(
        const cv::Mat& screen,
        const cv::Mat& tpl,
        QPoint& pt,
        double& score,
        double threshold)
{
    if(screen.empty() || tpl.empty())
        return false;

    cv::Mat result;

    //尺寸 (H-h+1，W-w+1)
    int result_cols = screen.cols - tpl.cols + 1;
    int result_rows = screen.rows - tpl.rows + 1;
    result.create(result_rows,result_cols,CV_32FC1);
    // 使用归一化相关系数匹配方法
    cv::matchTemplate(
                screen,
                tpl,
                result,
                cv::TM_CCOEFF_NORMED
                );
    // 找到最匹配的位置
    double minVal,maxVal;
    cv::Point minLoc,maxLoc;
    cv::minMaxLoc(
                result,
                &minVal,
                &maxVal,
                &minLoc,
                &maxLoc
                );

    score = maxVal;

    if(maxVal >= threshold) {
        pt.setX(maxLoc.x + tpl.cols/2);
        pt.setY(maxLoc.y + tpl.rows/2);
        return true;
    }

    return false;
}