#ifndef DATASET_H
#define DATASET_H

#include <string>
#include <vector>

#include "CloudPoint.h"
#include "Cluster.h"
#include "optparse.h"

/**
 * @brief This class represents the data to be analysed.
 *
 * It holds structured information about the data 
 * and provides the interface between the persistent data and the various algorithms.
 */
class DataSet {

public:

  /** Default Constructor */
  DataSet();

  /** Destructor */
  ~DataSet();

  /** Reads data from a text file. */
  static bool readFromFile(const Config &config,
			   DataSet &trainingData,
			   DataSet &evaluationData);

  /** Returns the minimum of a coordinate */
  float getCoordinateMin(int coordinate);
  
  /** Returns the maximum of a coordinate */
  float getCoordinateMax(int coordinate);
  
  /** Returns cloud points. 
   * @return All Cloud Points.
   */
  inline std::vector<CloudPoint> &points() { return m_points; }

  /** Returns clusters after pre-clustering step.
   * @return pre-clusters.
   */
  inline std::vector<Cluster> &preClusters() { return m_preClusters; }
  
  /** Returns clusters after full clustering. 
   * @return clusters,
   */
  inline std::vector<Cluster> &clusters() { return m_clusters; }
  
private:

  std::vector<CloudPoint> m_points;
  std::vector<Cluster> m_preClusters;
  std::vector<Cluster> m_clusters;

  std::vector<float> m_mins;
  std::vector<float> m_maxs;
};

#endif
