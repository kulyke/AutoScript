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

// 构建一个相对于图像尺寸的矩形，参数中的比例值应该在0到1之间
cv::Rect buildRelativeRect(const cv::Size& size,
                          double xRatio,
                          double yRatio,
                          double widthRatio,
                          double heightRatio)
{
    const int x = std::clamp(static_cast<int>(std::floor(size.width * xRatio)), 0, std::max(0, size.width - 1));
    const int y = std::clamp(static_cast<int>(std::floor(size.height * yRatio)), 0, std::max(0, size.height - 1));
    const int width = std::clamp(static_cast<int>(std::ceil(size.width * widthRatio)), 1, size.width - x);
    const int height = std::clamp(static_cast<int>(std::ceil(size.height * heightRatio)), 1, size.height - y);
    return cv::Rect(x, y, width, height);
}

std::optional<cv::Rect> locateTemplateByAnchor(TemplateMatcher& matcher,
                                               const cv::Mat& screenMat,
                                               const cv::Mat& fullTemplate,
                                               const cv::Rect& anchorRect,
                                               double threshold)
{
    if (screenMat.empty() || fullTemplate.empty()) {
        return std::nullopt;
    }
    if (anchorRect.width <= 0 || anchorRect.height <= 0) {
        return std::nullopt;
    }
    if (anchorRect.x < 0 || anchorRect.y < 0
        || anchorRect.x + anchorRect.width > fullTemplate.cols
        || anchorRect.y + anchorRect.height > fullTemplate.rows) {
        return std::nullopt;
    }

    const cv::Mat anchorTemplate = fullTemplate(anchorRect).clone();
    QPoint matchCenter;
    double score = 0.0;
    if (!matcher.findTemplate(screenMat, anchorTemplate, matchCenter, score, threshold)) {
        return std::nullopt;
    }

    const int fullLeft = matchCenter.x() - anchorTemplate.cols / 2 - anchorRect.x;
    const int fullTop = matchCenter.y() - anchorTemplate.rows / 2 - anchorRect.y;
    const cv::Rect fullRect(fullLeft, fullTop, fullTemplate.cols, fullTemplate.rows);
    const cv::Rect screenBounds(0, 0, screenMat.cols, screenMat.rows);
    if ((fullRect & screenBounds) != fullRect) {
        return std::nullopt;
    }

    return fullRect;
}

cv::Mat cropScreenRect(const cv::Mat& screenMat, const cv::Rect& rect)
{
    if (screenMat.empty()) {
        return cv::Mat();
    }

    const cv::Rect screenBounds(0, 0, screenMat.cols, screenMat.rows);
    const cv::Rect boundedRect = rect & screenBounds;
    if (boundedRect.width <= 0 || boundedRect.height <= 0) {
        return cv::Mat();
    }

    return screenMat(boundedRect).clone();
}

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

std::vector<cv::Rect> filterDigitBoundsForOcr(const std::vector<cv::Rect>& boxes, const cv::Size& imageSize);
std::vector<cv::Rect> mergeSplitDigitBounds(const std::vector<cv::Rect>& boxes, const cv::Size& imageSize);
bool looksLikeSplitZero(const cv::Mat& digit);

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

    if (looksLikeSplitZero(digit)) {
        return 0;
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
    const std::vector<cv::Rect> rawBoxes = findDigitBounds(digitBinary);
    const std::vector<cv::Rect> filteredBoxes = filterDigitBoundsForOcr(rawBoxes, digitBinary.size());
    const std::vector<cv::Rect> boxes = mergeSplitDigitBounds(filteredBoxes, digitBinary.size());
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

int decimalDigitCount(int value)
{
    int count = 1;
    while (value >= 10) {
        value /= 10;
        ++count;
    }
    return count;
}

bool looksLikeLeftEdgeArtifact(const cv::Rect& box, const cv::Size& imageSize)
{
    if (imageSize.width <= 0 || imageSize.height <= 0) {
        return false;
    }

    const int leftEdgeLimit = std::max(2, imageSize.width / 10);
    const int narrowWidthLimit = std::max(3, imageSize.width / 7);
    const int tallHeightLimit = std::max(6, imageSize.height / 2);
    return box.x <= leftEdgeLimit
        && box.width <= narrowWidthLimit
        && box.height >= tallHeightLimit
        && box.height > box.width * 2;
}

std::vector<cv::Rect> filterDigitBoundsForOcr(const std::vector<cv::Rect>& boxes, const cv::Size& imageSize)
{
    std::vector<cv::Rect> filteredBoxes;
    filteredBoxes.reserve(boxes.size());
    for (const cv::Rect& box : boxes) {
        if (looksLikeLeftEdgeArtifact(box, imageSize)) {
            continue;
        }
        filteredBoxes.push_back(box);
    }

    return filteredBoxes.empty() ? boxes : filteredBoxes;
}

bool shouldMergeSplitDigitPair(const cv::Rect& leftBox, const cv::Rect& rightBox, const cv::Size& imageSize)
{
    if (imageSize.width <= 0 || imageSize.height <= 0) {
        return false;
    }

    const int gap = rightBox.x - (leftBox.x + leftBox.width);
    if (gap < 0) {
        return false;
    }

    const int maxGap = std::max(2, imageSize.width / 18);
    const int maxTopOffset = std::max(2, imageSize.height / 8);
    const int maxBottomOffset = std::max(2, imageSize.height / 8);
    const int narrowWidthLimit = std::max(3, imageSize.width / 4);
    const int tallHeightLimit = std::max(8, (imageSize.height * 3) / 5);

    const int leftBottom = leftBox.y + leftBox.height;
    const int rightBottom = rightBox.y + rightBox.height;
    const int mergedWidth = rightBox.x + rightBox.width - leftBox.x;
    const int maxMergedWidth = std::max(8, (imageSize.width * 3) / 5);

    return gap <= maxGap
        && std::abs(leftBox.y - rightBox.y) <= maxTopOffset
        && std::abs(leftBottom - rightBottom) <= maxBottomOffset
        && leftBox.width <= narrowWidthLimit
        && rightBox.width <= narrowWidthLimit
        && leftBox.height >= tallHeightLimit
        && rightBox.height >= tallHeightLimit
        && mergedWidth <= maxMergedWidth;
}

std::vector<cv::Rect> mergeSplitDigitBounds(const std::vector<cv::Rect>& boxes, const cv::Size& imageSize)
{
    if (boxes.size() < 2) {
        return boxes;
    }

    std::vector<cv::Rect> mergedBoxes;
    mergedBoxes.reserve(boxes.size());
    for (size_t index = 0; index < boxes.size(); ++index) {
        cv::Rect current = boxes[index];
        if (index + 1 < boxes.size() && shouldMergeSplitDigitPair(current, boxes[index + 1], imageSize)) {
            current |= boxes[index + 1];
            ++index;
        }
        mergedBoxes.push_back(current);
    }

    return mergedBoxes;
}

bool looksLikeSplitZero(const cv::Mat& digit)
{
    const std::vector<cv::Rect> boxes = findDigitBounds(digit);
    if (boxes.size() != 2) {
        return false;
    }

    const cv::Rect& leftBox = boxes[0];
    const cv::Rect& rightBox = boxes[1];
    if (!shouldMergeSplitDigitPair(leftBox, rightBox, digit.size())) {
        return false;
    }

    const int gap = rightBox.x - (leftBox.x + leftBox.width);
    const int mergedWidth = rightBox.x + rightBox.width - leftBox.x;
    return gap >= 1 && mergedWidth >= std::max(6, digit.cols / 3);
}

struct PreparedDigitOcrInput
{
    cv::Mat image;
    int expectedDigitCount = 0;
};

PreparedDigitOcrInput prepareDigitOcrInputForPaddle(const cv::Mat& digitRoi)
{
    PreparedDigitOcrInput prepared;
    if (digitRoi.empty()) {
        return prepared;
    }

    const cv::Mat digitBinary = thresholdOilDigits(digitRoi);
    if (digitBinary.empty()) {
        prepared.image = digitRoi.clone();
        return prepared;
    }

    const std::vector<cv::Rect> rawBoxes = findDigitBounds(digitBinary);
    const std::vector<cv::Rect> filteredBoxes = filterDigitBoundsForOcr(rawBoxes, digitBinary.size());
    const std::vector<cv::Rect> digitBoxes = mergeSplitDigitBounds(filteredBoxes, digitBinary.size());
    if (digitBoxes.empty()) {
        prepared.image = digitRoi.clone();
        return prepared;
    }

    cv::Rect unionRect = digitBoxes.front();
    for (size_t index = 1; index < digitBoxes.size(); ++index) {
        unionRect |= digitBoxes[index];
    }

    const int padX = std::max(2, digitBinary.cols / 24);
    const int padY = std::max(2, digitBinary.rows / 12);
    const cv::Rect imageBounds(0, 0, digitBinary.cols, digitBinary.rows);
    unionRect.x = std::max(0, unionRect.x - padX);
    unionRect.y = std::max(0, unionRect.y - padY);
    unionRect.width = std::min(imageBounds.width - unionRect.x, unionRect.width + padX * 2);
    unionRect.height = std::min(imageBounds.height - unionRect.y, unionRect.height + padY * 2);

    cv::Mat croppedBinary = digitBinary(unionRect).clone();
    cv::copyMakeBorder(croppedBinary,
                       croppedBinary,
                       padY,
                       padY,
                       padX,
                       padX,
                       cv::BORDER_CONSTANT,
                       cv::Scalar(0));
    cv::resize(croppedBinary,
               croppedBinary,
               cv::Size(),
               3.0,
               3.0,
               cv::INTER_NEAREST);
    cv::cvtColor(croppedBinary, prepared.image, cv::COLOR_GRAY2BGR);
    prepared.expectedDigitCount = static_cast<int>(digitBoxes.size());
    return prepared;
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

void VisionEngine::warmUpPaddleOcr()
{
    if (m_paddleOcrDisabled) {
        return;
    }

    if (m_paddleOcrProcess && m_paddleOcrProcess->state() != QProcess::NotRunning) {
        return;
    }

    qDebug() << "prewarming paddle OCR process";
    ensurePaddleOcrProcess();
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

    cv::imwrite(QString("debug_%1_%2.png").arg("worldZoneOil").arg(count++).toStdString(), oilRoi);

    return readDigitsFromRoi(oilRoi, "worldZoneOil");
}

std::optional<OilRefillDialogInfo> VisionEngine::readOilRefillDialogInfo(const QImage& screen)
{
    cv::Mat screenMat = QImageToMat(screen);
    if (screenMat.empty()) {
        return std::nullopt;
    }

    OilRefillDialogInfo dialogInfo;
    if (!findTemplate(screen,
                      "worldZone.oil.refill.confirm.button",
                      dialogInfo.confirmButtonPoint,
                      -1.0)) {
        return std::nullopt;
    }
    if (!findTemplate(screen,
                      "worldZone.oil.refill.cancel.button",
                      dialogInfo.cancelButtonPoint,
                      -1.0)) {
        return std::nullopt;
    }

    // 解析当前油量和补给数量
    auto readSupplyEntry = [this, &screenMat](const QString& templateKey,
                                              double anchorWidthRatio,
                                              double anchorHeightRatio,
                                              double digitsXRatio,
                                              double digitsYRatio,
                                              double digitsWidthRatio,
                                              double digitsHeightRatio,
                                              int& outCount,
                                              QPoint& outPoint,
                                              const QString& logContext) -> bool {
        QString resolvedPath;
        double defaultThreshold = 0.9;
        const cv::Mat fullTemplate = loadTemplate(templateKey, resolvedPath, defaultThreshold);
        if (fullTemplate.empty()) {
            return false;
        }

        const cv::Rect anchorRect = buildRelativeRect(fullTemplate.size(), 0.0, 0.0, anchorWidthRatio, anchorHeightRatio);
        const std::optional<cv::Rect> fullRect = locateTemplateByAnchor(m_matcher,
                                                                        screenMat,
                                                                        fullTemplate,
                                                                        anchorRect,
                                                                        defaultThreshold);
        if (!fullRect.has_value()) {
            return false;
        }

        outPoint = QPoint(fullRect->x + fullRect->width / 2,
                          fullRect->y + fullRect->height / 2);
        const cv::Rect digitsRect = buildRelativeRect(fullTemplate.size(),
                                                      digitsXRatio,
                                                      digitsYRatio,
                                                      digitsWidthRatio,
                                                      digitsHeightRatio);
        const cv::Mat digitsRoi = cropScreenRect(screenMat,
                                                 cv::Rect(fullRect->x + digitsRect.x,
                                                          fullRect->y + digitsRect.y,
                                                          digitsRect.width,
                                                          digitsRect.height));
        static int count = 0;
        cv::imwrite(QString("debug_%1_%2.png").arg(logContext).arg(count++).toStdString(), digitsRoi);
        const std::optional<int> digits = readDigitsFromRoi(digitsRoi, logContext);
        if (!digits.has_value()) {
            return false;
        }

        outCount = *digits;
        return true;
    };

    QPoint currentOilAnchorPoint;
    if (!readSupplyEntry("worldZone.oil.refill.currentOil",
                         0.72,
                         1.0,
                         0.72,
                         0.0,
                         0.28,
                         1.0,
                         dialogInfo.currentOil,
                         currentOilAnchorPoint,
                         "oilRefillCurrentOil")) {
        return std::nullopt;
    }

    // Supply counts can be three digits on real accounts, so keep more room on the left edge
    // than the template preview suggests. The previous ROI could clip the leading digit.
    constexpr double kSupplyDigitsXRatio = 0.62;
    constexpr double kSupplyDigitsYRatio = 0.74;
    constexpr double kSupplyDigitsWidthRatio = 0.38;
    constexpr double kSupplyDigitsHeightRatio = 0.26;

    if (!readSupplyEntry("worldZone.oil.refill.supply.blue",
                         0.72,
                         0.78,
                         kSupplyDigitsXRatio,
                         kSupplyDigitsYRatio,
                         kSupplyDigitsWidthRatio,
                         kSupplyDigitsHeightRatio,
                         dialogInfo.blueSupplyCount,
                         dialogInfo.blueSupplyPoint,
                         "oilRefillBlueCount")) {
        return std::nullopt;
    }

    if (!readSupplyEntry("worldZone.oil.refill.supply.purple",
                         0.72,
                         0.78,
                         kSupplyDigitsXRatio,
                         kSupplyDigitsYRatio,
                         kSupplyDigitsWidthRatio,
                         kSupplyDigitsHeightRatio,
                         dialogInfo.purpleSupplyCount,
                         dialogInfo.purpleSupplyPoint,
                         "oilRefillPurpleCount")) {
        return std::nullopt;
    }

    if (!readSupplyEntry("worldZone.oil.refill.supply.yellow",
                         0.72,
                         0.78,
                         kSupplyDigitsXRatio,
                         kSupplyDigitsYRatio,
                         kSupplyDigitsWidthRatio,
                         kSupplyDigitsHeightRatio,
                         dialogInfo.yellowSupplyCount,
                         dialogInfo.yellowSupplyPoint,
                         "oilRefillYellowCount")) {
        return std::nullopt;
    }

    qDebug() << "oil refill dialog state currentOil=" << dialogInfo.currentOil
             << "blue=" << dialogInfo.blueSupplyCount
             << "purple=" << dialogInfo.purpleSupplyCount
             << "yellow=" << dialogInfo.yellowSupplyCount;
    return dialogInfo;
}

std::optional<int> VisionEngine::readDigitsWithPaddle(const cv::Mat& digitRoi, const QString& logContext)
{
    if (digitRoi.empty() || !ensurePaddleOcrProcess()) {
        return std::nullopt;
    }

    const PreparedDigitOcrInput preparedInput = prepareDigitOcrInputForPaddle(digitRoi);
    const cv::Mat& ocrInput = preparedInput.image.empty() ? digitRoi : preparedInput.image;

    std::vector<uchar> encodedBytes;
    if (!cv::imencode(".png", ocrInput, encodedBytes)) {
        return std::nullopt;
    }
    cv::imwrite(QString("debug_%1_input.png").arg(logContext).toStdString(), ocrInput);

    const QByteArray imageBytes(reinterpret_cast<const char*>(encodedBytes.data()),
                                static_cast<int>(encodedBytes.size()));
    const QJsonObject requestObject{
        {"image_base64", QString::fromLatin1(imageBytes.toBase64())}
    };
    const QByteArray requestLine = QJsonDocument(requestObject).toJson(QJsonDocument::Compact) + '\n';
    m_paddleOcrProcess->write(requestLine);
    if (!m_paddleOcrProcess->waitForBytesWritten(1000)) {
        ++m_paddleOcrFailureCount;
        qDebug() << logContext << "paddle OCR write failed:" << m_paddleOcrProcess->errorString();
        if (m_paddleOcrFailureCount >= kPaddleMaxFailures) {
            m_paddleOcrDisabled = true;
        }
        shutdownPaddleOcrProcess();
        return std::nullopt;
    }

    const QByteArray responseLine = readNextJsonLine(m_paddleOcrProcess, kPaddleResponseTimeoutMs);
    if (responseLine.isEmpty()) {
        ++m_paddleOcrFailureCount;
        qDebug() << logContext << "paddle OCR response timeout, stderr="
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
        qDebug() << logContext << "invalid paddle OCR response:" << responseLine;
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
            if (preparedInput.expectedDigitCount > 0
                && decimalDigitCount(*value) > preparedInput.expectedDigitCount) {
                qDebug() << logContext
                         << "reject paddle OCR digit count mismatch value=" << *value
                         << "expectedDigits=" << preparedInput.expectedDigitCount;
                return std::nullopt;
            }
            m_paddleOcrFailureCount = 0;
            qDebug() << logContext << "OCR success value=" << *value << "mode=paddleocr";
            return value;
        }
    }

    ++m_paddleOcrFailureCount;
    qDebug() << logContext << "paddle OCR returned no digits:" << responseLine;
    if (m_paddleOcrFailureCount >= kPaddleMaxFailures) {
        qDebug() << "disabling paddle OCR after repeated failures";
        m_paddleOcrDisabled = true;
        shutdownPaddleOcrProcess();
    }
    return std::nullopt;
}

std::optional<int> VisionEngine::readDigitsFallback(const cv::Mat& digitRoi, const QString& logContext)
{
    if (digitRoi.empty()) {
        return std::nullopt;
    }

    const cv::Mat digitBinary = thresholdOilDigits(digitRoi);
    const std::optional<int> value = parseOilDigits(digitBinary);
    if (value.has_value()) {
        qDebug() << logContext << "OCR success value=" << *value << "mode=local-fallback";
    }
    return value;
}

std::optional<int> VisionEngine::readDigitsFromRoi(const cv::Mat& digitRoi, const QString& logContext)
{
    if (const std::optional<int> paddleValue = readDigitsWithPaddle(digitRoi, logContext)) {
        return paddleValue;
    }

    return readDigitsFallback(digitRoi, logContext);
}