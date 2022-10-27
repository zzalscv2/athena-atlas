/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#ifndef ISOLATIONSELECTION_IISOLATIONCLOSEBYCORRECTIONTOOL_H
#define ISOLATIONSELECTION_IISOLATIONCLOSEBYCORRECTIONTOOL_H

#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgTools/IAsgTool.h>
#include <AsgTools/CurrentContext.h>
#include <IsolationSelection/Defs.h>
#include <PATCore/AcceptData.h>
#include <PATCore/AcceptInfo.h>
#include <PATInterfaces/CorrectionCode.h>
#include <xAODEgamma/ElectronContainer.h>
#include <xAODEgamma/PhotonContainer.h>
#include <xAODMuon/MuonContainer.h>
#include <xAODPFlow/FlowElementContainer.h>

namespace CP {
    class IIsolationCloseByCorrectionTool : public virtual asg::IAsgTool {
        ASG_TOOL_INTERFACE(CP::IIsolationCloseByCorrectionTool)
    public:
        // This function calculates and applies (the particle is not redecorated) the corrections for close-by objects for each isolation
        // variables and tests whether the particle passes the isolation working point after the corrections. Note that to use this
        // functionality, a IsolationSelectionTool must have been passed to the tool (which must have been intialised indicating which
        // isolation working point to use). The result returned is a AcceptData object which is the decision made by the tool with respect
        // to the particle passing the working point.
        virtual asg::AcceptData acceptCorrected(const xAOD::IParticle& x, const xAOD::IParticleContainer& closePar) const = 0;

        // This function calculates the values of the corrections for close-by objects to be applied to the isolation variables (without
        // redecorating the particles). The corrections are returned in a vector (one correction per isolation type provided). This function
        // is intended for experts only who want to check the effects of the corrections.
        virtual CorrectionCode getCloseByCorrection(std::vector<float>& corrections, const xAOD::IParticle& par,
                                                    const std::vector<xAOD::Iso::IsolationType>& types,
                                                    const xAOD::IParticleContainer& closePar) const = 0;

        virtual CorrectionCode getCloseByIsoCorrection(const xAOD::ElectronContainer* Electrons = nullptr, const xAOD::MuonContainer* Muons = nullptr,
                                                       const xAOD::PhotonContainer* Photons = nullptr) const = 0;
        virtual CorrectionCode subtractCloseByContribution(xAOD::IParticle& x, const xAOD::IParticleContainer& closebyPar) const = 0;

        virtual float getOriginalIsolation(const xAOD::IParticle& P, IsoType type) const = 0;
        virtual float getOriginalIsolation(const xAOD::IParticle* P, IsoType type) const = 0;

        /// Retrieve the associated clusters from the Particle and calculate the average eta/phi/energy
        virtual void associateCluster(const xAOD::IParticle* particle, float& eta, float& phi, float& energy) const = 0;

        virtual void associateFlowElement(const EventContext& ctx, const xAOD::IParticle* particle, float& eta, float& phi,
                                          float& energy) const = 0;

        /// Retrieve the track particles associated with the primary particle. The tracks must be candidates to enter the track isolation
        /// variables
        virtual TrackSet getTrackCandidates(const EventContext& ctx, const xAOD::IParticle* particle) const = 0;

        /// Retrieve the reference particle to define the cone axis in which the track particles contributing to the isolation have to be
        virtual const xAOD::IParticle* isoRefParticle(const xAOD::IParticle* particle) const = 0;

        virtual ~IIsolationCloseByCorrectionTool() = default;
    };

}  // namespace CP
#endif
