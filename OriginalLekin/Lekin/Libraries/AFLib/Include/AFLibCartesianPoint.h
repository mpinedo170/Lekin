#pragma once

/////////////////////////////////////////////////////////////////////////////
// TCartesianPoint
// 2-dimensional point with floating-point coordinates

namespace AFLibMath {
class AFLIB TCartesianPoint
{
public:
    double m_x;  // x-coordinate
    double m_y;  // y-coordinate

    TCartesianPoint();
    TCartesianPoint(double x, double y);
    TCartesianPoint(const CPoint& pt);

    // get distance from "this" to a given point
    double GetDistanceToPt(const TCartesianPoint& cpt);

    // get distance from "this" to a given point, update maxDist
    void GetDistanceToPt(const TCartesianPoint& cpt, double& maxDist);

    // get distance from "this" to a given interval or line
    double GetDistanceToRay(
        const TCartesianPoint& cptBegin, const TCartesianPoint& cptEnd, bool bIntervalOnly);

    // get distance from "this" to a given interval or line, update maxDist
    void GetDistanceToRay(
        const TCartesianPoint& cptBegin, const TCartesianPoint& cptEnd, bool bIntervalOnly, double& maxDist);
};
}  // namespace AFLibMath

/////////////////////////////////////////////////////////////////////////////
// TCartesianPointArray
// Array of TCartesianPoint

namespace AFLibMath {
class TCartesianPointArray : public CArray<TCartesianPoint>
{};
}  // namespace AFLibMath
