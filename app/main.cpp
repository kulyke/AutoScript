#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include "MainWindow.h"

#include <iostream>
#include <QDebug>

static QFile logFile;
static QTextStream logStream;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString levelStr;
    switch (type) {
    case QtDebugMsg:    levelStr = "[DEBUG]"; break;
    case QtInfoMsg:     levelStr = "[INFO]"; break;
    case QtWarningMsg:  levelStr = "[WARN]"; break;
    case QtCriticalMsg: levelStr = "[CRIT]"; break;
    case QtFatalMsg:    levelStr = "[FATAL]"; break;
    }

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString logMsg = QString("%1 %2 %3").arg(timestamp).arg(levelStr).arg(msg);

    if (logStream.device()) {
        logStream << logMsg << "\n";
        logStream.flush();
    }
    std::cerr << logMsg.toStdString() << std::endl;
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 打开日志文件
    logFile.setFileName("debug.log");
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
        logStream.setDevice(&logFile);
        qInstallMessageHandler(messageHandler);
        qDebug() << "=== Application Started ===";
    }

    MainWindow w;
    w.show();

    return a.exec();
}