#ifndef TEMPLATEMATCHER_H
#define TEMPLATEMATCHER_H

#include <opencv2/opencv.hpp>
#include <QPoint>

/**
 * @brief 模板匹配器，负责在屏幕截图中查找特定的图像
 * 
 */
class TemplateMatcher
{
public:
    /**
     * @brief               在屏幕截图中查找模板图像
     * @param screen        屏幕截图
     * @param tpl           模板图像
     * @param pt            模板图像在屏幕截图中的中心位置
     * @param score         匹配得分，范围[0,1]，值越大表示匹配越好
     * @param threshold     匹配阈值，默认0.9，只有当匹配得分大于等于该值时才认为找到匹配
     * @return              是否找到匹配
     */
    bool findTemplate(
        const cv::Mat& screen,
        const cv::Mat& tpl,
        QPoint& pt,
        double& score,
        double threshold = 0.9
    );

};

#endif