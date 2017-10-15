#include "ClassificationAlg.h"

#include "TMVA/Tools.h"
#include "TMVA/Factory.h"
#include "TMVA/DataLoader.h"
#include "TMVA/Reader.h"

#include "TFile.h"
#include "TPrincipal.h"
#include "TStopwatch.h"

#include <set>

ClassificationAlg::ClassificationAlg()
{
}

ClassificationAlg::~ClassificationAlg()
{
}



/**
 * This is the top level classification method.
 * It splits clusters into vertical layers and uses the RGB color values per layer as discriminating information.
 * It implements two approaches:
 * - Unsupervised classification based on a Principal Components Analysis followed by a k-means clustering 
 * in the phase space of the 3 leading PCA components.
 * - Supervised classification based on a multi-class Boosted Decision Trees.
 *
 * See @ref index for detailed documentation of the underlying algorithms.
 *
 * @param trainingDS Data set to be used for training.
 * @param evaluationDS Data set to be classified.
 * @param config Configuration.
 */
void ClassificationAlg::classifyClusters(DataSet &trainingDS, DataSet &evaluationDS, const Config &config) {

  if(config.get("unsupervisedClassification")) {

    runPCA(trainingDS, evaluationDS, config);
  }else{

    runMVA(trainingDS, evaluationDS, config);
  }
}



/**
 * Runs a Principal Component Analysis with input data the RGB color coponent for each cluster layer.
 * Training data is used to compute the PCA transformation. \n
 * For classification, performs a k-means clustering to the PCA space reduced to the leading 3 components.
 *
 * @param trainingDS Data set to be used for training.
 * @param evaluationDS Data set to be classified.
 * @param config Configuration.
 */
void ClassificationAlg::runPCA(DataSet &trainingDS, DataSet &evaluationDS, const Config &config)
{

  TStopwatch sw;
  bool verbose = config.get("verbose");
  if(verbose) {
    sw.Start();
  }
  
  //
  // Train a Principal Component Analysis
  //
  trainPCA(trainingDS, config);

  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "PCA training done."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }


  //
  // Apply PCA to all data
  // and reduce phase space to the 3 leading components
  //
  std::vector<Cluster> &trainingClusters = trainingDS.clusters();
  std::vector<Cluster> &evaluationClusters = evaluationDS.clusters();
  std::vector<Cluster*> kmeansInputs;
  int nSplit = config.get("trainingClustersSplitN");
  float splitFrac = config.get("trainingClustersSplitF");
  for(unsigned int i=0; i<trainingClusters.size(); i++) {
    Cluster &bigCl = trainingClusters[i];
    std::vector<Cluster> &splitClusters = bigCl.core().randomSplit(nSplit, splitFrac);
    kmeansInputs.push_back(&bigCl.core());
    for(unsigned int j=0; j<splitClusters.size(); j++) {
      kmeansInputs.push_back(&splitClusters[j]);
    }
  }
  for(unsigned int i=0; i<evaluationClusters.size(); i++) {
    kmeansInputs.push_back(&evaluationClusters[i].core());
  }
  applyPCA(kmeansInputs, config);
  
  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "PCA transformation done."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }


  //
  // Run k-meam clustering on PCA data
  //
  const int kmeans = 3;
  std::vector<std::vector<Cluster*> > pcaClusters(kmeans);
  int nIterations = runKmeansOnPCA(kmeans, kmeansInputs, pcaClusters, config);
  
  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "K-means converged after " << nIterations << " iterations."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }


  //
  // Classify clusters based on kmean results
  //
  m_classNames.push_back("TeamA");
  m_classNames.push_back("TeamB");
  m_classNames.push_back("Referees");  
  int iReferees = -1;
  unsigned int minSize = 999;
  for(unsigned int i=0; i<pcaClusters.size(); i++) {
    if(pcaClusters[i].size() < minSize) {
      minSize = pcaClusters[i].size();
      iReferees = i;
    }
  }
    
  int classId = 0;
  for(unsigned int i=0; i<pcaClusters.size(); i++) {
    int thisClassId = classId;
    if((int)i==iReferees) {
      thisClassId = pcaClusters.size()-1;
    }else{
      thisClassId = classId;
      classId++;
    }
    for(unsigned int j=0; j<pcaClusters[i].size(); j++) {
      pcaClusters[i][j]->setClassId(thisClassId);
    }
  }
  for(unsigned int i=0; i<trainingClusters.size(); i++) {
    Cluster &cl = trainingClusters[i];
    cl.setClassId(cl.core().classId());
  }
  for(unsigned int i=0; i<evaluationClusters.size(); i++) {
    Cluster &cl = evaluationClusters[i];
    cl.setClassId(cl.core().classId());
  }
  
  
  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "PCA/Kmeans Classification of clusters done."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }
}



/**
 * Computes the PCA transformation parameters based on the training data.
 *
 * @param ds Input data set.
 * @param config Configuration.
 */
void ClassificationAlg::trainPCA(DataSet &ds, const Config &config)
{
  
  std::vector<Cluster> &trainingClusters = ds.clusters();

  
  //
  // Prepare PCA
  //
  int nLayers = config.get("nLayersPerCluster");
  
  int pcaDataSize = 3*nLayers;
  m_pca = new TPrincipal(pcaDataSize, "ND");
  double *pcaDataRow = new double[pcaDataSize];

  //
  // Load training data
  // 
  int nSplit = config.get("trainingClustersSplitN");
  float splitFrac = config.get("trainingClustersSplitF");
  for(unsigned int i=0; i<trainingClusters.size(); i++) {
    const Cluster &bigCl = trainingClusters[i];
    const std::vector<Cluster> &splitClusters = bigCl.core().randomSplit(nSplit, splitFrac);
    for(unsigned int j=0; j<splitClusters.size(); j++) {
      const Cluster &pl = splitClusters[j];
      const std::vector<CloudPoint> &layers = pl.layers(nLayers);
      for(unsigned int k=0; k<layers.size(); k++) {
	const CloudPoint &p = layers[k];
	pcaDataRow[3*k+0] = p.r();
	pcaDataRow[3*k+1] = p.g();
        pcaDataRow[3*k+2] = p.b();
      }
      m_pca->AddRow(pcaDataRow);
    }
  }


  //
  // Run PCA
  //
  m_pca->MakePrincipals();

  delete [] pcaDataRow;
}


/**
 * @param clusters Input clusters.
 * @param config Configuration.
 */
void ClassificationAlg::applyPCA(std::vector<Cluster*> &clusters, const Config &config)
{

  int nLayers = config.get("nLayersPerCluster");
  int pcaDataSize = 3*nLayers;
  double *pcaInDataRow = new double[pcaDataSize];
  double *pcaOutDataRow = new double[pcaDataSize];

  for(unsigned int i=0; i<clusters.size(); i++) {
    Cluster &cl = *clusters[i];
    const std::vector<CloudPoint> &layers = cl.layers(nLayers);

    for(unsigned int k=0; k<layers.size(); k++) {
      const CloudPoint &p = layers[k];
      pcaInDataRow[3*k+0] = p.r();
      pcaInDataRow[3*k+1] = p.g();
      pcaInDataRow[3*k+2] = p.b();
    }

    m_pca->X2P(pcaInDataRow, pcaOutDataRow);
    Point pcaColor(pcaOutDataRow[0], pcaOutDataRow[1], pcaOutDataRow[2]);
    cl.setPcaColor(pcaColor);
  }
  delete [] pcaInDataRow;
  delete [] pcaOutDataRow;
}

/**
 * @param kmeans Number of output clusters.
 * @param clusters Input clusters.
 * @param pcaClusters Output clusters.
 * @param config Configuration.
 */
int ClassificationAlg::runKmeansOnPCA(int kmeans, std::vector<Cluster*> &clusters,
			       std::vector<std::vector<Cluster*> > &pcaClusters,
			       const Config &config)
{

  std::vector<Point> seeds;
  std::set<int> used;
  for(int i=0; i<kmeans; i++) {
    int iCl = rand()%clusters.size();
    while(used.find(iCl) != used.end()) {
      iCl = rand()%clusters.size();
    }
    used.insert(iCl);
    seeds.push_back(clusters[iCl]->pcaColor());
  }

  bool converged = false;
  int nIterations = 0;
  int nIterationsMax = config.get("maxKmeansIterations");
  while(!converged && nIterations < nIterationsMax) {

    // Assignment step
    for(unsigned int i=0; i<pcaClusters.size(); i++) {
      pcaClusters[i].clear();
    }
    for(unsigned int i=0; i<clusters.size(); i++) {
      int jj = -1;
      float minDist = 99999.;
      for(unsigned int j=0; j<seeds.size(); j++) {
	float dist = clusters[i]->pcaColor().dist3DSq(seeds[j]);
	if(dist < minDist) {
	  minDist = dist;
	  jj = j;
	}
      }
      pcaClusters[jj].push_back(clusters[i]);
    }

    // Update step
    converged = true;
    for(unsigned int i=0; i<pcaClusters.size(); i++) {
      Point oldSeed = seeds[i];
      Point newSeed;
      for(unsigned int j=0; j<pcaClusters[i].size(); j++) {
	newSeed.setX(newSeed.x() + pcaClusters[i][j]->pcaColor().x());
	newSeed.setY(newSeed.y() + pcaClusters[i][j]->pcaColor().y());
	newSeed.setZ(newSeed.z() + pcaClusters[i][j]->pcaColor().z());
      }
      newSeed.setX(newSeed.x()/pcaClusters[i].size());
      newSeed.setY(newSeed.y()/pcaClusters[i].size());
      newSeed.setZ(newSeed.z()/pcaClusters[i].size());
      seeds[i] = newSeed;
      if(newSeed.dist3DSq(oldSeed) > 0.001) converged = false;
    }
    nIterations++;
  }

  return nIterations;
}



/**
 * Performs training if requested using the training data set 
 * and then run the classification on the evaluation data set. \n
 * Implements a Gradient Boosted Decision Tree as MVA.
 *
 * @param trainingDS Data set to be used for training.
 * @param evaluationDS Data set to be classified.
 * @param config Configuration.
 */
void ClassificationAlg::runMVA(DataSet &trainingDS, DataSet &evaluationDS, const Config &config)
{

  std::vector<Cluster> &clusters = evaluationDS.clusters();
  int nLayers = config.get("nLayersPerCluster");

  TStopwatch sw;
  bool verbose = config.get("verbose");
  if(verbose) {
    sw.Start();
  }

  
  //
  // Run training if requested
  //
  if(config.get("runMVATraining")) {
    trainMVA(trainingDS, config);
  }
  
  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "MVA training done."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }

  
  //
  // Initialize MVA
  //
  TMVA::Tools::Instance();
  
  TMVA::Reader *reader = new TMVA::Reader( "!Color:Silent" );

  std::vector<float> vars(3*nLayers);
  for(int k=0; k<nLayers; k++) {
    TString suffix = TString::Format("%d", k);
    reader->AddVariable( "r"+suffix, &vars[3*k+0] );
    reader->AddVariable( "g"+suffix, &vars[3*k+1] );
    reader->AddVariable( "b"+suffix, &vars[3*k+2] );
  }
  
  reader->BookMVA( "BDT", "outputs/weights/TMVAMulticlass_BDT.weights.xml" );


  //
  // Classify clusters
  //
  m_classNames.clear();
  for(unsigned int i=0; i<reader->DataInfo().GetNClasses(); i++) {
    m_classNames.push_back(reader->DataInfo().GetClassInfo(i)->GetName());
  }
  for(unsigned int i=0; i<clusters.size(); i++) {
    const Cluster &pl = clusters[i].core();
    const std::vector<CloudPoint> &layers = pl.layers(nLayers);

    for(unsigned int k=0; k<layers.size(); k++) {
      const CloudPoint &p = layers[k];
      vars[3*k+0] = p.r();
      vars[3*k+1] = p.g();
      vars[3*k+2] = p.b();
    }
    
    const std::vector< float > &res = reader->EvaluateMulticlass("BDT");
    
    int classId = -1;
    for(unsigned int ic=0; ic<res.size(); ic++) {
      if(classId == -1 || res[ic] > res[classId]) {
	classId = ic;
      }
    }
    clusters[i].setClassId(classId);
  }
  
  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "MVA classification done."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }
  
  delete reader;
}

  
/**
 * @param ds Data set to be classified.
 * @param config Configuration.
 */
void ClassificationAlg::trainMVA(DataSet &ds, const Config &config)
{

  std::vector<Cluster> &trainingClusters = ds.clusters();
  
  int nLayers = config.get("nLayersPerCluster");

  
  //
  // Initialize MVA
  //
  TMVA::Tools::Instance();
  
  std::string outfileName = config.get("tmvaOutputFile"); 
  TFile* outputFile=0;
  TMVA::Factory *factory=0;

  TString factoryOptions = "!V:Silent:!DrawProgressBar:!Color:Transformations=I:AnalysisType=Auto";
  if(outfileName != "None") {
    outputFile = TFile::Open( outfileName.c_str(), "RECREATE" );
    factory = new TMVA::Factory( "TMVAMulticlass", outputFile, factoryOptions );
  }else{
    factory = new TMVA::Factory( "TMVAMulticlass", factoryOptions );
  }

  TMVA::DataLoader *dataLoader = new TMVA::DataLoader("outputs");

  
  //
  // Read truth positions
  //
  std::map<std::string, std::vector<Point> > truePositions;
  std::string truthFileName = config.get("truePositionsFileName");
  std::ifstream ifile(truthFileName, std::ios::in);
  if(!ifile) {
    throw std::runtime_error("ERROR: File "+truthFileName+" not found");
  }
  while(!ifile.eof()) {
    float x, z;
    std::string clName;
    ifile >> x >> z >> clName;
    if(ifile.good()) {
      truePositions[clName].push_back(Point(x, 0, z));
    }
  }
  ifile.close();

  m_classNames.clear();
  for(std::map<std::string, std::vector<Point> >::iterator itr=truePositions.begin();
      itr != truePositions.end(); itr++) {
    m_classNames.push_back(itr->first);
  }


  //
  // Classify training clusters using truth information
  //
  for(unsigned int i=0; i<trainingClusters.size(); i++) {
    Cluster &cl = trainingClusters[i];
    float distMin = 99999.;
    TString className = "";
    for(std::map<std::string, std::vector<Point> >::iterator itr=truePositions.begin();
	itr != truePositions.end(); itr++) {
      for(unsigned int j=0; j<itr->second.size(); j++) {
	float dist = itr->second[j].dist2DSq(cl.core().com());
	if(dist < distMin) {
	  distMin = dist;
	  className = itr->first;
	}
      }
    }
    for(unsigned int j=0; j<m_classNames.size(); j++) {
      if(m_classNames[j] == className) {
	cl.setClassId(j);
	break;
      }
    }
  }


  //
  // Load training data
  //
  std::vector<double> vars(3*nLayers);
  
  for(int i=0; i<nLayers; i++) {
    TString suffix = TString::Format("%d", i);
    dataLoader->AddVariable( "r"+suffix, 'F' );
    dataLoader->AddVariable( "g"+suffix, 'F' );
    dataLoader->AddVariable( "b"+suffix, 'F' );
  }

  int nSplit = config.get("trainingClustersSplitN");
  float splitFrac = config.get("trainingClustersSplitF");
  for(unsigned int i=0; i<trainingClusters.size(); i++) {
    const Cluster &bigCl = trainingClusters[i];
    std::vector<Cluster> splitClusters = bigCl.core().randomSplit(nSplit, splitFrac);
    std::string className = m_classNames[bigCl.classId()];
    for(unsigned int j=0; j<splitClusters.size(); j++) {

      const Cluster &pl = splitClusters[j];
      
      const std::vector<CloudPoint> &layers = pl.layers(nLayers);
      
      for(unsigned int k=0; k<layers.size(); k++) {
	const CloudPoint &p = layers[k];
	vars[3*k+0] = p.r();
	vars[3*k+1] = p.g();
        vars[3*k+2] = p.b();
      }

      int r = rand() % 2;
      if(r) {
	dataLoader->AddTrainingEvent(className.c_str(), vars, 1);
      }else{
	dataLoader->AddTestEvent(className.c_str(), vars, 1);
      }
    }
  }
   
  dataLoader->PrepareTrainingAndTestTree( "", "!V" );


  //
  // Perform training
  //
  factory->BookMethod( dataLoader, TMVA::Types::kBDT, "BDT",
		       "!H:!V:"
		       "NTrees=200:"
		       "BoostType=Grad:"
		       "Shrinkage=0.30:"
		       "UseBaggedBoost:"
		       "BaggedSampleFraction=0.50:"
		       "nCuts=20:"
		       "MaxDepth=2:");

  factory->TrainAllMethods();

  if(outputFile) {
    factory->TestAllMethods();
    factory->EvaluateAllMethods();
    outputFile->Close();
    delete outputFile;
  }
  
  delete factory;
  delete dataLoader;
}
