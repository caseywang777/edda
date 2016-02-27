#include <thrust/iterator/counting_iterator.h>
#include <thrust/transform.h>
#include "uncertain_isocontour.h"

#ifndef __CUDACC__
#define __constant__
#endif

namespace edda{

class LevelCrossingFunctor {
  int dim[3];
  int celldim[3];
  const double isov;
  const detail::MakeStridedGmm makeStridedGmm;
public:
  LevelCrossingFunctor(const detail::MakeStridedGmm &m, int dim_[3], double isov_)
    : makeStridedGmm(m), isov(isov_)
  {
    dim[0] = dim_[0];
    dim[1] = dim_[1];
    dim[2] = dim_[2];
    celldim[0] = dim[0]-1;
    celldim[1] = dim[1]-1;
    celldim[2] = dim[2]-1;
  }


  __host__ __device__
  float operator() (int idx) {
    int i = idx % celldim[0];
    int j = (idx / celldim[0]) % celldim[1];
    int k = idx / celldim[0] / celldim[1];
    float cdf[8];
#define IJK_TO_IDX(i,j,k)  (i+dim[0]*(j+dim[1]*(k)))
    cdf[0] = dist::getCdf( makeStridedGmm( IJK_TO_IDX(i  ,j  ,k  )),  isov);
    cdf[1] = dist::getCdf( makeStridedGmm( IJK_TO_IDX(i+1,j  ,k  )),  isov);
    cdf[2] = dist::getCdf( makeStridedGmm( IJK_TO_IDX(i  ,j+1,k  )),  isov);
    cdf[3] = dist::getCdf( makeStridedGmm( IJK_TO_IDX(i+1,j+1,k  )),  isov);
    cdf[4] = dist::getCdf( makeStridedGmm( IJK_TO_IDX(i  ,j  ,k+1)),  isov);
    cdf[5] = dist::getCdf( makeStridedGmm( IJK_TO_IDX(i+1,j  ,k+1)),  isov);
    cdf[6] = dist::getCdf( makeStridedGmm( IJK_TO_IDX(i  ,j+1,k+1)),  isov);
    cdf[7] = dist::getCdf( makeStridedGmm( IJK_TO_IDX(i+1,j+1,k+1)),  isov);
#undef IJK_TO_IDX
    float prob1=1., prob2=1.;
    for (int l=0; l<8; l++) {
      prob1 *= cdf[l];
      prob2 *= 1.-cdf[l];
    }
    return 1.-prob1-prob2;
  }
};

#if 0 // debug
class LevelCrossingFunctor {
  GmmNdArray *gmmArray;
  int dim[3];
  int celldim[3];
  double isov;
public:
  LevelCrossingFunctor() {}

  __host__ __device__
  float operator() (int idx) {
    return (float)idx;
  }
};
#endif


ReturnStatus levelCrossing(std::shared_ptr<GmmNdArray> gmmArray_, int dim[3],
                           double isov, std::shared_ptr<NdArray<float> > &probField)
{
  int new_dim[3]={dim[0]-1, dim[1]-1, dim[2]-1};
  int newsize = new_dim[0]*new_dim[1]*new_dim[2];

  probField.reset( new NdArray<Real>(3, new_dim) );

  thrust::transform( thrust::make_counting_iterator(0),
                     thrust::make_counting_iterator(newsize),
                     probField->begin(),
                     LevelCrossingFunctor( gmmArray_->getMakeStridedGmm(), dim, isov)
                     );

  return ReturnStatus::SUCCESS;
}


ReturnStatus levelCrossingSerial(AbstractDataArray *input, int dim[3], double isov, float *probField)
{
  int i,j,k;
  int count=0;
  int new_dim[3]={dim[0]-1, dim[1]-1, dim[2]-1};

  if (input->getLength() < dim[0]*dim[1]*dim[2]) {
    return ReturnStatus::FAIL;
  }

  //precompute cdf for speedup
  shared_ary<float> cdfField (dim[0]*dim[1]*dim[2]);
  count = 0;
  for (k=0; k<dim[2]; k++)
    for (j=0; j<dim[1]; j++)
      for (i=0; i<dim[0]; i++) {
        // compute level crossing
        cdfField[count] = dist::getCdf( input->getScalar(count) , isov);
        count++;
      }

  count = 0;
  for (k=0; k<new_dim[2]; k++)
    for (j=0; j<new_dim[1]; j++)
      for (i=0; i<new_dim[0]; i++) {
        // compute level crossing
        // = 1 - prob. of all larger than isovalue - prob. of all less than isovalue
        double cdf[8];
#define IJK_TO_IDX(i,j,k)  (i+dim[0]*(j+dim[1]*(k)))
        cdf[0] = cdfField[IJK_TO_IDX(i  ,j  ,k)];
        cdf[1] = cdfField[IJK_TO_IDX(i+1,j  ,k  )];
        cdf[2] = cdfField[IJK_TO_IDX(i  ,j+1,k  )];
        cdf[3] = cdfField[IJK_TO_IDX(i+1,j+1,k  )];
        cdf[4] = cdfField[IJK_TO_IDX(i  ,j  ,k+1)];
        cdf[5] = cdfField[IJK_TO_IDX(i+1,j  ,k+1)];
        cdf[6] = cdfField[IJK_TO_IDX(i  ,j+1,k+1)];
        cdf[7] = cdfField[IJK_TO_IDX(i+1,j+1,k+1)];
#undef IJK_TO_IDX
        double prob1=1., prob2=1.;
        for (int l=0; l<8; l++) {
          prob1 *= cdf[l];
          prob2 *= 1.-cdf[l];
        }

        probField[count] = 1.-prob1-prob2;
        count++;
      }

  return ReturnStatus::SUCCESS;
}

} // edda
