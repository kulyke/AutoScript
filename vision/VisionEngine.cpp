#include "VisionEngine.h"

#include "TemplateCatalog.h"

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

const TemplateMetadata* VisionEngine::resolveTemplateMetadata(const QString& templateRef) const
{
    return TemplateCatalog::findByKey(templateRef);
}

cv::Mat VisionEngine::loadTemplate(const QString& templateRef,
                                   QString& resolvedPath,
                                   double& resolvedThreshold)
{
    const TemplateMetadata* metadata = resolveTemplateMetadata(templateRef);
    resolvedPath = metadata ? metadata->path : templateRef;
    resolvedThreshold = metadata ? metadata->defaultThreshold : 0.9;

    const auto cacheIt = m_templateCache.constFind(resolvedPath);
    if (cacheIt != m_templateCache.constEnd()) {
        return cacheIt.value();
    }

    cv::Mat tpl = cv::imread(resolvedPath.toStdString());
    if (tpl.empty()) {
        return cv::Mat();
    }

    m_templateCache.insert(resolvedPath, tpl);
    return tpl;
}

bool VisionEngine::findTemplate(const QImage& screen, const QString& templateRef, QPoint& pt, double threshold)
{
    cv::Mat screenMat = QImageToMat(screen);
    if (screenMat.empty()) {
        qDebug() << "screen.isNull=" << screen.isNull() << " size=" << screen.size();
    }

    QString resolvedPath;
    double defaultThreshold = 0.9;
    cv::Mat tpl = loadTemplate(templateRef, resolvedPath, defaultThreshold);
    if(tpl.empty()) {
        qDebug() << "template load failed:" << templateRef << "resolvedPath=" << resolvedPath;
        return false;
    }

    const double effectiveThreshold = threshold >= 0.0 ? threshold : defaultThreshold;

    // 查找模板
    double score;
    bool ok = m_matcher.findTemplate(
                screenMat,
                tpl,
                pt,
                score,
                effectiveThreshold
                );
    if(ok) {
        qDebug() << "match success template=" << templateRef << "score=" << score;
    }
    return ok;
}