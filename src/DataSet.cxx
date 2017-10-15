#include "DataSet.h"

#include <iostream>

/** 
 * Initialize an empty data set.
 */
DataSet::DataSet()
{
  m_mins.resize(6);
  m_maxs.resize(6);
}

DataSet::~DataSet()
{
}

/** 
 * Expected format: Each line of the input file has 6 columns (X, Y, Z, R, G, B) \n
 *  - X, Y, Z: the ​ xyz position of the 3d point  \n
 *  - R, G, B: the ​ rgb component of the color of the 3d point
 * 
 * Data points are split into training and evaluation sets.
 * The ranges (min,max) of the data coordinates is comuted at this stage.
 *
 * @param config Configuration.
 * @param trainingData Training data set.
 * @param evaluationData Evaluation data set.
 * @return @c true upon success, @c false upon failure.
 */
bool DataSet::readFromFile(const Config &config,
			   DataSet &trainingData,
			   DataSet &evaluationData) {

  std::string fileName = config.get("inputFile");
  float evalFrac = config.get("evaluationDataFraction");

  std::ifstream ifile(fileName.c_str(), std::ios::in);
  if(!ifile) {
    std::cout << "Error: could not open file " << fileName << std::endl;
    return false;
  }

  std::vector<float> mins(6);
  std::vector<float> maxs(6);

  bool isFirst = true;
  while(!ifile.eof()) {
    
    CloudPoint cp;
    ifile >> cp;
    
    if(ifile.good()) {
      if(cp.isValid()) {
	
	float r = rand() / float(RAND_MAX);
	if(r < evalFrac) {
	  evaluationData.m_points.push_back(cp);
	}else{
	  trainingData.m_points.push_back(cp);
	}
	
	if(isFirst) {
	  isFirst = false;
	  mins[0] = cp.x();
	  mins[1] = cp.y();
	  mins[2] = cp.z();
	  mins[3] = cp.r();
	  mins[4] = cp.g();
	  mins[5] = cp.b();
	  maxs[0] = cp.x();
	  maxs[1] = cp.y();
	  maxs[2] = cp.z();
	  maxs[3] = cp.r();
	  maxs[4] = cp.g();
	  maxs[5] = cp.b();
	}else{
	  if(mins[0] > cp.x()) mins[0] = cp.x();
	  if(mins[1] > cp.y()) mins[1] = cp.y();
	  if(mins[2] > cp.z()) mins[2] = cp.z();
	  if(mins[3] > cp.r()) mins[3] = cp.r();
	  if(mins[4] > cp.g()) mins[4] = cp.g();
	  if(mins[5] > cp.b()) mins[5] = cp.b();
	  if(maxs[0] < cp.x()) maxs[0] = cp.x();
	  if(maxs[1] < cp.y()) maxs[1] = cp.y();
	  if(maxs[2] < cp.z()) maxs[2] = cp.z();
	  if(maxs[3] < cp.r()) maxs[3] = cp.r();
	  if(maxs[4] < cp.g()) maxs[4] = cp.g();
	  if(maxs[5] < cp.b()) maxs[5] = cp.b();
	}
      }else{
	std::cout << "Error: invalid data read: " << cp << std::endl;
	return false;
      }
    }
  }

  ifile.close();

  trainingData.m_mins = mins;
  trainingData.m_maxs = maxs;
  evaluationData.m_mins = mins;
  evaluationData.m_maxs = maxs;

  return true;
}

/**
 * Returns the absolute minimum of a given coordinate.
 *
 * @param coordinate Indes of the coordinate.
 * @ return the absolute minimum.
 */
float DataSet::getCoordinateMin(int coordinate)
{
  if(coordinate < 0 || coordinate >= 6) return -999;
  return m_mins[coordinate];
}

/**
 * Returns the absolute maximum of a given coordinate.
 *
 * @param coordinate Indes of the coordinate.
 * @ return the absolute maximum.
 */
  
float DataSet::getCoordinateMax(int coordinate)
{
  if(coordinate < 0 || coordinate >= 6) return -999;
  return m_maxs[coordinate];
}

