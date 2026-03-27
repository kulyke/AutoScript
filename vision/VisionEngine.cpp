#include "VisionEngine.h"

#include <QDebug>

VisionEngine::VisionEngine(QObject *parent)
    : QObject(parent)
{

}

cv::Mat VisionEngine::QImageToMat(const QImage &img)
{
    QImage rgb = img.convertToFormat(QImage::Format_RGB888);

    cv::Mat mat(
                rgb.height(),
                rgb.width(),
                CV_8UC3,
                (void*)rgb.bits(),
                rgb.bytesPerLine()
                );

    return mat.clone();
}

bool VisionEngine::findTemplate(const QImage& screen, const QString& templatePath, QPoint& pt, double threshold)
{
    cv::Mat screenMat = QImageToMat(screen);
    // 加载模板图像
    cv::Mat tpl = cv::imread(templatePath.toStdString());
    if(tpl.empty()) {
        qDebug()<<"template load failed";
        return false;
    }

    // 查找模板
    double score;
    bool ok = m_matcher.findTemplate(
                screenMat,
                tpl,
                pt,
                score,
                threshold
                );
    if(ok) {
        qDebug()<<"match success score="<<score;
    }
    return ok;
}