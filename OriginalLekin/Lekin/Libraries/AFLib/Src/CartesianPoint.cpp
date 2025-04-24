#include "StdAfx.h"

#include "AFLibCartesianPoint.h"

#include "AFLibMathGlobal.h"

using AFLibMath::TCartesianPoint;

TCartesianPoint::TCartesianPoint()
{
    m_x = m_y = Nan;
}

TCartesianPoint::TCartesianPoint(double x, double y)
{
    m_x = x;
    m_y = y;
}

TCartesianPoint::TCartesianPoint(const CPoint& pt)
{
    m_x = pt.x;
    m_y = pt.y;
}

double TCartesianPoint::GetDistanceToPt(const TCartesianPoint& cpt)
{
    return sqrt((cpt.m_x - m_x) * (cpt.m_x - m_x) + (cpt.m_y - m_y) * (cpt.m_y - m_y));
}

void TCartesianPoint::GetDistanceToPt(const TCartesianPoint& cpt, double& maxDist)
{
    if (fabs(m_x - cpt.m_x) >= maxDist) return;
    if (fabs(m_y - cpt.m_y) >= maxDist) return;

    double dist = GetDistanceToPt(cpt);
    maxDist = min(dist, maxDist);
}

double TCartesianPoint::GetDistanceToRay(
    const TCartesianPoint& cptBegin, const TCartesianPoint& cptEnd, bool bIntervalOnly)
{
    double dx = cptEnd.m_x - cptBegin.m_x;
    double dy = cptEnd.m_y - cptBegin.m_y;

    double num = dx * (cptBegin.m_x - m_x) + dy * (cptBegin.m_y - m_y);
    double denom = dx * dx + dy * dy;
    double t = denom == 0 ? 0. : double(-num) / denom;

    if (bIntervalOnly) t = max(min(t, 1.), 0.);

    TCartesianPoint cpt(cptBegin.m_x + t * dx, cptBegin.m_y + t * dy);
    return GetDistanceToPt(cpt);
}

void TCartesianPoint::GetDistanceToRay(
    const TCartesianPoint& cptBegin, const TCartesianPoint& cptEnd, bool bIntervalOnly, double& maxDist)
{  // if point is not in the square limited by ptBegin and ptEnd (expanded by maxDist margin), just return
   // maxDist

    if (bIntervalOnly)
    {
        if (m_x - cptBegin.m_x >= maxDist && m_x - cptEnd.m_x >= maxDist) return;
        if (m_x - cptBegin.m_x <= -maxDist && m_x - cptEnd.m_x <= -maxDist) return;

        if (m_y - cptBegin.m_y >= maxDist && m_y - cptEnd.m_y >= maxDist) return;
        if (m_y - cptBegin.m_y <= -maxDist && m_y - cptEnd.m_y <= -maxDist) return;
    }

    double dist = GetDistanceToRay(cptBegin, cptEnd, bIntervalOnly);
    maxDist = min(dist, maxDist);
}
