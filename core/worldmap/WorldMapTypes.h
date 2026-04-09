#ifndef WORLDMAPTYPES_H
#define WORLDMAPTYPES_H

#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QSize>
#include <QSizeF>
#include <QString>
#include <QStringList>
#include <QTransform>
#include <QVector>

enum class WorldZoneType {
    Dangerous,  //危险海域
    Safe,       //安全海域
    Obscure,    //隐秘海域
    Abyssal,    //深渊海域
    Stronghold, //塞壬要塞
    Archive     //档案坐标

};

enum class WorldMapPage {
    Unknown,
    WorldMap
};

struct WorldZoneMetadata
{
    int zoneId = -1;
    QString key;
    QString displayName;
    QStringList aliases;

    QPointF worldAnchor; //世界坐标系中的锚点位置（通常是该区域的中心或一个显著位置）
    QPointF missionAnchor; //任务坐标系中的锚点位置（通常是该区域在游戏界面上的位置）

    int regionId = 0; //所属大区（如：1-1、2-4等中的数字部分）
    int hazardLevel = 0; //危险等级 (0-5，0表示安全海域，5表示极危险海域)
    bool isPort = false; //是否为港口（如：赤城、加贺、天城等）
    bool isAzurPort = false; //是否为碧蓝航线专用港口（如：赤城、加贺、天城等）

    QVector<WorldZoneType> defaultTypes; //默认类型列表（一个区域可能同时具有多个类型，如：既是危险海域又是隐秘海域）
    QVector<int> neighborZoneIds; //相邻区域的ID列表（用于导航和路径规划）
};

struct WorldMapGotoRequest
{
    int targetZoneId = -1;
    QVector<WorldZoneType> preferredTypes; //优先考虑的区域类型（如果目标区域具有这些类型中的任意一个，则优先选择该区域）
    bool refreshCurrentZone = false; //是否强制刷新当前区域信息（即使当前区域已知且安全）
    bool stopIfAlreadySafe = false; //如果目标区域已知且安全，是否停止导航（避免不必要的移动）
    bool allowNeighborUnlockFallback = true; //是否允许在邻近区域解锁失败时回退
};

struct WorldMapCalibration
{
    QSize screenSize = QSize(1280, 720);
    QRect viewport = QRect(QPoint(0, 0), screenSize);
    QSizeF referenceMapSize = QSizeF(2570.0, 1696.0); //世界地图参考尺寸（通常是根据游戏中世界地图的实际像素尺寸设定的，用于坐标转换）
    QPointF fixedScreenCenter = QPointF(508.0, 423.0); //固定屏幕中心点（根据游戏界面设计，通常是世界地图的中心位置，用于坐标转换和导航）
};

struct WorldMapTransformSnapshot
{
    bool valid = false;      //当前变换快照是否有效（即是否成功捕获了当前的世界地图状态）
    double confidence = 0.0; //变换快照的置信度（用于评估当前快照的可靠性）

    QRect viewport;          //当前视口（屏幕坐标系中的矩形区域，表示当前世界地图在屏幕上的显示区域）
    QSizeF referenceMapSize; //当前参考地图尺寸（世界坐标系中的尺寸，表示当前世界地图的实际大小，用于坐标转换）

    QPointF worldCenter;     //当前世界中心点（世界坐标系中的点，表示当前世界地图的中心位置，用于坐标转换和导航）
    QPointF screenCenter;    //当前屏幕中心点（屏幕坐标系中的点，表示当前世界地图在屏幕上的中心位置，用于坐标转换和导航）
    WorldMapPage page = WorldMapPage::Unknown;

    QTransform worldToScreen;
    QTransform screenToWorld;
};

#endif