#include "VisionEngine.h"

#include "TemplateCatalog.h"

#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QRegularExpression>

#include <algorithm>
#include <array>
#include <chrono>
#include <optional>
#include <vector>

namespace {

constexpr int kPaddleReadyTimeoutMs = 30000;
constexpr int kPaddleResponseTimeoutMs = 8000;
constexpr int kPaddleMaxFailures = 3;

QString findBundledPython(const QDir& baseDir)
{
#ifdef Q_OS_WIN
    static const QStringList relativePaths = {
        ".venv/Scripts/python.exe",
        "../.venv/Scripts/python.exe",
        "../../.venv/Scripts/python.exe",
    };
#else
    static const QStringList relativePaths = {
        ".venv/bin/python3",
        ".venv/bin/python",
        "../.venv/bin/python3",
        "../.venv/bin/python",
        "../../.venv/bin/python3",
        "../../.venv/bin/python",
    };
#endif

    for (const QString& relativePath : relativePaths) {
        const QString candidate = QDir::cleanPath(baseDir.filePath(relativePath));
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }

    return {};
}

std::optional<int> extractDigitsValue(const QString& text)
{
    static const QRegularExpression digitsPattern("(\\d+)");
    const QRegularExpressionMatch match = digitsPattern.match(text);
    if (!match.hasMatch()) {
        return std::nullopt;
    }

    bool ok = false;
    const int value = match.captured(1).toInt(&ok);
    if (!ok) {
        return std::nullopt;
    }
    return value;
}

QByteArray readNextJsonLine(QProcess* process, int timeoutMs)
{
    if (!process) {
        return {};
    }

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    while (std::chrono::steady_clock::now() < deadline) {
        while (process->canReadLine()) {
            const QByteArray line = process->readLine().trimmed();
            if (!line.isEmpty()) {
                return line;
            }
        }

        const auto now = std::chrono::steady_clock::now();
        if (now >= deadline) {
            break;
        }

        const int remainingMs = static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(deadline - now).count());
        if (!process->waitForReadyRead(std::max(1, remainingMs))) {
            break;
        }
    }

    return {};
}

cv::Mat extractLetters(const cv::Mat& image, const cv::Scalar& letterColor, int threshold)
{
    cv::Mat diff;
    cv::absdiff(image, cv::Mat(image.size(), image.type(), letterColor), diff);

    std::vector<cv::Mat> channels;
    cv::split(diff, channels);

    cv::Mat maxDiff = channels[0].clone();
    for (size_t index = 1; index < channels.size(); ++index) {
        cv::max(maxDiff, channels[index], maxDiff);
    }

    cv::Mat mask;
    cv::threshold(maxDiff, mask, threshold, 255, cv::THRESH_BINARY_INV);
    return mask;
}

std::vector<cv::Rect> findDigitBounds(const cv::Mat& binary)
{
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(binary, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    std::vector<cv::Rect> boxes;
    for (const auto& contour : contours) {
        const cv::Rect box = cv::boundingRect(contour);
        const int area = box.width * box.height;
        if (area < 18) {
            continue;
        }
        if (box.height < binary.rows / 3) {
            continue;
        }
        if (box.width > binary.cols / 2) {
            continue;
        }
        boxes.push_back(box);
    }

    std::sort(boxes.begin(), boxes.end(), [](const cv::Rect& lhs, const cv::Rect& rhs) {
        return lhs.x < rhs.x;
    });
    return boxes;
}

int classifyDigitBySegments(const cv::Mat& digit)
{
    if (digit.empty()) {
        return -1;
    }

    cv::Mat resized;
    cv::resize(digit, resized, cv::Size(24, 36), 0.0, 0.0, cv::INTER_NEAREST);

    const std::array<cv::Rect, 7> segmentRects = {
        cv::Rect(5, 0, 14, 5),
        cv::Rect(0, 4, 6, 13),
        cv::Rect(18, 4, 6, 13),
        cv::Rect(5, 15, 14, 6),
        cv::Rect(0, 19, 6, 13),
        cv::Rect(18, 19, 6, 13),
        cv::Rect(5, 31, 14, 5)
    };

    std::array<int, 7> segments{};
    for (size_t index = 0; index < segmentRects.size(); ++index) {
        const cv::Mat region = resized(segmentRects[index]);
        const double fillRatio = static_cast<double>(cv::countNonZero(region))
            / static_cast<double>(region.total());
        segments[index] = fillRatio >= 0.18 ? 1 : 0;
    }

    const std::array<std::array<int, 7>, 10> digitPatterns = {{
        {{1, 1, 1, 0, 1, 1, 1}},
        {{0, 0, 1, 0, 0, 1, 0}},
        {{1, 0, 1, 1, 1, 0, 1}},
        {{1, 0, 1, 1, 0, 1, 1}},
        {{0, 1, 1, 1, 0, 1, 0}},
        {{1, 1, 0, 1, 0, 1, 1}},
        {{1, 1, 0, 1, 1, 1, 1}},
        {{1, 0, 1, 0, 0, 1, 0}},
        {{1, 1, 1, 1, 1, 1, 1}},
        {{1, 1, 1, 1, 0, 1, 1}}
    }};

    int bestDigit = -1;
    int bestDistance = std::numeric_limits<int>::max();
    for (int digitValue = 0; digitValue < static_cast<int>(digitPatterns.size()); ++digitValue) {
        int distance = 0;
        for (size_t segmentIndex = 0; segmentIndex < segments.size(); ++segmentIndex) {
            if (segments[segmentIndex] != digitPatterns[digitValue][segmentIndex]) {
                ++distance;
            }
        }
        if (distance < bestDistance) {
            bestDistance = distance;
            bestDigit = digitValue;
        }
    }

    return bestDistance <= 2 ? bestDigit : -1;
}

std::optional<int> parseOilDigits(const cv::Mat& digitBinary)
{
    const std::vector<cv::Rect> boxes = findDigitBounds(digitBinary);
    if (boxes.empty()) {
        return std::nullopt;
    }

    int value = 0;
    for (const cv::Rect& box : boxes) {
        cv::Mat digit = digitBinary(box).clone();
        const int digitValue = classifyDigitBySegments(digit);
        if (digitValue < 0) {
            return std::nullopt;
        }
        value = value * 10 + digitValue;
    }

    return value;
}

cv::Mat thresholdOilDigits(const cv::Mat& oilRoi)
{
    const std::array<cv::Scalar, 3> candidateColors = {
        cv::Scalar(247, 247, 247),
        cv::Scalar(201, 201, 201),
        cv::Scalar(165, 165, 165)
    };
    const std::array<int, 3> candidateThresholds = {36, 32, 28};

    cv::Mat bestBinary;
    int bestScore = -1;
    for (size_t index = 0; index < candidateColors.size(); ++index) {
        cv::Mat binary = extractLetters(oilRoi, candidateColors[index], candidateThresholds[index]);
        cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(2, 2));
        cv::morphologyEx(binary, binary, cv::MORPH_OPEN, kernel);

        const int score = cv::countNonZero(binary);
        if (score > bestScore) {
            bestScore = score;
            bestBinary = binary;
        }
    }

    return bestBinary;
}

}

VisionEngine::VisionEngine(QObject *parent)
    : QObject(parent)
{

}

VisionEngine::~VisionEngine()
{
    shutdownPaddleOcrProcess();
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

QString VisionEngine::paddleOcrPythonProgram() const
{
    const QString configuredProgram = qEnvironmentVariable("AUTOSCRIPT_PYTHON");
    if (!configuredProgram.isEmpty()) {
        return configuredProgram;
    }

    const QString bundledPython = findBundledPython(QDir(QCoreApplication::applicationDirPath()));
    if (!bundledPython.isEmpty()) {
        return bundledPython;
    }

    return "python";
}

QString VisionEngine::paddleOcrScriptPath() const
{
    const QString configuredPath = qEnvironmentVariable("AUTOSCRIPT_PADDLE_OCR_SCRIPT");
    if (!configuredPath.isEmpty()) {
        return configuredPath;
    }

    const QDir appDir(QCoreApplication::applicationDirPath());
    const QString deployedPath = appDir.filePath("scripts/paddle_ocr_service.py");
    if (QFileInfo::exists(deployedPath)) {
        return deployedPath;
    }

    const QString parentPath = appDir.filePath("../scripts/paddle_ocr_service.py");
    if (QFileInfo::exists(parentPath)) {
        return QDir::cleanPath(parentPath);
    }

    const QString grandParentPath = appDir.filePath("../../scripts/paddle_ocr_service.py");
    return QDir::cleanPath(grandParentPath);
}

void VisionEngine::shutdownPaddleOcrProcess()
{
    if (!m_paddleOcrProcess) {
        return;
    }

    if (m_paddleOcrProcess->state() != QProcess::NotRunning) {
        m_paddleOcrProcess->write("{\"command\":\"shutdown\"}\n");
        m_paddleOcrProcess->waitForBytesWritten(500);
        if (!m_paddleOcrProcess->waitForFinished(1000)) {
            m_paddleOcrProcess->kill();
            m_paddleOcrProcess->waitForFinished(1000);
        }
    }

    m_paddleOcrProcess->deleteLater();
    m_paddleOcrProcess = nullptr;
}

bool VisionEngine::ensurePaddleOcrProcess()
{
    if (m_paddleOcrDisabled) {
        return false;
    }

    const QString enabledValue = qEnvironmentVariable("AUTOSCRIPT_ENABLE_PADDLE_OCR");
    if (enabledValue == "0" || enabledValue.compare("false", Qt::CaseInsensitive) == 0) {
        m_paddleOcrDisabled = true;
        qDebug() << "paddle OCR disabled by environment";
        return false;
    }

    if (m_paddleOcrProcess && m_paddleOcrProcess->state() != QProcess::NotRunning) {
        return true;
    }

    shutdownPaddleOcrProcess();

    const QString scriptPath = paddleOcrScriptPath();
    if (!QFileInfo::exists(scriptPath)) {
        qDebug() << "paddle OCR script not found:" << scriptPath;
        m_paddleOcrDisabled = true;
        return false;
    }

    m_paddleOcrProcess = new QProcess(this);
    const QString pythonProgram = paddleOcrPythonProgram();
    qDebug() << "starting paddle OCR process with" << pythonProgram << scriptPath;
    m_paddleOcrProcess->setProgram(pythonProgram);
    m_paddleOcrProcess->setArguments({scriptPath});
    m_paddleOcrProcess->setWorkingDirectory(QCoreApplication::applicationDirPath());
    m_paddleOcrProcess->setProcessChannelMode(QProcess::SeparateChannels);
    m_paddleOcrProcess->start();

    if (!m_paddleOcrProcess->waitForStarted(3000)) {
        qDebug() << "failed to start paddle OCR process:" << m_paddleOcrProcess->errorString();
        m_paddleOcrDisabled = true;
        shutdownPaddleOcrProcess();
        return false;
    }

    const QByteArray readyLine = readNextJsonLine(m_paddleOcrProcess, kPaddleReadyTimeoutMs);
    if (readyLine.isEmpty()) {
        qDebug() << "paddle OCR ready timeout, stderr="
                 << QString::fromUtf8(m_paddleOcrProcess->readAllStandardError()).trimmed();
        ++m_paddleOcrFailureCount;
        if (m_paddleOcrFailureCount >= kPaddleMaxFailures) {
            m_paddleOcrDisabled = true;
        }
        shutdownPaddleOcrProcess();
        return false;
    }

    const QJsonDocument readyDoc = QJsonDocument::fromJson(readyLine);
    if (!readyDoc.isObject() || !readyDoc.object().value("ready").toBool()) {
        qDebug() << "unexpected paddle OCR ready payload:" << readyLine
                 << "stderr=" << QString::fromUtf8(m_paddleOcrProcess->readAllStandardError()).trimmed();
        ++m_paddleOcrFailureCount;
        if (m_paddleOcrFailureCount >= kPaddleMaxFailures) {
            m_paddleOcrDisabled = true;
        }
        shutdownPaddleOcrProcess();
        return false;
    }

    m_paddleOcrFailureCount = 0;
    qDebug() << "paddle OCR process ready";
    return true;
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
    } else {
        qDebug() << "match failed template=" << templateRef << "score=" << score;
    }
    return ok;
}

cv::Mat VisionEngine::locateWorldZoneOilRoi(const cv::Mat& screenMat)
{
    QString resolvedPath;
    double defaultThreshold = 0.9;
    cv::Mat fullTemplate = loadTemplate("worldZone.oil.add.button", resolvedPath, defaultThreshold);
    if (fullTemplate.empty()) {
        qDebug() << "oil anchor template load failed:" << resolvedPath;
        return cv::Mat();
    }

    cv::imwrite("debug_full_template.png", fullTemplate);
    const int anchorSize = std::min(fullTemplate.cols, fullTemplate.rows);
    if (anchorSize <= 0 || fullTemplate.cols <= anchorSize) {
        return cv::Mat();
    }

    const int offset = 10;
    const cv::Rect anchorRect(fullTemplate.cols - anchorSize + offset, 0, anchorSize - offset, fullTemplate.rows);
    const cv::Mat anchorTemplate = fullTemplate(anchorRect).clone();
    cv::imwrite("debug_anchor_template.png", anchorTemplate);

    QPoint matchCenter;
    double score = 0.0;
    if (!m_matcher.findTemplate(screenMat, fullTemplate, matchCenter, score, 0.5)) {
        return cv::Mat();
    }

    const int matchLeft = matchCenter.x() - fullTemplate.cols / 2;
    const int matchTop = matchCenter.y() - fullTemplate.rows / 2;
    const int oilWidth = std::min(30, fullTemplate.cols - anchorRect.width);
    const int oilHeight = fullTemplate.rows - 8 * 2;
    const int oilLeft = matchLeft + fullTemplate.cols - anchorRect.width - oilWidth;
    const int oilTop = matchTop + 8;

    const cv::Rect screenBounds(0, 0, screenMat.cols, screenMat.rows);
    const cv::Rect oilRect = cv::Rect(oilLeft, oilTop, oilWidth, oilHeight) & screenBounds;
    if (oilRect.width <= 0 || oilRect.height <= 0) {
        return cv::Mat();
    }

    return screenMat(oilRect).clone();
}

std::optional<int> VisionEngine::readWorldZoneOilCountWithPaddle(const cv::Mat& oilRoi)
{
    if (oilRoi.empty() || !ensurePaddleOcrProcess()) {
        return std::nullopt;
    }

    std::vector<uchar> encodedBytes;
    if (!cv::imencode(".png", oilRoi, encodedBytes)) {
        return std::nullopt;
    }

    const QByteArray imageBytes(reinterpret_cast<const char*>(encodedBytes.data()),
                                static_cast<int>(encodedBytes.size()));
    const QJsonObject requestObject{
        {"image_base64", QString::fromLatin1(imageBytes.toBase64())}
    };
    const QByteArray requestLine = QJsonDocument(requestObject).toJson(QJsonDocument::Compact) + '\n';
    m_paddleOcrProcess->write(requestLine);
    if (!m_paddleOcrProcess->waitForBytesWritten(1000)) {
        ++m_paddleOcrFailureCount;
        qDebug() << "paddle OCR write failed:" << m_paddleOcrProcess->errorString();
        if (m_paddleOcrFailureCount >= kPaddleMaxFailures) {
            m_paddleOcrDisabled = true;
        }
        shutdownPaddleOcrProcess();
        return std::nullopt;
    }

    const QByteArray responseLine = readNextJsonLine(m_paddleOcrProcess, kPaddleResponseTimeoutMs);
    if (responseLine.isEmpty()) {
        ++m_paddleOcrFailureCount;
        qDebug() << "paddle OCR response timeout, stderr="
                 << QString::fromUtf8(m_paddleOcrProcess->readAllStandardError()).trimmed();
        if (m_paddleOcrFailureCount >= kPaddleMaxFailures) {
            m_paddleOcrDisabled = true;
        }
        shutdownPaddleOcrProcess();
        return std::nullopt;
    }

    const QJsonDocument responseDoc = QJsonDocument::fromJson(responseLine);
    if (!responseDoc.isObject()) {
        ++m_paddleOcrFailureCount;
        qDebug() << "invalid paddle OCR response:" << responseLine;
        if (m_paddleOcrFailureCount >= kPaddleMaxFailures) {
            m_paddleOcrDisabled = true;
        }
        shutdownPaddleOcrProcess();
        return std::nullopt;
    }

    const QJsonObject responseObject = responseDoc.object();
    if (responseObject.value("ok").toBool()) {
        const std::optional<int> value = extractDigitsValue(responseObject.value("text").toString());
        if (value.has_value()) {
            m_paddleOcrFailureCount = 0;
            qDebug() << "oil OCR success value=" << *value << "mode=paddleocr";
            return value;
        }
    }

    ++m_paddleOcrFailureCount;
    qDebug() << "paddle OCR returned no digits:" << responseLine;
    if (m_paddleOcrFailureCount >= kPaddleMaxFailures) {
        qDebug() << "disabling paddle OCR after repeated failures";
        m_paddleOcrDisabled = true;
        shutdownPaddleOcrProcess();
    }
    return std::nullopt;
}

std::optional<int> VisionEngine::readWorldZoneOilCountFallback(const cv::Mat& oilRoi)
{
    if (oilRoi.empty()) {
        return std::nullopt;
    }

    const cv::Mat oilBinary = thresholdOilDigits(oilRoi);
    const std::optional<int> oilValue = parseOilDigits(oilBinary);
    if (oilValue.has_value()) {
        qDebug() << "oil OCR success value=" << *oilValue << "mode=local-fallback";
    }
    return oilValue;
}

static int count = 0;
std::optional<int> VisionEngine::readWorldZoneOilCount(const QImage& screen)
{
    cv::Mat screenMat = QImageToMat(screen);
    if (screenMat.empty()) {
        return std::nullopt;
    }

    const cv::Mat oilRoi = locateWorldZoneOilRoi(screenMat);
    if (oilRoi.empty()) {
        return std::nullopt;
    }

    cv::imwrite(QString("debug_oil_roi_%1.png").arg(count++).toStdString(), oilRoi);

    // if (const std::optional<int> paddleValue = readWorldZoneOilCountWithPaddle(oilRoi)) {
    //     return paddleValue;
    // }

    // return readWorldZoneOilCountFallback(oilRoi);
    return readWorldZoneOilCountWithPaddle(oilRoi);
}