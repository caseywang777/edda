/////////////////////////////////////////////////////////////////////////////
//
//                 OSU Flow Vis Library
//                 Created: Han-Wei Shen, Liya Li 
//                 The Ohio State University	
//                 Date:		06/2005
//                 Streamlines
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _GRID_H_
#define _GRID_H_

#include <cassert>
#include <vector>
#include "common.h"
//#include "Element.h"
#include "Interpolator.h"
#include "VectorMatrix.h"
//#include "Cell.h"

namespace edda{

enum CellType
{
	TRIANGLE,
	CUBE,
	POLYGON,
	TETRAHEDRON
};

// define the cell type
enum CellTopoType
{
	T0_CELL,					// vertex
	T1_CELL,					// edge
	T2_CELL,					// triangle, quarilateral
	T3_CELL,					// tetrahedra, cube
	T4_CELL						// hetrahedra, added by lijie
};

enum SliceType
{
	X_ALIGNED,
	Y_ALIGNED,
	Z_ALIGNED
};




//////////////////////////////////////////////////////////////////////////
//
// base class for grid
//
//////////////////////////////////////////////////////////////////////////
class Grid
{
public:
    Grid() {}
    virtual ~Grid() {}
	// physical coordinate of vertex verIdx
    virtual ReturnStatus at_vertex(int verIdx, VECTOR3& pos) = 0;
    // check whether the physical point is defined
	virtual bool at_phys(VECTOR3& pos) = 0;			
	// get vertex list of a cell
    virtual ReturnStatus getCellVertices(int cellId, std::vector<int>& vVertices) = 0;
	// get the cell id and also interpolating coefficients for the given physical position
    virtual ReturnStatus phys_to_cell(PointInfo& pInfo) = 0;
	// interpolation
    virtual void interpolate(VECTOR3& nodeData, std::vector<VECTOR3>& vData, VECTOR3 coeff) = 0;
	// the volume of cell
    virtual double cellVolume(int cellId) = 0;
	// type of cell
	virtual CellType GetCellType(void) = 0;
	// get min and maximal boundary
	virtual void Boundary(VECTOR3& minB, VECTOR3& maxB) = 0;
	// set bounding box
	virtual void SetBoundary(VECTOR3& minB, VECTOR3& maxB) = 0;
	// get grid spacing in x,y,z dimensions
	virtual void GetGridSpacing(int cellId, float& xspace, float& yspace, float& zspace) = 0;
	// boundary intersection
	virtual void BoundaryIntersection(VECTOR3& intersectP, VECTOR3& startP, 
					  VECTOR3& endP,float* stepSize, float oldStepSize) = 0;
	// whether the point is in the bounding box
	virtual bool isInBBox(VECTOR3& pos) = 0;
	// whether the point is in the bounding box not counting ghost cells
	virtual bool isInRealBBox(VECTOR3& p) = 0;
	virtual bool isInRealBBox(VECTOR3& pos, float t) = 0;

protected:
	// reset parameters
	virtual void Reset(void) = 0;
	// compute bounding box
	virtual void ComputeBBox(void) = 0;
	// whether in a cell
	virtual bool isInCell(PointInfo& pInfo, const int cellId) = 0;
};

//////////////////////////////////////////////////////////////////////////
//
// Cartesian Grid (Regular and Irregular)
//
//////////////////////////////////////////////////////////////////////////
class CartesianGrid : public Grid
{
public:
	// constructor and destructor
    CartesianGrid(int xdim, int ydim, int zdim);
	CartesianGrid();
	~CartesianGrid();
    virtual void GetDimension(int& xdim, int& ydim, int& zdim);

#if 0
	// physical coordinate of vertex verIdx
    virtual ReturnStatus at_vertex(int verIdx, VECTOR3& pos) =0;
	// whether the physical point is in the boundary
	virtual bool at_phys(VECTOR3& pos) =0; 
	// get vertex list of a cell
    virtual ReturnStatus getCellVertices(int cellId, std::vector<int>& vVertices) =0;
	// get the cell id and also interpolating coefficients for the given physical position
    virtual ReturnStatus phys_to_cell(PointInfo& pInfo) =0;
	
	// interpolation
    template <class T>
    virtual void interpolate(T& nodeData, std::vector<T>& vData, VECTOR3 coeff) =0;
	// the volume of cell
    virtual double cellVolume(int cellId) = 0;
	// type of cell
	virtual CellType GetCellType(void) = 0; 
	// get min and maximal boundary
	virtual void Boundary(VECTOR3& minB, VECTOR3& maxB) = 0; 
	// set bounding box (includes ghost cells)
	virtual void SetBoundary(VECTOR3& minB, VECTOR3& maxB) = 0; 
	// set bounding box (does not include ghost cells)
	virtual void SetRealBoundary(VECTOR4& minB, VECTOR4& maxB) = 0;
	// get grid spacing in x,y,z dimensions
	virtual void GetGridSpacing(int cellId, float& xspace, float& yspace, float& zspace) = 0; 
	// boundary intersection
	virtual void BoundaryIntersection(VECTOR3& intersectP, VECTOR3& startP, 
					  VECTOR3& endP,float* stepSize, float oldStepSize) = 0; 
#endif
protected:
	// reset parameters
	void Reset(void);
	// dimension related
	inline int xdim(void) { return m_nDimension[0];}
	inline int ydim(void) { return m_nDimension[1];}
	inline int zdim(void) { return m_nDimension[2];}
	inline int xcelldim(void) {return (m_nDimension[0] - 1);}
	inline int ycelldim(void) {return (m_nDimension[1] - 1);}
	inline int zcelldim(void) {return (m_nDimension[2] - 1);}

	int m_nDimension[3];				// dimension

	// min and maximal boundary (includes ghost cells)
	VECTOR3 m_vMinBound, m_vMaxBound;

	// min and maximal boundary (does not include ghost cells)
	VECTOR4 m_vMinRealBound, m_vMaxRealBound;
};

//////////////////////////////////////////////////////////////////////////
//
// regular cartesian grid
//
//////////////////////////////////////////////////////////////////////////
// map coordinates in computational space to physical space
#define UCGridPhy2Comp(x, y, f) (((x) - (y))*(f))

class RegularCartesianGrid : public CartesianGrid
{
private:
	float mappingFactorX;				// mapping from physical space to computational space
	float mappingFactorY;
	float mappingFactorZ;
	float oneOvermappingFactorX;
	float oneOvermappingFactorY;
	float oneOvermappingFactorZ;
	float gridSpacing;			        // the minimal grid spacing of all dimensions

public:
	RegularCartesianGrid(int xdim, int ydim, int zdim);
	RegularCartesianGrid();
	~RegularCartesianGrid();
	// physical coordinate of vertex verIdx
    ReturnStatus at_vertex(int verIdx, VECTOR3& pos);
	// whether the physical point is in the boundary
	bool at_phys(VECTOR3& pos);			
	// get vertex list of a cell
    ReturnStatus getCellVertices(int cellId, std::vector<int>& vVertices);
	// get the cell id and also interpolating coefficients for the given physical position
    ReturnStatus phys_to_cell(PointInfo& pInfo);
	// interpolation
    template<class T>
    void interpolate(T &nodeData, std::vector<T>& vData, VECTOR3 coeff);
	// the volume of cell
    double cellVolume(int cellId);
	// cell type
	CellType GetCellType(void) {return CUBE;}
	// set bounding box (includes ghost cells)
	void SetBoundary(VECTOR3& minB, VECTOR3& maxB);
	// set bounding box (does not include ghost cells)
	void SetRealBoundary(VECTOR4& minB, VECTOR4& maxB);
	// get min and maximal boundary
	void Boundary(VECTOR3& minB, VECTOR3& maxB);
	// get grid spacing in x,y,z dimensions
	void GetGridSpacing(int cellId, float& xspace, float& yspace, float& zspace) 
	{ xspace = oneOvermappingFactorX; yspace = oneOvermappingFactorY; zspace = oneOvermappingFactorZ; }
	void BoundaryIntersection(VECTOR3&, VECTOR3&, VECTOR3&, float*, float);
	// whether the point is in the bounding box
	bool isInBBox(VECTOR3& pos);
	// whether the point is in the bounding box (not counting ghost cells)
	bool isInRealBBox(VECTOR3& pos);
	bool isInRealBBox(VECTOR3& pos, float t);


protected:
	void Reset(void);
	// compute bounding box
	void ComputeBBox(void);
	// whether in a cell
	bool isInCell(PointInfo& pInfo, const int cellId);
};

/* 
//Comment the following code out since they have not been implemented
// 
//////////////////////////////////////////////////////////////////////////
//
//	irregular cartesian grid
//
//////////////////////////////////////////////////////////////////////////
class IrregularCartesianGrid : public CartesianGrid
{
private:
	float* m_pXSpacing;			// space array for x, y, z dimension
	float* m_pYSpacing;
	float* m_pZSpacing;

public:
	IrregularCartesianGrid(int xdim, int ydim, int zdim);
	IrregularCartesianGrid();
	~IrregularCartesianGrid();

protected:
	void Reset(void);
};

//////////////////////////////////////////////////////////////////////////
//
// curvilinear grid
//
//////////////////////////////////////////////////////////////////////////
class CurvilinearGrid : public Grid
{
private:
	int m_nDimension[3];				// dimension

public:
	// constructor and deconstructor
	CurvilinearGrid(int xdim, int ydim, int zdim);
	CurvilinearGrid();
	~CurvilinearGrid();
};

*/ 

//////////////////////////////////////////////////////////////////////////
//
// irregular grid
//
//////////////////////////////////////////////////////////////////////////
class IrregularGrid : public Grid
{
private:
	int m_nNodeNum;						// number of nodes
	int m_nTetraNum;					// number of tetras
	CVertex* m_pVertexGeom;				// geometry of all vertices
	CTetra* m_pTetra;					// tetra
	TetraInfo* m_pTetraInfo;			// pre-computed tetra information
	TVertex* m_pVertexTopo;				// vertex topology
	VECTOR3 m_vMinBound, m_vMaxBound;	// min and maximal boundary
	bool m_bTetraInfoInit;				// whether the tetra information is pre-computed

public:
	// constructor and deconstructor
	IrregularGrid();
	IrregularGrid(int nodeNum, int tetraNum, CVertex* pVertexGeom, CTetra* pTetra, TVertex* pVertexTopo);
	~IrregularGrid();

	// from virtual functions
    virtual void Reset(void);
    virtual void GetDimension(int& xdim, int& ydim, int& zdim) ;
    virtual ReturnStatus at_vertex(int verIdx, VECTOR3& pos);
    virtual bool at_phys(VECTOR3& pos);
    virtual ReturnStatus getCellVertices(int cellId, std::vector<int>& vVertices);
    virtual ReturnStatus phys_to_cell(PointInfo& pInfo);
    virtual void interpolate(VECTOR3& nodeData, std::vector<VECTOR3>& vData, VECTOR3 coeff);
    virtual double cellVolume(int cellId);
    virtual bool isInCell(PointInfo& pInfo, const int cellId);
    virtual CellType GetCellType(void) {return TETRAHEDRON;}

    virtual void ComputeBBox(void);
    virtual void SetBoundary(VECTOR3& minB, VECTOR3& maxB);
    virtual void Boundary(VECTOR3& minB, VECTOR3& maxB);
    virtual bool isInBBox(VECTOR3& pos);
    virtual bool isInRealBBox(VECTOR3& pos);
    virtual bool isInRealBBox(VECTOR3& pos, float t);

	// irregular specific functions
    virtual void SetTetraInfoInit(bool bInit);
    virtual bool GetTetraInfoInit(void);
    virtual int nextTetra(PointInfo& pInfo, int tetraId);
    virtual void PreGetP2NMatrix(MATRIX3& m, int cellId);
    virtual bool Physical2NaturalCoord(VECTOR3& nCoord, VECTOR3& pCoord, int cellId);

    virtual void GetGridSpacing(int cellId, float& xspace, float& yspace, float& zspace) {assert(false);}
	virtual void BoundaryIntersection(VECTOR3& intersectP, VECTOR3& startP, 
                      VECTOR3& endP,float* stepSize, float oldStepSize){assert(false);}

};

float getStepSize(VECTOR3& p, VECTOR3& p1, VECTOR3& p2, float oldStepSize);

}  // namesapce edda
#endif