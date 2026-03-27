#ifndef VISIONENGINE_H
#define VISIONENGINE_H

#include <QObject>
#include <QImage>
#include <QPoint>
#include <opencv2/opencv.hpp>

#include "TemplateMatcher.h"
/**
 * @brief  视觉引擎，执行具体的视觉任务
 * 
 */
class VisionEngine : public QObject
{
    Q_OBJECT

public:

    explicit VisionEngine(QObject *parent = nullptr);
    /**
     * @brief 在屏幕截图中查找模板图像
     * @param screen        屏幕截图
     * @param templatePath  模板图像路径
     * @param pt            模板图像在屏幕截图中的中心位置
     * @param threshold     匹配阈值，默认0.9，只有当匹配得分大于等于该值时才认为找到匹配
     * @return              是否找到匹配
     */
    bool findTemplate(const QImage& screen, const QString& templatePath, QPoint& pt, double threshold = 0.9);

private:
    /**
     * @brief 将QImage转换为OpenCV的cv::Mat格式
     * @param img 输入的QImage
     * @return 转换后的cv::Mat
     */
    cv::Mat QImageToMat(const QImage& img);

private:
    TemplateMatcher m_matcher;

};

#endif