#ifndef VISIONENGINE_H
#define VISIONENGINE_H

#include <QObject>
#include <QImage>
#include <QPoint>
#include <QHash>
#include <optional>
#include <opencv2/opencv.hpp>

#include "TemplateMatcher.h"

class QProcess;
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
    ~VisionEngine() override;
    void warmUpPaddleOcr();
    /**
     * @brief 在屏幕截图中查找模板图像
     * @param screen        屏幕截图
     * @param templateRef   模板逻辑键或模板图像路径
     * @param pt            模板图像在屏幕截图中的中心位置
     * @param threshold     匹配阈值；小于 0 时使用模板元数据中的默认阈值
     * @return              是否找到匹配
     */
    bool findTemplate(const QImage& screen, const QString& templateRef, QPoint& pt, double threshold = -1.0);
    /**
     * @brief 识别世界区域页面中的当前石油值
     * @param screen 当前屏幕截图
     * @return 识别出的石油值；识别失败时返回空
     */
    std::optional<int> readWorldZoneOilCount(const QImage& screen);

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
    /**
     * @brief 识别石油值的OCR结果字符串转换为整数
     * @param ocrResult OCR识别结果字符串
     * @return 识别出的石油值；转换失败时返回空
     */
    cv::Mat locateWorldZoneOilRoi(const cv::Mat& screenMat);
    /**
     * @brief 使用Paddle OCR识别石油值
     * @param oilRoi 石油值区域的图像
     * @return 识别出的石油值；识别失败时返回空
     */
    std::optional<int> readWorldZoneOilCountWithPaddle(const cv::Mat& oilRoi);
    /**
     * @brief 使用传统图像处理方法识别石油值
     * @param oilRoi 石油值区域的图像
     * @return 识别出的石油值；识别失败时返回空
     */
    std::optional<int> readWorldZoneOilCountFallback(const cv::Mat& oilRoi);
    
    bool ensurePaddleOcrProcess();
    void shutdownPaddleOcrProcess();
    QString paddleOcrPythonProgram() const;
    QString paddleOcrScriptPath() const;

private:
    TemplateMatcher m_matcher;
    // 模板缓存，避免重复加载同一模板图像
    QHash<QString, cv::Mat> m_templateCache;

    // Paddle OCR 相关成员
    QProcess* m_paddleOcrProcess = nullptr;
    bool m_paddleOcrDisabled = false;
    int m_paddleOcrFailureCount = 0;

};

#endif