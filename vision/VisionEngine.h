#ifndef VISIONENGINE_H
#define VISIONENGINE_H

#include <QObject>
#include <QImage>
#include <QPoint>
#include <QHash>
#include <opencv2/opencv.hpp>

#include "TemplateMatcher.h"

struct TemplateMetadata;
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
     * @param templateRef   模板逻辑键或模板图像路径
     * @param pt            模板图像在屏幕截图中的中心位置
     * @param threshold     匹配阈值；小于 0 时使用模板元数据中的默认阈值
     * @return              是否找到匹配
     */
    bool findTemplate(const QImage& screen, const QString& templateRef, QPoint& pt, double threshold = -1.0);

private:
    /**
     * @brief 将QImage转换为OpenCV的cv::Mat格式
     * @param img 输入的QImage
     * @return 转换后的cv::Mat
     */
    cv::Mat QImageToMat(const QImage& img);
    /**
     * @brief 根据模板引用解析模板元数据
     * @param templateRef 模板逻辑键或模板图像路径
     * @return 模板元数据指针，如果未找到则返回nullptr
     */
    const TemplateMetadata* resolveTemplateMetadata(const QString& templateRef) const;
    /**
     * @brief 加载模板图像
     * @param templateRef 模板逻辑键或模板图像路径
     * @param resolvedPath 输出参数，返回解析后的模板图像路径
     * @param resolvedThreshold 输出参数，返回解析后的匹配阈值
     * @return 加载的模板图像，如果加载失败则返回空的cv::Mat
     */
    cv::Mat loadTemplate(const QString& templateRef, QString& resolvedPath, double& resolvedThreshold);

private:
    TemplateMatcher m_matcher;
    // 模板缓存，避免重复加载同一模板图像
    QHash<QString, cv::Mat> m_templateCache;

};

#endif