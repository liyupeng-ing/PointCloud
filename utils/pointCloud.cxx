/**
 * @file
 */

#include <iostream>
#include <unistd.h>
#include <string>
#include <iomanip>

#include "DataSet.h"
#include "TStopwatch.h"
#include "ClusteringAlg.h"
#include "ClassificationAlg.h"
#include "optparse.h"

void parseCommandLine(Config &config, int argc, char **argv);

/**
 * @defgroup CloudPoints Main Program
 *
 * @brief Main program.
 *
 * @b Objective: Analyse cloud points to identify players and their teams.
 * See @ref index for full description of the algorithms involved.
 *
 * @{
 */

/**
 * @brief Main function
 *
 * This program performs the following tasks:
 * - Parse the command line and defines configuration.
 * - Read data from a text file.
 * - Runs a spacial clustering to identify players on the field.
 * - Runs a color analysis to identify the teams.
 *
 * Full documentation of the algorithms is available in @ref index.
 * 
 * @param argc Number of command line arguments.
 * @param argv Array of command line arguments.
 * @return 0 upon successfull exit
 */
int main(int argc, char **argv) {

  srand(123);
  
  //
  // Initialize program settings
  //
  Config config;
  parseCommandLine(config, argc, argv);

  TStopwatch sw;
  if(config.get("verbose")) {
    std::cout << "Hello" 
	      << std::endl;
    sw.Start();
  }

  
  //
  // Read data from the input file
  //
  DataSet trainingData;
  DataSet evaluationData;
  DataSet::readFromFile(config,
			trainingData,
			evaluationData);

  if(config.get("verbose")) {
    sw.Stop();
    std::cout << std::endl
	      << "Reading data done: "
	      << trainingData.points().size() << " (training) and "
	      << evaluationData.points().size() << " (evaluation) data points are read."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }
  
  //
  // Runs clustering algorithm
  //
  ClusteringAlg clAlg;

  if(config.get("verbose")) {
    std::cout << std::endl << "Running clustering on training data" << std::endl;
  }  
  clAlg.runClustering(trainingData, config);

  if(config.get("verbose")) {
    std::cout << std::endl << "Running clustering on evaluation data" << std::endl;
  }  
  clAlg.runClustering(evaluationData, config);
  
  if(config.get("verbose")) {
    sw.Stop();
    std::cout << std::endl
	      << "Clustering done: "
	      << trainingData.clusters().size() << " (training) and "
	      << evaluationData.clusters().size() << " (evaluation) clusters are found."
	      << std::endl;
    sw.Print();
    sw.Start();
  }

  //
  // Run classification algorithm
  //
  ClassificationAlg classificationAlg;
  classificationAlg.classifyClusters(trainingData, evaluationData, config);
  
  if(config.get("verbose")) {
    sw.Stop();
    std::cout << std::endl
	      << "Classification done."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }
  


  //
  // Output results
  //
  const std::vector<std::string> &classNames = classificationAlg.classNames();
  for(unsigned int i=0; i<classNames.size(); i++) {
    std::cout << classNames[i] << ": ";
    bool isFirst = true;
    for(unsigned int j=0; j<evaluationData.clusters().size(); j++) {
      const Cluster &cl = evaluationData.clusters()[j];    
      if(cl.classId() == (int)i) {
	const Point clPos = cl.core().com();
	std::cout << std::fixed << std::setprecision(2) << (isFirst?"[":", ") << "[" << clPos.x() << ", " << clPos.z() << "]";
	isFirst = false;
      }
    }
    std::cout << "]" << std::endl;
  }
  
  return 0;
}

/** 
 * @brief Prase command line arguments.
 *
 * @param config Configuration to parse into.
 * @param argc Number of command line arguments.
 * @param argv Array of command line arguments.
 *
 * #### Configuration details: 
 */
void parseCommandLine(Config &config, int argc, char **argv)
{

  optparse::OptionParser parser = optparse::OptionParser().description("Point Cloud Analysis");

  /** - @b -v, <b> \-\-verbose </b> Turns ON verbose mode. */
  parser.add_option("-v", "--verbose").action("store_true").dest("verbose").set_default(false)
    .help("Turns ON verbose mode.");

  /** - @b -i, <b> \-\-inputFile </b> Name of the data input file.*/
  parser.add_option("-i", "--inputFile").action("store").dest("inputFile").set_default("./share/point_cloud_data.txt")
    .help("Name of the data input file.");

  /** - @b -f, <b> \-\-evaluationDataFraction </b> Fraction of data to use for evaluation. */
  parser.add_option("-f", "--evaluationDataFraction").action("store").dest("evaluationDataFraction").set_default(0.2)
    .help("Fraction of data to use for evaluation.");

  /** - @b -p, <b> \-\-skipPreClustering </b> Don't run pre-clustering. */
  parser.add_option("-p", "--skipPreClustering").action("store_true").dest("skipPreClustering").set_default(false)
    .help("Don't run pre-clustering.");
  
  /** - @b -P, <b> \-\-preClusteringSize </b> Size parameter in unit length for pre-clustering. */
  parser.add_option("-P", "--preClusteringSize").action("store").dest("preClusteringSize").set_default(0.2)
    .help("Size parameter in unit length for pre-clustering.");

  /** - @b -d, <b> \-\-densityWindow </b> Size of the window used to compute densities. */
  parser.add_option("-d", "--densityWindow").action("store").dest("densityWindow").set_default(0.5)
    .help("Size of the window used to compute densities.");
  
  /** - @b -D, <b> \-\-seedDensityThreshold </b> Density threshold for seed selection, normalized to maximum density. */
  parser.add_option("-D", "--seedDensityThreshold").action("store").dest("seedDensityThreshold").set_default(0.5)
    .help("Density threshold for seed selection, normalized to maximum density.");

  /** - @b -c, <b> \-\-clusterCoreSize </b> Size parameter in units of standard deviations for outlier removal. */
  parser.add_option("-c", "--clusterCoreSize").action("store").dest("clusterCoreSize").set_default(2)
    .help("Size parameter in units of standard deviations for outlier removal.");

  /** - @b -u, <b> \-\-unsupervisedClassification </b> Run unsuppervised classification. */
  parser.add_option("-u", "--unsupervisedClassification").action("store_true").dest("unsupervisedClassification").set_default(false)
    .help("Run unsuppervised classification.");

  /** - @b -l, <b> \-\-nLayersPerCluster </b> Number of layers per cluster for color analysis. */
  parser.add_option("-l", "--nLayersPerCluster").action("store").dest("nLayersPerCluster").set_default(5)
    .help("Number of layers per cluster for color analysis.");

  /** - @b -t, <b> \-\-runMVATraining </b> Runs MVA training when runing in supervised classification mode. */
  parser.add_option("-t", "--runMVATraining").action("store_true").dest("runMVATraining").set_default(false)
    .help("Runs MVA training when runing in supervised classification mode.");

  /** - @b -o, <b> \-\-tmvaOutputFile </b> Output file to save TMVA performance histograms. Put "None" to skip saving histograms. */
  parser.add_option("-o", "--tmvaOutputFile").action("store").dest("tmvaOutputFile").set_default("None")
    .help("Output file to save TMVA performance histograms. Put \"None\" to skip saving histograms.");

  /** - @b -T, <b> \-\-truePositionsFileName </b> File name containing truth player positions and teams. */
  parser.add_option("-T", "--truePositionsFileName").action("store").dest("truePositionsFileName")
    .set_default("./share/point_cloud_true_positions.txt")
    .help("File name containing truth player positions and teams.");

  /** - @b -N, <b> \-\-trainingClustersSplitN </b> Number of sub-clusters for training splitting. */
  parser.add_option("-N", "--trainingClustersSplitN").action("store").dest("trainingClustersSplitN").set_default(300)
    .help("Number of sub-clusters for training splitting.");

  /** - @b -F, <b> \-\-trainingClustersSplitF </b> Fraction of points in each sub-cluster for training splitting. */
  parser.add_option("-F", "--trainingClustersSplitF").action("store").dest("trainingClustersSplitF").set_default(0.25)
    .help("Fraction of points in each sub-cluster for training splitting.");
  
  /** - @b -K, <b> \-\-maxKmeansIterations </b> Maximum number of k-means iterations during PCA/kmeans classification. */
  parser.add_option("-K", "--maxKmeansIterations").action("store").dest("maxKmeansIterations").set_default(1000)
    .help("Maximum number of k-means iterations during PCA/kmeans classification.");

  config = parser.parse_args(argc, argv);

}

/**
 * @}
 */
