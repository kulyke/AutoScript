#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QString>
#include <memory>

#include "screencapture.h"
#include "devicecontroller.h"
#include "visionengine.h"
#include "taskmanager.h"
#include "../config/AdbConfig.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QTreeWidget;
class QLabel;
class QTextEdit;
class QTableWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void createUI();

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onStart();
    void onStop();
    void appendLog(const QString& msg);
    void onTaskStatusChanged(const QString& taskId,
                             const QString& taskName,
                             const QString& statusText,
                             const QString& stateName);
    void onTaskFinished(const QString& taskId,
                        const QString& taskName,
                        const QString& finalStatus);

private:
    void refreshEmulatorView();
    QString createTaskId(const QString& taskTypeName) const;
    int findTaskStatusRow(const QString& taskId) const;
    void setTaskStatusRow(const QString& taskId,
                          const QString& statusText,
                          const QString& stateName);
    void removeTaskStatusRow(const QString& taskId);

private:
    Ui::MainWindow *ui;

    QTreeWidget* m_taskTree;
    QLabel* m_emulatorView;
    QTableWidget* m_taskStatus;
    QTextEdit* m_logEdit;

private:
    ScreenCapture* m_capture;
    DeviceController* m_device;
    VisionEngine* m_vision;
    TaskManager* m_taskManager;
    
    std::shared_ptr<AdbConfig> m_adbConfig;
    QThread* m_taskThread;//任务线程，承载截图、视觉和任务调度，避免阻塞 UI
    
    //当前屏幕截图
    QImage m_currentFrame;
};

#endif