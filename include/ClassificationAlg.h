#ifndef CLASSIFICATIONALG_H
#define CLASSIFICATIONALG_H

#include "DataSet.h"
#include "optparse.h"

class TPrincipal;

/**
 * @brief This class implements the classification algorithm.
 *
 * Implements two approaches:
 * - Supervised classification: uses a BDT and requires a training data set with "truth" information.
 * - Unsupervised classification: uses a PCA and discovers the classification features from the data itself.
 *
 * See @ref index for detailed documentation of the underlying algorithms.
 */
class ClassificationAlg {

public:

  /** Default Constructor. */
  ClassificationAlg();

  /** Destructor. */
  ~ClassificationAlg();

  /** Perform classification. */
  void classifyClusters(DataSet &trainingDS, DataSet &evaluationDS, const Config &config);

  /** Return class names. 
   * @return Class names.
   */
  const std::vector<std::string> &classNames() const { return m_classNames; }
  
private:
  
  /** Performs unsupervised PCA-based classification. */
  void runPCA(DataSet &trainingDS, DataSet &evaluationDS, const Config &config);

  /** Performs PCA training. */
  void trainPCA(DataSet &ds, const Config &config);

  /** Apply PCA transformation to a list of clusters. */
  void applyPCA(std::vector<Cluster*> &clusters, const Config &config);

  /** Apply k-means clustering on PCA result. */
  int runKmeansOnPCA(int kmeans, std::vector<Cluster*> &clusters,
		     std::vector<std::vector<Cluster*> > &pcaClusters,
		     const Config &config);
  
  /** Perform supervised MVA-based classification. */
  void runMVA(DataSet &trainingDS, DataSet &evaluationDS, const Config &config);

  /** Performs MVA training. */
  void trainMVA(DataSet &ds, const Config &config);

private:

  std::vector<std::string> m_classNames;

  TPrincipal *m_pca;
};

#endif
