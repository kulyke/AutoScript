#ifndef WORLDMAPRUNTIMECONTEXT_H
#define WORLDMAPRUNTIMECONTEXT_H

#include "WorldMapTypes.h"

#include <QString>

struct WorldMapRuntimeContext
{
    WorldMapGotoRequest gotoRequest;

    int currentZoneId = -1;
    int focusedZoneId = -1;
    int lastPinnedZoneId = -1;

    QString currentZoneName;
    QString focusedZoneName;
    QString lastResolutionSource;
    QString lastNavigationAction;

    bool hasTargetZoneRequest() const
    {
        return gotoRequest.targetZoneId >= 0;
    }

    bool hasCurrentZone() const
    {
        return currentZoneId >= 0;
    }

    bool hasFocusedZone() const
    {
        return focusedZoneId >= 0;
    }

    void clearCurrentZone()
    {
        currentZoneId = -1;
        currentZoneName.clear();
        lastResolutionSource.clear();
    }

    void clearFocusedZone()
    {
        focusedZoneId = -1;
        focusedZoneName.clear();
    }

    void clearGotoRequest()
    {
        gotoRequest = WorldMapGotoRequest();
        clearFocusedZone();
        lastNavigationAction.clear();
    }
};

#endif