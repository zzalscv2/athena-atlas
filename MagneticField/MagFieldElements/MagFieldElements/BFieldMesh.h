/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

/**
 *
 * @class BFieldMesh
 *
 * A 3-d mesh (axes z,r,phi) representing a simple field map.
 * 
 * The mesh is made up from 'cells'
 * We hold numz, numr , numphi positions defining
 * the corners of the cells.
 *
 * Then we have
 * numz X numr X numphi field values at these corners
 *
 * The field type is templated - it may be short (for the toroid)
 * or double (for the solenoid)
 *
 * Masahiro Morii, Harvard University
 *
 * AthenaMT : RD Schaffer , Christos Anastopoulos
 */
//
#ifndef BFIELDMESH_H
#define BFIELDMESH_H

#include "CxxUtils/restrict.h"
#include "MagFieldElements/BFieldCache.h"
#include "MagFieldElements/BFieldVector.h"
#include <array>
#include <cmath>
#include <vector>

template<class T>
class BFieldMesh
{
public:
  BFieldMesh() = default;
  BFieldMesh(const BFieldMesh&) = default;
  BFieldMesh(BFieldMesh&&) = default;
  BFieldMesh& operator=(const BFieldMesh&) = default;
  BFieldMesh& operator=(BFieldMesh&&) = default;
  ~BFieldMesh() = default;
  /** @brief constructor with mesh dimensions and scale*/
  BFieldMesh(double zmin,
             double zmax,
             double rmin,
             double rmax,
             double phimin,
             double phimax,
             double bscale);

  /** @brief set mesh range/dimensions */
  void setRange(double zmin,
                double zmax,
                double rmin,
                double rmax,
                double phimin,
                double phimax);

  /** @brief set B scale */
  void setBscale(double bscale);
  /**  @brief scale B scale by a factor*/
  void scaleBscale(double factor);
  /**  @brief allocate space to vectors */
  void reserve(int nz, int nr, int nphi, int nfield);
  /**  @brief allocate space to vectors */
  void reserve(int nz, int nr, int nphi);
  /** @brief add a position for a cell corner for an  axis  */
  void appendMesh(int axis, double value);
  /** @brief append a filed vector  */
  void appendField(const BFieldVector<T>& field);
  /** @brief  build Look Up Table*/
  void buildLUT();
  /* @brief test if a point is inside this mesh*/
  bool inside(double z, double r, double phi) const;
  /* @brief get the cache corresponding to a particular cell*/
  void getCache(double z,
                double r,
                double phi,
                BFieldCache& cache,
                double scaleFactor = 1.0) const;
  /* @brief get the bfield given a point in xyz*/
  void getB(const double* ATH_RESTRICT xyz,
            double* ATH_RESTRICT B,
            double* ATH_RESTRICT deriv = nullptr) const;
  /* @brief minimum in axis*/
  double min(size_t axis) const;
  /* @brief maximum in axis*/
  double max(size_t axis) const;
  /* @brief minimum in z*/
  double zmin() const;
  /* @brief maximum in z*/
  double zmax() const;
  /* @brief minimun in r*/
  double rmin() const;
  /* @brief maximum in r*/
  double rmax() const;
  /* @brief minimun in phi*/
  double phimin() const;
  /* @brief maximum in phi*/
  double phimax() const;
  /* @brief number of corner cell coordinates for axis*/
  unsigned nmesh(size_t axis) const;
  /* @brief coordinate at axis of cell corner */
  double mesh(size_t axis, size_t index) const;
  /* @brief field entries in mesh*/
  unsigned nfield() const;
  /* @brief field vector at cell corner at index*/
  const BFieldVector<T>& field(size_t index) const;
  /* @brief scale*/
  double bscale() const;
  /* @brief memory size*/
  int memSize() const;

protected:
  std::array<double, 3> m_min;
  std::array<double, 3> m_max;
  std::array<std::vector<double>, 3> m_mesh;

private:
  std::vector<BFieldVector<T>> m_field;
  double m_scale = 1.0;
  double m_nomScale; // nominal m_scale from the map

  // look-up table and related variables
  std::array<std::vector<int>, 3> m_LUT;
  std::array<double, 3> m_invUnit; // inverse unit size in the LUT
  int m_roff, m_zoff;
};
#include "MagFieldElements/BFieldMesh.icc"
#endif
