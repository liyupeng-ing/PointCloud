#ifndef CLOUD_POINT_H
#define CLOUD_POINT_H

#include <fstream>

#include "Point.h"


/**
 * @brief Class representing a 3D point with an associated RGB color triplet.
 */
class CloudPoint : public Point {

public:

  /** Default Constructor. */
  CloudPoint();

  /** Full constructor. */
  CloudPoint(float _x, float _y, float _z, int _r, int _g,  int _b);
  
  /** Destructor. */
  ~CloudPoint();

  /** Checks the validity of the data. */
  bool isValid();
  

  /** Returns the red color component.
   * @return Red color component.
   */
  inline int r() const { return m_r; }

  /** Returns the green color component. 
   * @return Green color component.
   */
  inline int g() const { return m_g; }

  /** Returns the blue color component. 
   * @return Blue color component.
   */
  inline int b() const { return m_b; }


  /** Sets the red color component. 
   * @param _r Red color component.
   */
  inline void setR(int _r) { m_r = _r; }

  /** Sets the green color component. 
   * @param _g Green color component.
   */
  inline void setG(int _g) { m_g = _g; }

  /** Sets the blue color component. 
   * @param _b Blue color component.
   */
  inline void setB(int _b) { m_b = _b; }


  /** Returns a unique identifier of this point. */
  inline int id() { return m_id; }
  
private:
  
  int m_r;
  int m_g;
  int m_b;
  int m_id;
  static int s_nextId;

  /** Reads data from an input stream. */
  friend std::istream& operator>>(std::istream& in, CloudPoint& cp);

  /** Displays data to an output stream. */
  friend std::ostream& operator<<(std::ostream& out, const CloudPoint& cp);
};

#endif
