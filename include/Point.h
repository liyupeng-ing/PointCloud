#ifndef POINT_H
#define POINT_H

/**
 * @brief Class representing a 3D point.
 *
 * Implements cartesian coordinate system where (x,z) 
 * represent the horizontal coordinates and y the vertical coordinate.
 */
class Point {

public:

  /** Default constructor */
  Point();

  /** Full Constructor. */
  Point(float _x, float _y, float _z);

  /** Destructor */
  virtual ~Point();

  
  /** Returns x coordinate 
   * @return x position
   */
  inline float x() const { return m_x; }
  
  /** Returns y coordinate 
   * @return y position
   */
  inline float y() const { return m_y; }
  
  /** Returns z coordinate
   * @return z position
    */
  inline float z() const { return m_z; }

  
  /** Sets x coordinate 
   * @param _x x position
   */
  inline void setX(float _x) { m_x = _x; }
  
  /** Sets y coordinate 
   * @param _y y position
   */
  inline void setY(float _y) { m_y = _y; }
  
  /** Sets z coordinate 
   * @param _z z position
   */
  inline void setZ(float _z) { m_z = _z; }

  /** Returns 2D squared distance in the (x,z) plane */
  float dist2DSq(const Point &p) const;

  /** Returns 3D squared distance in the (x,y,z) space */
  float dist3DSq(const Point &p) const;
  
protected:

  float m_x;
  float m_y;
  float m_z;
};

#endif
