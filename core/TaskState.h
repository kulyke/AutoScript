#ifndef TASKSTATE_H
#define TASKSTATE_H

#include <QImage>

class TaskState : public QObject
{
    Q_OBJECT
public:
    TaskState(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~TaskState() {}

    virtual QString name() const = 0;
    /**
     * @brief 更新任务状态
     * @param frame 当前屏幕截图
     * @return 下一个任务状态
     */
    virtual TaskState* update(const QImage& frame) = 0;

};

#endif