#include "CloudPoint.h"

/**
 * Creates a black point at the origine.
 */
CloudPoint::CloudPoint() :
  CloudPoint(0, 0, 0, 0, 0, 0)
{
}

/**
 * Color components are expected in the range [0, 255].
 *
 * @param _x Value of the x coordinate.
 * @param _y Value of the y coordinate.
 * @param _z Value of the z coordinate.
 * @param _r Value of the red color component.
 * @param _g Value of the green color component.
 * @param _b Value of the blue color component.
 */
CloudPoint::CloudPoint(float _x, float _y, float _z, int _r, int _g,  int _b) :
  Point(_x, _y, _z),
  m_r(_r), m_g(_g), m_b(_b),
  m_id(s_nextId)
{
  s_nextId++;
}

CloudPoint::~CloudPoint()
{
}

/**
 * Color components should be in the range [0, 255].
 *
 * @return true if the data is valid, false otherwise.
 */
bool CloudPoint::isValid() {
  return ( (0 <= m_r && m_r < 256) &&
	   (0 <= m_g && m_g < 256) &&
	   (0 <= m_b && m_b < 256) );
}

int CloudPoint::s_nextId = 2001;

/**
 * Expected input format: X Y Z R G B
 *
 * @param in Input stream to read from.
 * @param cp Cloud point to read into.
 * @return The input stream.
 */
std::istream& operator>>(std::istream& in, CloudPoint& cp)
{
  in >> cp.m_x >> cp.m_y >> cp.m_z >> cp.m_r >> cp.m_g >> cp.m_b;
  return in;
}

/**
 * @param out Output stream to write to.
 * @param cp Cloud point to read from.
 * @return The output stream.
 */
std::ostream& operator<<(std::ostream& out, const CloudPoint& cp)
{
  out << "(" << cp.m_x << ", " << cp.m_y << ", " << cp.m_z << ") - ["
      << cp.m_r << ", " << cp.m_g << ", " << cp.m_b << "]";
  return out;
}
