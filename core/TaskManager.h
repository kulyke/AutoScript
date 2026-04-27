#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
#include <QElapsedTimer>
#include <QImage>
#include <QList>
#include <QMutex>
#include <QQueue>

class TaskBase;
/**
 * @brief 任务管理器，负责管理和调度所有任务
 * 
 */
class TaskManager : public QObject
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = nullptr);
    ~TaskManager() override;
    /**
     * @brief 添加任务
     * @param task 任务指针
     */
    void addTask(TaskBase* task);
    /**
     * @brief 移除任务
     * @param task 任务指针
     */
    void removeTask(TaskBase* task);
    /**
    * @brief 通过任务实例ID移除任务
    * @param taskId 任务实例ID
     */
    void removeTaskById(const QString& taskId);
    /**
     * @brief 获取所有任务
     * @return 任务列表
     */
    QList<TaskBase*> tasks() const { return m_tasks; }
    /**
     * @brief 获取当前正在执行的任务
     * @return 当前任务指针
     */
    TaskBase* currentTask() const;
    /**
     * @brief 启动所有任务
     */
    void start();
    /**
     * @brief 停止所有任务
     */
    void stop();
    /**
     * @brief 关闭任务管理器并清空所有任务
     */
    void shutdown();

public slots:
    void onFrameReady(const QImage& frame);

private slots:
    void processPendingFrame();

signals:
    void logMessage(const QString& msg);
    void taskStatusChanged(const QString& taskId,
                           const QString& taskName,
                           const QString& statusText,
                           const QString& stateName);
    void taskFinished(const QString& taskId,
                      const QString& taskName,
                      const QString& finalStatus);

private:
    void processFrame(const QImage& frame);

    QQueue<TaskBase*> m_tasks;//任务队列，按照优先级排序
    bool m_running = false;
    bool m_processingFrame = false; //是否正在处理当前帧，防止重入
    const int m_minFrameIntervalMs = 40; //最小帧间隔，单位毫秒；在截图独立线程下允许更快驱动状态机
    QElapsedTimer m_frameTimer; //帧计时器，用于控制帧率

    QMutex m_frameMutex; //保护以下成员变量，防止多线程访问冲突
    QImage m_pendingFrame; //当前待处理的帧，onFrameReady中更新，processPendingFrame中读取并清空
    bool m_hasPendingFrame = false; //是否有待处理的帧
    bool m_processScheduled = false; //是否已计划调用processPendingFrame，防止重复调用

};

#endif