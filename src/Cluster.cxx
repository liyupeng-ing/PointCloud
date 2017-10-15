#include "Cluster.h"

#include <iostream>

/**
 * Initialize a cluster with no points.
 */
Cluster::Cluster() :
  m_com(Point(0,0,0)),
  m_seed(Point(0,0,0)),
  m_pcaColor(Point(0,0,0)),
  m_core(0),
  m_density(0),
  m_classId(-1),
  m_fPerCluster(0)
{
}

/**
 * @param cl Cluster to copy
 */
Cluster::Cluster(const Cluster &cl) :
  m_points(cl.m_points),
  m_com(cl.m_com),
  m_seed(cl.m_seed),
  m_pcaColor(cl.m_pcaColor),
  m_core(0),
  m_density(cl.m_density),
  m_classId(cl.m_classId),
  m_layers(cl.m_layers),
  m_splitClusters(cl.m_splitClusters),
  m_fPerCluster(cl.m_fPerCluster)
{
  if(cl.m_core != 0) {
    m_core = new Cluster(*cl.m_core);
  }
}

Cluster::~Cluster()
{
  if(m_core) delete m_core;
}

/**
 * @param cp Cloud point to add.
 * 
 * Adds the point to the cluster and updates the center of mass.
 */
void Cluster::addPoint(const CloudPoint &cp) {
  m_com.setX( ( m_com.x() * m_points.size() + cp.x() ) / (m_points.size()+1.) );
  m_com.setY( ( m_com.y() * m_points.size() + cp.y() ) / (m_points.size()+1.) );
  m_com.setZ( ( m_com.z() * m_points.size() + cp.z() ) / (m_points.size()+1.) );
  m_points.push_back(cp);
  m_layers.clear();
  m_splitClusters.clear();
}

/**
 * @param cl Cluster of points to add.
 */
void Cluster::addPoints(const Cluster &cl) {
  for(unsigned int i=0; i<cl.m_points.size(); i++) {
    addPoint(cl.m_points[i]);
  }
}

/**
 * @param nLayers Number of requested layers.
 * @return Vector of layers.
 *
 * This information is used by the classification algorithm.
 * Each layer is represented by a point and a color which are averaged over all point in this layer.
 */
const std::vector<CloudPoint> &Cluster::layers(int nLayers) const {

  if(nLayers == (int)m_layers.size()) return m_layers;
  
  double ymax = 0;
  for(unsigned int i=0; i<m_points.size(); i++) {
    if(m_points[i].y() > ymax) ymax = m_points[i].y();
  }
  m_layers.resize(nLayers);
  std::vector<int> nPointsPerLayer(nLayers, 0);
  
  for(unsigned int i=0; i<m_points.size(); i++) {
    const CloudPoint &p = m_points[i];
    int iLayer = (int)(nLayers*p.y()/ymax);
    if(iLayer >= nLayers) iLayer = nLayers-1;
    CloudPoint &layer = m_layers[iLayer];
    layer.setY(layer.y()+p.y());
    layer.setR(layer.r()+p.r());
    layer.setG(layer.g()+p.g());
    layer.setB(layer.b()+p.b());
    nPointsPerLayer[iLayer]++;
  }
  
  for(unsigned int i=0; i<m_layers.size(); i++) {
    if(nPointsPerLayer[i] != 0) {
      m_layers[i].setX(m_seed.x());
      m_layers[i].setY(m_layers[i].y()/nPointsPerLayer[i]);
      m_layers[i].setZ(m_seed.z());
      m_layers[i].setR(m_layers[i].r()/nPointsPerLayer[i]);
      m_layers[i].setG(m_layers[i].g()/nPointsPerLayer[i]);
      m_layers[i].setB(m_layers[i].b()/nPointsPerLayer[i]);
    }else{
      std::cout << "WARNING: Layer with no points found" << std::endl;
    }
  }

  return m_layers;
}

/**
 * @param nClusters Number of random sub-clusters to generate.
 * @param fPerCluster Franction of points per cluster.
 * @return Random sub-clusters
 *
 * This is intended to be used during training. 
 * 
 */
std::vector<Cluster> &Cluster::randomSplit(int nClusters, float fPerCluster) const
{

  if(nClusters == (int)m_splitClusters.size() &&
     fPerCluster == m_fPerCluster) {
    return m_splitClusters;
  }

  m_splitClusters.resize(nClusters);

  for(unsigned int i=0; i<m_splitClusters.size(); i++) {
    for(unsigned int j=0; j<m_points.size(); j++) {
      double f = rand() / (double)(RAND_MAX);
      if(f < fPerCluster) {
	m_splitClusters[i].addPoint(m_points[j]);
      }
    }
  }

  return m_splitClusters;
}
