/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRKVERTEXSEEDFINDERUTILIS_GAUSSIANTRACKDENSITY_H
#define TRKVERTEXSEEDFINDERUTILIS_GAUSSIANTRACKDENSITY_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "TrkVertexFitterInterfaces/IVertexTrackDensityEstimator.h"

#include <map>

#include "AthAllocators/ArenaPoolSTLAllocator.h"
namespace Trk
{

  class Track;
  class GaussianTrackDensity;

  /**
   @class GaussianTrackDensity

   Implementation of IVertexTrackDensityEstimator modeling reconstructed tracks as
   two-dimensional Gaussian distributions in (d0, z0) space and sampling
   the aggregate density distribution at user-requested points along the beam axis.
   @author Dave Casper <dcasper@uci.edu>

   @author Christos Anastopoulos (Athena MT)
  */

  class GaussianTrackDensity final: public extends<AthAlgTool, IVertexTrackDensityEstimator>
  {
  public:
    /// Inherit constructor.
    using base_class::base_class;

    /**
     * @brief Find position of global maximum for density function.
     * @param vectorTrk List of input tracks.
     */
    virtual double
    globalMaximum (const std::vector<const Track*>& vectorTrk) const override final;

    /**
     * @brief Find position of global maximum for density function.
     * @param vectorTrk List of input tracks.
     * @param density[out] Helper to hold density results.
     */
    virtual double
    globalMaximum (const std::vector<const Track*>& vectorTrk,
                   std::unique_ptr<ITrackDensity>& density) const override final;

    /**
     * @brief Find position of global maximum for density function.
     * @param perigeeList List of input tracks.
     */
    virtual double
    globalMaximum (const std::vector<const TrackParameters*>& perigeeList) const override final;

    /**
     * @brief Find position of global maximum for density function.
     * @param perigeeList List of input tracks.
     * @param density[out] Helper to hold density results.
     */
    virtual double
    globalMaximum (const std::vector<const TrackParameters*>& perigeeList,
                   std::unique_ptr<ITrackDensity>& density) const override final;

    virtual std::pair<double, double> globalMaximumWithWidth(
        const std::vector<const TrackParameters*>& perigeeList)
        const override final;

   private:
    struct TrackEntry {
      TrackEntry() = default;
      TrackEntry(const TrackEntry&) = default;
      TrackEntry(TrackEntry&&) = default;
      TrackEntry& operator=(const TrackEntry&) = default;
      TrackEntry& operator=(TrackEntry&&) = default;
      ~TrackEntry() = default;

      TrackEntry(double c0, double c1, double c2, double zMin, double zMax)
          : c_0(c0), c_1(c1), c_2(c2), lowerBound(zMin), upperBound(zMax) {}
      explicit TrackEntry(double z)
          : c_0(0), c_1(0), c_2(0), lowerBound(z), upperBound(z) {}
      // Cached information for a single track
      double c_0 = 0;  // z-independent term in exponent
      double c_1 = 1;  // linear coefficient in exponent
      double c_2 = 0;  // quadratic coefficient in exponent
      double lowerBound = 0;
      double upperBound = 0;
    };

    // helper to handle the evaluation of the parametrised track density 
    struct TrackDensityEval{
      public:
        // initialise with the z coordinate at which the density is to be evaluated 
        TrackDensityEval(double z_coordinate): m_z(z_coordinate){} 
        // add the contribution for one track to the density. 
        // will internally check if the z coordinate is within 
        // the bounds of the track 
        void addTrack (const TrackEntry & entry);
        // retrieve the density and its derivatives
        inline double density() const {return m_density;}
        inline double firstDerivative() const {return m_firstDerivative;}
        inline double secondDerivative() const {return m_secondDerivative;}
      private: 
        double m_z; 
        double m_density{0};
        double m_firstDerivative{0};
        double m_secondDerivative{0}; 
    };

    class TrackDensity final : public ITrackDensity {
       public:
        explicit TrackDensity(bool gaussStep) : m_gaussStep(gaussStep) {}
        virtual ~TrackDensity() = default;

        /**
         *  Evaluate the density function at the specified coordinate
         *  along the beamline.
         */
        virtual double trackDensity(double z) const override final;

        /**
         *  Evaluate the density and its first two derivatives
         *  at the specified coordinate.
         */
        virtual void trackDensity(
            double z, double& density, double& firstDerivative,
            double& secondDerivative) const override final;
        /**
         * @brief Return position of global maximum for density function.
         */
        double globalMaximum() const;
        /**
         * @brief Return position of global maximum with Gaussian width for
         * density function.
         */
        std::pair<double, double> globalMaximumWithWidth() const;
        /**
         * @brief Add a track to the set being considered.
         * @param itrk Track parameters.
         * @param d0SignificanceCut Significance cut on d0.
         * @param z0SignificanceCut Significance cut on z0.
         */
        void addTrack(const Perigee& itrk, const double d0SignificanceCut,
                      const double z0SignificanceCut);

       private:
        inline void updateMaximum(double trialZ, double trialValue,
                                  double secondDerivative, double& maxZ,
                                  double& maxValue,
                                  double& maxSecondDerivative) const {
        if (trialValue > maxValue) {
          maxZ = trialZ;
          maxValue = trialValue;
          maxSecondDerivative = secondDerivative;
        }
        }

        inline double stepSize(double y, double dy, double ddy) const {
        return (m_gaussStep ? (y * dy) / (dy * dy - y * ddy) : -dy / ddy);
        }
        bool m_gaussStep;
        double m_maxRange = 0;
        //  Cache for track information
        // functor to compare two Perigee values
        struct pred_perigee {
        inline bool operator()(const Perigee& left,
                               const Perigee& right) const {
          return left.parameters()[Trk::z0] < right.parameters()[Trk::z0];
        }
        };
        // functor to compare two TrackEntry values based on their upper limits (low
        // to high)
        struct pred_entry_by_max {
        inline bool operator()(const TrackEntry& left,
                               const TrackEntry& right) const {
          return left.upperBound < right.upperBound;
        }
        };

        using trackMap =
            std::map<Perigee, GaussianTrackDensity::TrackEntry, pred_perigee,
                     SG::ArenaPoolSTLAllocator<std::pair<
                         const Perigee, GaussianTrackDensity::TrackEntry>>>;

        using lowerMap = std::map<
            GaussianTrackDensity::TrackEntry, Perigee, pred_entry_by_max,
            SG::ArenaPoolSTLAllocator<
                std::pair<const GaussianTrackDensity::TrackEntry, Perigee>>>;

        trackMap m_trackMap;
        lowerMap m_lowerMap;
    };

    /**
     * @brief Find position of global maximum for density function.
     * @param pergigeeList List of input tracks.
     * @param density Helper density object.
     */
    double
    globalMaximumImpl (const std::vector<const TrackParameters*>& perigeeList,
                       TrackDensity& density) const;

    /**
     * @brief Find position of global maximum with Gaussian width for density function.
     * @param pergigeeList List of input tracks.
     * @param density Helper density object.
     */
    std::pair<double,double>
    globalMaximumWithWidthImpl (const std::vector<const TrackParameters*>& perigeeList,
                       TrackDensity& density) const;


    /**
     * @brief Add a set of tracks to a density object.
     * @param perigeeList Set of track parameters to add.
     * @param density Density object to which to add.
     */
    void addTracks(const std::vector<const TrackParameters*>& perigeeList,
                   TrackDensity& density) const;


 
    //  Cuts set by configurable properties
    
    //  Maximum allowed d0 significance to use (in sigma)
    Gaudi::Property<double> m_d0MaxSignificance{
      this,
      "MaxD0Significance",
      3.5,
      "Maximum radial impact parameter significance to use track"
    };

    // Tracks within this many sigma(z) are added to weight; increasing cut
    // trades CPU efficiency for improved smoothness in tails
    Gaudi::Property<double> m_z0MaxSignificance{
      this,
      "MaxZ0Significance",
      12.0,
      "Maximum longitudinal impact parameter significance to include track in "
      "weight"
    };

    // Assumed shape of density function near peak; Gaussian (true) or parabolic
    // (false)
    Gaudi::Property<bool> m_gaussStep{
      this,
      "GaussianStep",
      true,
      "Peak search: True means assume Gaussian behavior, False means "
      "Newton/parabolic"
    };
  };
}
#endif
