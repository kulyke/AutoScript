#ifndef WORLDMAPTRANSFORM_H
#define WORLDMAPTRANSFORM_H

#include "WorldMapTypes.h"

#include <QImage>
#include <QPoint>

class VisionEngine;

class WorldMapTransform
{
public:
    explicit WorldMapTransform(VisionEngine* vision = nullptr);

    void setVisionEngine(VisionEngine* vision);
    void setCalibration(const WorldMapCalibration& calibration);

    WorldMapCalibration calibration() const;
    bool hasWorldCenter() const;
    QPointF worldCenter() const;
    void setWorldCenter(const QPointF& worldCenter);
    void clearWorldCenter();

    bool update(const QImage& frame, QString* error = nullptr);
    bool updateFromKnownCenter(const QImage& frame,
                               const QPointF& worldCenter,
                               QString* error = nullptr);

    bool isValid() const;
    WorldMapTransformSnapshot snapshot() const;

    QPointF worldToScreen(const QPointF& worldPoint) const;
    QPointF screenToWorld(const QPointF& screenPoint) const;

    bool isWorldPointVisible(const QPointF& worldPoint, const QRect& sightArea) const;
    /**
     * @brief 计算一个适合点击的屏幕坐标点，以便对准指定的世界区域进行点击操作
      * @param zone 目标世界区域的元数据，包含该区域在世界坐标系中的位置和尺寸等信息
      * @return 计算得到的屏幕坐标点，表示在当前世界地图视图中适合点击的位置，通常是该区域在屏幕上的中心点或一个合理的偏移位置，以确保点击能够成功选中该区域
     */
    QPoint computeTapPointForZone(const WorldZoneMetadata& zone) const;
    /**
     * @brief 计算一个适合滑动的屏幕坐标偏移向量，以便将当前视图对准指定的世界坐标点进行导航操作
      * @param worldPoint 目标世界坐标点，表示希望对准的世界区域在世界坐标系中的位置
      * @param swipeLimit 滑动限制，表示每次滑动的最大屏幕坐标偏移量，过大可能导致滑动过度，过小则可能需要多次滑动才能完成对焦。这个值需要根据游戏界面的设计和实际测试进行调整，通常设置在200-300像素之间比较合理。
      * @return 计算得到的屏幕坐标偏移向量，表示从当前视图中心到目标世界坐标点在屏幕上的偏移量。这个向量可以直接用于执行滑动操作，以将视图对准目标区域。
     */
    QPointF computeSwipeVectorToward(const QPointF& worldPoint,
                                     const QSizeF& swipeLimit) const;

private:
    WorldMapPage detectPage(const QImage& frame) const;
    bool rebuildSnapshot(const QPointF& worldCenter,
                         WorldMapPage page,
                         QString* error = nullptr);

    VisionEngine* m_vision = nullptr;
    WorldMapCalibration m_calibration;
    WorldMapTransformSnapshot m_snapshot;
    QPointF m_worldCenter;
    bool m_hasWorldCenter = false;
};

#endif