#ifndef WORLDOCEANPLANBATTLERUNTIMECONTEXT_H
#define WORLDOCEANPLANBATTLERUNTIMECONTEXT_H

struct WorldOceanPlanBattleRuntimeContext
{
    int oilRecoveryCount = 0; // 已执行的油量恢复次数
    int supplyBoxPurchaseCount = 0; // 已购买的补给箱数量
    bool supplyBoxDepleted = false; // 是否已检测到补给箱耗尽（无法继续购买）
    int lastObservedOil = -1; // 最近一次OCR识别到的石油数，-1表示尚未成功识别
};

#endif