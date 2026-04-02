#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QObject>
// #include <QList>
#include <QQueue>
#include <QImage>
#include <QElapsedTimer>

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
     * @brief 通过任务名称移除任务
     * @param taskName 任务名称
     */
    void removeTaskByName(const QString& taskName);
    /**
     * @brief 获取所有任务
     * @return 任务列表
     */
    QList<TaskBase*> tasks() const { return m_tasks; }
    /**
     * @brief 设置当前任务
     * @param task 任务指针
     */
    void setCurrentTask(TaskBase* task) { m_currentTask = task; }
    /**
     * @brief 获取当前正在执行的任务
     * @return 当前任务指针
     */
    TaskBase* currentTask() const { return m_currentTask; }
    /**
     * @brief 启动所有任务
     */
    void start();
    /**
     * @brief 停止所有任务
     */
    void stop();

public slots:
    void onFrameReady(const QImage& frame);

signals:
    void logMessage(const QString& msg);
    void taskStatusChanged(const QString& taskName,
                           const QString& statusText,
                           const QString& stateName);
    void taskFinished(const QString& taskName,
                      const QString& finalStatus);

private:
    QList<TaskBase*> m_idleTasks;//空闲任务列表
    // QList<TaskBase*> m_tasks;//正在执行&等待的任务列表
    QQueue<TaskBase*> m_tasks;//任务队列，按照优先级排序
    bool m_running = false;
    bool m_processingFrame = false; //是否正在处理当前帧，防止重入
    int m_minFrameIntervalMs = 120; //最小帧间隔，单位毫秒，过快时会丢帧以降低状态机驱动频率
    QElapsedTimer m_frameTimer; //帧计时器，用于控制帧率
    TaskBase* m_currentTask = nullptr; //当前正在执行的任务

};

#endif