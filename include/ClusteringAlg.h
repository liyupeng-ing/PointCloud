#ifndef CLUSTERING_ALG_H
#define CLUSTERING_ALG_H

#include <vector>

#include "DataSet.h"
#include "optparse.h"

/**
 * @brief This class implements the clustering algorithm.
 *
 * This includes:
 * - A fast pre-clustering step intended to speed up the actual clustering.
 * - Density calculation.
 * - Full clustering.
 * - Outlier removal.
 *
 * See @ref index for detailed documentation of the underlying algorithms.
 */
class ClusteringAlg {

public:

  /** Default Constructor. */
  ClusteringAlg();

  /** Destructor. */
  ~ClusteringAlg();

  /** Runs the clustering chain. */
  void runClustering(DataSet &ds, const Config &config);

private:
  
  /** Runs a pre-clustering step. */
  void runPreClustering(DataSet &ds, const Config &config);

  /** Compute densities of pre-clusters. */
  void computeDensities(DataSet &ds, const Config &config);

  /** Runs the actual clustering algorithm. */
  void runSeededClustering(DataSet &ds, const Config &config);

  /** Cleanup clusters from noise. */
  void cleanupClusters(DataSet &ds, const Config &config);
  
};

#endif
