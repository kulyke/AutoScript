#ifndef WORLDOCEANSTEPS_H
#define WORLDOCEANSTEPS_H

#include "../../core/FlowStep.h"

#include <QElapsedTimer>
#include <QString>

class VisionEngine;
class DeviceController;
struct OilRefillDialogInfo;
struct WorldOceanPlanBattleRuntimeContext;

/**
 * @brief 购买补给箱步骤
 */
class PurchaseEnergySupplyBoxStep : public FlowStep
{
public:
    PurchaseEnergySupplyBoxStep(VisionEngine* vision,
                                DeviceController* device,
                                WorldOceanPlanBattleRuntimeContext* runtimeContext,
                                QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    VisionEngine* m_vision;
    DeviceController* m_device;
    WorldOceanPlanBattleRuntimeContext* m_runtimeContext;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
    bool m_waitingConfirm = false;
};

class UseOilRefillSuppliesStep : public FlowStep
{
public:
    enum class SupplyType {
        None,
        Blue,
        Purple,
        Yellow,
    };

    enum class Stage {
        AnalyzeDialog,
        WaitAfterSelect,
        WaitAfterUse,
    };

    UseOilRefillSuppliesStep(VisionEngine* vision,
                             DeviceController* device,
                             WorldOceanPlanBattleRuntimeContext* runtimeContext,
                             QString stepName);

    QString name() const override;
    FlowStepStatus execute(const QImage& frame) override;
    void reset() override;
    QString takeRuntimeMessage() override;
    QString errorString() const override;

private:
    /**
     * @brief 根据对话框信息和当前油量决定是否点击补给数量以及准备后续使用
     */
    FlowStepStatus tapSupplyAndPrepareToUse(const OilRefillDialogInfo& dialogInfo, SupplyType supplyType);
    /**
     * @brief 点击补给使用按钮
      * @param dialogInfo 油量补给对话框信息
      * @return 点击是否成功
     */
    FlowStepStatus tapUseButton(const OilRefillDialogInfo& dialogInfo);

    VisionEngine* m_vision;
    DeviceController* m_device;
    WorldOceanPlanBattleRuntimeContext* m_runtimeContext;
    QString m_name;
    QString m_error;
    QString m_runtimeMessage;
    QElapsedTimer m_stageTimer; // 用于阶段间的等待计时
    Stage m_stage = Stage::AnalyzeDialog; // 当前步骤执行阶段
    SupplyType m_pendingSupplyType = SupplyType::None; // 当前正在使用的补给类型
    int m_initialBlueSupplyCount = -1;
};

#endif