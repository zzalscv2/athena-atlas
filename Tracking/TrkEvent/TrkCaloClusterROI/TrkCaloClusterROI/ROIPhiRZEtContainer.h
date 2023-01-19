/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ROIPHIRZETCOLLECTION_H
#define ROIPHIRZETCOLLECTION_H

#include "ROIPhiRZContainer.h"
#include "TVector2.h"

// class to hold ROIs defined by Phi, r, z and Et
class ROIPhiRZEt : public std::array<float,4>
{
public:
   enum ERoiParameters {kPhi, kR, kZ, kEt};
   inline double eta() const {
      const double R   = r();
      const double Z   = z();
      return std::atanh( Z / std::sqrt( R*R + Z*Z ) );
   }
   inline double theta() const {
      double Z = z();
      double R = r();
      return std::atan2(1., Z / R);
   }
   inline float phi() const {
      return (*this)[kPhi];
   }
   inline float r() const {
      return (*this)[kR];
   }
   inline float z() const {
      return (*this)[kZ];
   }
   inline float Et() const {
      return (*this)[kEt];
   }
};

/** @brief container for phi sorted ROIs defined by phi, r, z and Et.
*/
class ROIPhiRZEtContainer : public std::vector<ROIPhiRZEt>
{
public:

   inline bool hasMatchingROI_optimized( float phi, double eta, double r, double z, float roi_Et_min, float roi_r_width) const {
      return hasMatchingROI_optimized(*this,phi,eta,r,z,roi_Et_min,roi_r_width);
   }

   inline std::vector<float> getClosestROIdREt( float phi, double eta, double r, double z, float roi_Et_min) const {
      return getClosestROIdREt(*this,phi,eta,r,z,roi_Et_min);
   }

   void addROI(const Amg::Vector3D &global_position, float roi_phi_width, float roi_Et) {

      float phi = global_position.phi();
      assert (std::abs(phi)<=M_PI );
      float z = global_position.z();
      float r = global_position.perp();
      if ( std::abs(phi) > M_PI - roi_phi_width) {
         constexpr float pi_2 = 2*M_PI;
         float sign_phi_times_2_pi = std::copysign(pi_2,phi);
         // wrap ROIs close to -pi and pi around. Thus when searching for the lower bound ROI for phi-phi_width
         // ROIs close to -pi and ROIs close to +pi will be found.
         this->emplace_back( ROIPhiRZEt{phi - sign_phi_times_2_pi, r, z, roi_Et} );
     }
     this->emplace_back( ROIPhiRZEt{phi, r, z, roi_Et});
   }

   void sort() {
      // sort output ROIs by phi
      std::sort( this->begin(), this->end(), order );
   }

   static inline double eta(const ROIPhiRZEt &roi) {
      const double R   = roi.r();
      const double Z   = roi.z();
      return std::atanh( Z / std::sqrt( R*R + Z*Z ) );
   }
   static inline double theta(const ROIPhiRZEt &roi) {
      return std::atan2(1., roi.z() / roi.r());
   }
   static inline float phi(const ROIPhiRZEt &roi) {
      return roi.phi();
   }
   static inline float Et(const ROIPhiRZEt &roi) {
      return roi.Et();
   }

protected:

   /** @brief Helper function to compute a z position corrected delta eta.
    */
   static inline double deltaEta(const ROIPhiRZEt &roi, double other_r, double other_z, double other_eta) {
      //Correct eta of ROI to take into account the z postion of the reference
      double newR   = roi.r() - other_r;
      double newZ   = roi.z() - other_z;
      double newEta =  std::atanh( newZ / std::sqrt( newR*newR + newZ*newZ ) );
      double delta_eta = std::abs(newEta - other_eta);
      return delta_eta;
   }

   /** Helper function to order ROIs defined by phi,r,z,Et by phi.
    */
   static inline bool order(const ROIPhiRZEt &a, const ROIPhiRZEt &b) { return a.phi() < b.phi(); }

   static inline bool hasMatchingROI_optimized( const ROIPhiRZEtContainer &rois, float phi, double eta, double r, double z, float roi_et_min, float roi_r_width) {
      // simple dR check
      for (ROIPhiRZEtContainer::const_iterator roi_iter = rois.begin(); roi_iter != rois.end(); ++roi_iter) {
         if (roi_iter->Et() < roi_et_min) {continue;}
         float deta = deltaEta(*roi_iter, r, z, eta);
         float dphi = TVector2::Phi_mpi_pi(phi-roi_iter->phi());
         float dr = std::sqrt(deta*deta+dphi*dphi);
         if ( dr < roi_r_width) {return true;}
      }
      return false;
   }

   static inline std::vector<float> getClosestROIdREt( const ROIPhiRZEtContainer &rois, float phi, double eta, double r, double z, float roi_et_min) {
      // get closest ROI
      float min_dR = 9999999;
      float et = -2;
      for (ROIPhiRZEtContainer::const_iterator roi_iter = rois.begin(); roi_iter != rois.end(); ++roi_iter) {
         if (roi_iter->Et() < roi_et_min) {
            continue;
         }
         float deta = deltaEta(*roi_iter, r, z, eta);
         float dphi = TVector2::Phi_mpi_pi(phi-roi_iter->phi());
         float dr = std::sqrt(deta*deta+dphi*dphi);
         if ( dr < min_dR) {
            min_dR = dr;
            et = roi_iter->Et();   
         }
      }
      std::vector<float> dREt{min_dR, et};
      return dREt;
   }
};

CLASS_DEF( ROIPhiRZEtContainer , 1263624698 , 1 )

#endif
