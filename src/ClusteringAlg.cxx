#include "ClusteringAlg.h"

#include <cmath>
#include <iostream>

#include "TStopwatch.h"

ClusteringAlg::ClusteringAlg()
{
}

ClusteringAlg::~ClusteringAlg()
{
}

/**
 * Runs the clustering chain including:
 * - A fast pre-clustering step intended to speed up the actual clustering.
 * - Density calculation.
 * - Full clustering.
 * - Outlier removal.
 *
 * See @ref index for detailed documentation of the underlying algorithms.
 *
 * @param ds Data set to be clustered.
 * @param config Configuration.
 */
void ClusteringAlg::runClustering(DataSet &ds, const Config &config) {

  bool verbose = config.get("verbose");
  
  TStopwatch sw;
  if(verbose) {
    sw.Start();
  }

  
  //
  // Run the pre-clustering step
  //
  runPreClustering(ds, config);

  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "Pre-clustering done: points are grouped into "
	      << ds.preClusters().size() << " clusters."
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }


  //
  // Compute densities
  //
  computeDensities(ds, config);
  
  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "Densities computed"
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }

  
  //
  // Run Clustering
  //
  runSeededClustering(ds, config);

  if(verbose) {
    sw.Stop();
    std::cout << std::endl
	      << "Clustering Done"
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }


  //
  // Cleanup clusters
  //
  cleanupClusters(ds, config);

  if(config.get("verbose")) {
    sw.Stop();
    std::cout << std::endl
	      << "Cleanup done"
	      << std::endl;
    sw.Print("m");
    sw.Start();
  }

  
}
 
/** 
 * Runs a fast crude clustering algorithm the purpose of which is to speedup the actual clustering step.
 * 
 * @param ds Data set to be clustered.
 * @param config Configuration.
 */
void ClusteringAlg::runPreClustering(DataSet &ds, const Config &config)
{

  const std::vector<CloudPoint> &points = ds.points();
  std::vector<Cluster> &clusters = ds.preClusters();

  bool skipPreClustering = config.get("skipPreClustering");
  float dmin = config.get("preClusteringSize");

  for(unsigned int i=0; i<points.size(); i++) {

    const CloudPoint &cp = points[i];
    int icl = -1;
    
    if(!skipPreClustering) {
      for(unsigned int j=0; j<clusters.size(); j++) {
	const Cluster &cl = clusters[j];
	if(fabs(cl.com().x() - cp.x()) > dmin) continue;
	if(fabs(cl.com().z() - cp.z()) > dmin) continue;
	icl = j;
	break;
      }
    }
    
    if(icl >= 0) {
      clusters[icl].addPoint(cp);
    }else{
      Cluster cl;
      cl.addPoint(cp);
      clusters.push_back(cl);
    }
  }

}

  
/**
 * Compute densities by couting cloud points in a neighborhood.
 *
 * @param ds Data set to be clustered.
 * @param config Configuration.
 */
void ClusteringAlg::computeDensities(DataSet &ds, const Config &config) {
  
  float d = config.get("densityWindow");
  float dmax = 0;

  std::vector<Cluster> &preClusters = ds.preClusters();
  for(unsigned int i=0; i<preClusters.size(); i++) {
    Cluster &cli = preClusters[i];
    float density = 0;
    for(unsigned int j=0; j<preClusters.size(); j++) {
      const Cluster &clj = preClusters[j];
      if(fabs(clj.com().x()-cli.com().x()) > d) continue;
      if(fabs(clj.com().z()-cli.com().z()) > d) continue;
      density+=clj.points().size();
    }
    cli.setDensity(density);
    if(density > dmax) {
      dmax = density;
    }
  }
  for(unsigned int i=0; i<preClusters.size(); i++) {
    Cluster &cli = preClusters[i];
    cli.setDensity(cli.density()/dmax);
  }

}
 
/**
 * The clustering algoithm consists of the following steps:
 * - Find seeds which are local density maxima.
 * - Filter seeds to eliminate noise.
 * - Assign points to the nearest seed.
 *
 * @param ds Data set to be clustered.
 * @param config Configuration.
 */
void ClusteringAlg::runSeededClustering(DataSet &ds, const Config &config) {

  const std::vector<Cluster> &preClusters = ds.preClusters();
  std::vector<Cluster> &clusters = ds.clusters();

  //
  // Start by finding seeds which are local density maxima
  //
  std::vector<Cluster> seeds;
  std::vector<Cluster> leftovers;
  float d = config.get("densityWindow");
  for(unsigned int i=0; i<preClusters.size(); i++) {
    const Cluster &cli = preClusters[i];
    bool isLocalMax = true;
    for(unsigned int j=0; j<preClusters.size(); j++) {
      if(i==j) continue;
      const Cluster &clj = preClusters[j];
      if(fabs(clj.com().x()-cli.com().x()) > d) continue;
      if(fabs(clj.com().z()-cli.com().z()) > d) continue;
      if(clj.density() > cli.density()) {
	isLocalMax = false;
	break;
      }
      if(clj.density() == cli.density() && i<j) {
	isLocalMax = false;
      }
    }
    if(isLocalMax) {
      Cluster cl;
      cl.addPoints(cli);
      cl.setDensity(cli.density());
      cl.setSeed(cli.com());
      seeds.push_back(cl);
    }else{
      leftovers.push_back(cli);
    }
  }


  //
  // Apply a selection to the seeds to eliminate noise and fragmented clusters
  //
  float densityTh = config.get("seedDensityThreshold");
  for(unsigned int i=0; i<seeds.size(); i++) {
    const Cluster &cl = seeds[i];
    bool isSelected = true;
    if(cl.density() < densityTh) isSelected = false;
    if(isSelected) {
      clusters.push_back(cl);
    }else{
      leftovers.push_back(cl);
    }
  }

  
  //
  // Assign each pre-cluster to the nearest seed
  //
  for(unsigned int i=0; i<leftovers.size(); i++) {
    const Cluster &cli = leftovers[i];
    int icl = 0;   
    float minDist2DSq = clusters[icl].seed().dist2DSq(cli.com());    
    for(unsigned int j=1; j<clusters.size(); j++) {
      const Cluster &cl = clusters[j];
      float dist2DSq = cl.seed().dist2DSq(cli.com());
      if(dist2DSq < minDist2DSq) {
  	minDist2DSq = dist2DSq;
  	icl = j;
      }
    }
    clusters[icl].addPoints(cli);
  }
}


/**
 * @param ds Data set to be clustered.
 * @param config Configuration.
 */
void ClusteringAlg::cleanupClusters(DataSet &ds, const Config &config) {
  
  std::vector<Cluster> &clusters = ds.clusters();

  float sx = 0;
  float sz = 0;
  float sxx = 0;
  float szz = 0;
  float sxz = 0;
  float nPoints = 0;
  for(unsigned int i=0; i<clusters.size(); i++) {
    const Cluster &cl = clusters[i];
    const Point &clPos = cl.seed();
    for(unsigned int j=0; j<cl.points().size(); j++) {
      float dx = clPos.x() - cl.points()[j].x();
      float dz = clPos.z() - cl.points()[j].z();
      sx += dx;
      sz += dz;
      sxx += dx*dx;
      szz += dz*dz;
      sxz += dx*dz;
      nPoints++;
    }
  }
  sx /= nPoints;
  sz /= nPoints;
  sxx /= nPoints;
  szz /= nPoints;
  sxz /= nPoints;
  sxx -= sx*sx;
  szz -= sz*sz;
  sxz -= sx*sz;
  
  float D = sxx*szz - sxz*sxz;

  float smax = config.get("clusterCoreSize");
  smax = smax*smax;
  
  for(unsigned int i=0; i<clusters.size(); i++) {
    Cluster &cl = clusters[i];
    const Point &clPos = cl.seed();
    Cluster *core = new Cluster();
    for(unsigned int j=0; j<cl.points().size(); j++) {
      float dx = (clPos.x() - cl.points()[j].x());
      float dz = (clPos.z() - cl.points()[j].z());
      float dS = (dx*dx*szz + dz*dz*sxx - 2*dx*dz*sxz) / D;
      bool selected = true;
      if(dS > smax) selected = false;
      if(selected) {
	core->addPoint(cl.points()[j]);
      }
    }
    cl.setCore(core);
  }

}
