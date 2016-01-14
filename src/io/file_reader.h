#ifndef FILE_READER_H_
#define FILE_READER_H_

#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include "dataset.h"
#include "distributions/gaussian.h"
#include "distributions/distribution.h"
#include "core/shared_ary.h"
#include "io/path.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>

namespace edda{

typedef dist::Gaussian<float> Gaussianf;
typedef Vector3<Gaussianf> Gaussianf3;

template<typename T>
shared_ary<T> loadRawFile(const std::string &fname, size_t len) {
  FILE * fIn;
  T *pData;

  fIn = fopen(fname.c_str(), "rb");
  assert(fIn != NULL);
  pData = new T[len];
  size_t read_len = fread(pData, sizeof(T), len, fIn);
  fclose(fIn);
  assert(len == read_len);
  return shared_ary<T>(pData, len);
}

shared_ary<Gaussianf> loadGaussianRawArray(std::string meanfile, std::string stdfile, size_t len) ;
shared_ary<Gaussianf3> loadVec3GaussianRawArray(std::string meanfile, std::string stdfile, size_t len);

//----------------------------------------------------------------------------------------------------------
// dataset creators

/// \brief create a regular grid dataset of gaussian distribution
std::shared_ptr<Dataset<Gaussianf> > loadGaussianRegularGrids(std::string &meanfile, std::string &stdfile, int dim[3]);

/// \brief create a regular grid dataset of random values from gaussian distributions
std::shared_ptr<Dataset<double> > loadGaussianSamplingRegularGrids(std::string &meanfile, std::string &stdfile, int dim[3]);

/// \brief create a regular grid dataset of 3d gaussian distribution
std::shared_ptr<Dataset<Gaussianf3> > loadVec3GaussianRegularGrids(std::string &meanfile, std::string &stdfile, int dim[3]);

/// \brief create a regular grid dataset of random samples from 3d gaussian distribution
std::shared_ptr<Dataset<VECTOR3> > loadVec3GaussianSamplingRegularGrids(std::string &meanfile, std::string &stdfile, int dim[3]);

void print(boost::property_tree::ptree const& pt)
{
    boost::property_tree::ptree::const_iterator end = pt.end();
    std::cout << "{" << std::endl;
    for (boost::property_tree::ptree::const_iterator it = pt.begin(); it != end; ++it) {
        std::cout << it->first << ": " << it->second.get_value<std::string>() << std::endl;
        print(it->second);
    }
    std::cout << "}" << std::endl;
}

template<typename T, class GetItemPolicy=GetItemAsIs>
std::shared_ptr<Dataset<T> > loadData(std::string filename)
{
  std::ifstream myfile(filename);
  std::string filepath = getPath(filename);

  if (myfile) {
    std::stringstream ss;
    ss << myfile.rdbuf();
    myfile.close();
    boost::property_tree::ptree pt;
    boost::property_tree::read_info(ss, pt);

    print(pt);

    // grid
    std::string gtype = pt.get_child("gtype").get_value<std::string>();
    //std::cout << "gtype: " <<  gtype << std::endl;
    Grid * grid;
    int array_size = 0;
    if (gtype.compare("regular")==0) {
      int xdim = pt.get_child("xdim").get_value<int>();
      int ydim = pt.get_child("ydim").get_value<int>();
      int zdim = pt.get_child("zdim").get_value<int>();

      grid = new RegularCartesianGrid(xdim, ydim, zdim);
      array_size = xdim * ydim * zdim;
    } else {
      std::cout << "gtype not supported: " << gtype << std::endl;
      exit(1);
    }

    // data array
    std::string dtype = pt.get_child("dtype").get_value<std::string>();
    int tuples = pt.get_child("tuples").get_value<int>();
    AbstractDataArray *data_array = NULL;
    if (dtype.compare("gaussian")==0) {
      std::string mfile = pt.get_child("mfile").get_value<std::string>();
      std::string sfile = pt.get_child("sfile").get_value<std::string>();
      if (isFilenameOnly(mfile)) mfile = filepath + "/" + mfile;
      if (isFilenameOnly(sfile)) sfile = filepath + "/" + sfile;
      if (tuples == 1) {
        shared_ary<Gaussianf> array = loadGaussianRawArray(mfile, sfile, array_size);
        data_array = new DataArray<Gaussianf, GetItemPolicy> (array);
      } else if (tuples == 3) {
        shared_ary<Gaussianf3> array = loadVec3GaussianRawArray(mfile, sfile, array_size);
        data_array = new DataArray<Gaussianf3, GetItemPolicy> (array);
      } else {
        std::cout << "tuple size " << tuples << "not supported." << std::endl;
        exit(1);
      }
    } else {
      std::cout << "gtype not supported: " << gtype << std::endl;
      exit(1);
    }

    return std::shared_ptr<Dataset<T> > (new Dataset<T> (grid, data_array) );
  }  else {
    std::cout << "Cannot open file: %s" << filename.c_str() << std::endl;
  }

  exit(1);
}

} // edda


#endif // FILE_READER_H_
