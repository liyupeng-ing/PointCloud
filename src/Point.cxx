#include "Point.h"

/**
 * Initialize a 3D point at the origin (0, 0, 0)
 */
Point::Point() :
  Point(0, 0, 0)
{
}

/**
 * @param _x Value of the x coordinate.
 * @param _y Value of the y coordinate.
 * @param _z Value of the z coordinate.
 */
Point::Point(float _x, float _y, float _z) :
  m_x(_x),
  m_y(_y),
  m_z(_z)
{
}

Point::~Point()
{
}


/**
 * This function is intended to compare distances in the horizontal plane.
 * It returns the squared distance to avoid the overhead of calling sqrt() unnecessarily.
 *
 * @param p Point with respect to which the distance is calculated.
 * @return Squared distance in the (x,z) plane.
 */
float Point::dist2DSq(const Point &p) const
{
  return (p.m_x-m_x)*(p.m_x-m_x) + (p.m_z-m_z)*(p.m_z-m_z);
}


/**
 * This function is intended to compare distances in 3D.
 * It returns the squared distance to avoid the overhead of calling sqrt() unnecessarily.
 *
 * @param p Point with respect to which the distance is calculated.
 * @return Squared distance in the (x,y,z) space.
 */
float Point::dist3DSq(const Point &p) const
{
  return (p.m_x-m_x)*(p.m_x-m_x) + (p.m_y-m_y)*(p.m_y-m_y) + (p.m_z-m_z)*(p.m_z-m_z);
}

