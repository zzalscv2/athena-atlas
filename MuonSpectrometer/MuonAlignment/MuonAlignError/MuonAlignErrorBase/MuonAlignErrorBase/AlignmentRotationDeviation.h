/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNERRORBASE_ALIGNMENTROTATIONDEVIATION_H
#define MUONALIGNERRORBASE_ALIGNMENTROTATIONDEVIATION_H

#include "MuonAlignErrorBase/AlignmentDeviation.h"

namespace MuonAlign {
    class AlignmentRotationDeviation : public Trk::AlignmentDeviation {
    public:
        AlignmentRotationDeviation(Amg::Vector3D center, Amg::Vector3D axis, double sigma,
                                   const std::vector<const Trk::RIO_OnTrack*>& hits);

        virtual ~AlignmentRotationDeviation() = default;

        /**
         * The number of free parameters
         */
        virtual int nPar() const;

        /**
         * The error matrix on the free parameters. Track fitters should use this
         * to compute a constraint on the free parameters.
         */
        virtual double getCovariance(int ipar, int jpar) const;

        /**
         * Return a Transform in the global coordinate system, given a list of
         * parameters.
         */
        virtual Amg::Transform3D getTransform(const std::vector<double>& parameters) const;

        /**
         * Verbose
         */
        virtual void print(std::ostream& out) const;

        /**
        * Get the rotation center
        */
        const Amg::Vector3D& getCenter() const { return m_center; }

        /**
        * Get the rotation axis
        */
        const Amg::Vector3D& getAxis() const { return m_axis; }

        /**
        * Get the error to apply to this NP
        */
        double getSigma() const { return m_sigma; }

    private:
        Amg::Vector3D m_center;
        Amg::Vector3D m_axis;
        double m_sigma;
    };
}  // namespace MuonAlign

#endif
