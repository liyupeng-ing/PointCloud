#ifndef CLUSTER_H
#define CLUSTER_H

#include <vector>

#include "CloudPoint.h"

/**
 * @brief Class describing a cluster of cloud points.
 *
 * This class provides access to individual points within a cluster as well as cluster properties
 * such as a center of mass, a core density measure, a seed position and a class ID. \n
 * These propoerties are calculated by the clustering algorithm.
 */
class Cluster {

public:

  /** Default constructor */
  Cluster();

  /** Copy constructor */
  Cluster(const Cluster &cl);

  /** Destructor */
  ~Cluster();

  
  /** Adds a point to the cluster */
  void addPoint(const CloudPoint &cp);
  
  /** Add points from another cluster */
  void addPoints(const Cluster &cl);
  
  /** Returns a vector of individual points. 
   * @return Points components.
   */
  inline const std::vector<CloudPoint> &points() const { return m_points; }

  
  /** Returns the center of mass 
   * @return Center of mass position.
   */
  inline const Point &com() const { return m_com; }
  
  /** Return layer positions */
  const std::vector<CloudPoint> &layers(int nLayers) const;
  
  /** Returns a vector of sub-clusters of randomly chosen points. */
  std::vector<Cluster> &randomSplit(int nClusters, float fPerCluster) const;


  /** Returns a density measure. 
   * @return density.
   */
  inline float density() const { return m_density; }
  
  /** Sets a density measure. 
   * @param _density Density.
   */
  inline void setDensity(float _density) { m_density = _density; }


  /** Returns a seed position. 
   * @return Seed position.
   */ 
  inline const Point &seed() const { return m_seed; }
  
  /** Sets a seed position. 
   * @param _seed Seed position.
   */
  inline void setSeed(const Point &_seed) { m_seed = _seed; }

  
  /** Returns a PCA color components 
   * @return PCA color components.
   */
  inline const Point &pcaColor() const { return m_pcaColor; }
  
  /** Sets a PCA color components. 
   * @param _pcaColor PCA color components.
   */
  inline void setPcaColor(const Point &_pcaColor) { m_pcaColor = _pcaColor; }

  
  /** Returns the core cluster after removing outliers. 
   * @return Core cluster.
   */
  inline const Cluster &core() const { return *m_core; }
  inline Cluster &core() { return *m_core; }
  
  /** Sets the core cluster after removing outliers. 
   * @param _core Core cluster.
   */
  inline void setCore(Cluster *_core) { m_core = _core; }

  
  /** Returns a class ID. 
   * @return a unique cluster identifier.
   */
  inline int classId() const { return m_classId; }
  
  /** Sets a class ID. 
   * @param _classId unique cluster identifier.
   */
  void setClassId(int _classId) { m_classId = _classId; }

  
private:
 
  std::vector<CloudPoint> m_points;
  Point m_com;
  Point m_seed;
  Point m_pcaColor;
  Cluster *m_core;
  
  float m_density;
  int m_classId;

  mutable std::vector<CloudPoint> m_layers;
  mutable std::vector<Cluster> m_splitClusters;
  mutable float m_fPerCluster;
};


#endif
