/*
 Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */
#ifndef ISOLATIONSELECTION_ISOLATIONSELECTIONTOOL_H
#define ISOLATIONSELECTION_ISOLATIONSELECTIONTOOL_H

// Framework include(s):
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"
#include <AsgTools/PropertyWrapper.h>

// Local include(s):
#include "IsolationSelection/IIsolationSelectionTool.h"
#include "IsolationSelection/IsolationWP.h"

// Forward declaration(s):
class TFile;
class TH1F;

class Interp3D;

namespace CP {

    class IsolationSelectionTool : public virtual CP::IIsolationSelectionTool, public asg::AsgTool {
        /// Create a proper constructor for Athena
        ASG_TOOL_CLASS(IsolationSelectionTool, CP::IIsolationSelectionTool)

    public:
        /// Constructor for standalone usage, but please do use the interface and ToolHandle
        IsolationSelectionTool(const std::string& name);
        /// Destructor
        virtual ~IsolationSelectionTool();

        /// Function initialising the tool
        virtual StatusCode initialize();
        /// Function finalizing the tool

        enum IsoWPType { Efficiency, Cut };
        virtual asg::AcceptData accept(const xAOD::Photon& x) const;
        virtual asg::AcceptData accept(const xAOD::Electron& x) const;
        virtual asg::AcceptData accept(const xAOD::Muon& x) const;
        virtual asg::AcceptData accept(const strObj& x) const;
        virtual asg::AcceptData accept(const xAOD::IParticle& x) const;  // for tracks, and others?

        virtual const asg::AcceptInfo& getPhotonAcceptInfo() const;
        virtual const asg::AcceptInfo& getElectronAcceptInfo() const;
        virtual const asg::AcceptInfo& getMuonAcceptInfo() const;
        virtual const asg::AcceptInfo& getObjAcceptInfo() const;

        virtual const std::vector<IsolationWP*>& getMuonWPs() const;
        virtual const std::vector<IsolationWP*>& getElectronWPs() const;
        virtual const std::vector<IsolationWP*>& getPhotonWPs() const;
        virtual const std::vector<IsolationWP*>& getObjWPs() const;

        StatusCode addWP(std::string WP, xAOD::Type::ObjectType type);
        StatusCode addWP(IsolationWP* wp, xAOD::Type::ObjectType type);
        StatusCode addMuonWP(std::string wpname);
        StatusCode addPhotonWP(std::string wpname);
        StatusCode addElectronWP(std::string wpname);
        StatusCode addUserDefinedWP(std::string WPname, xAOD::Type::ObjectType ObjType,
                                    std::vector<std::pair<xAOD::Iso::IsolationType, std::string>>& cuts, std::string key = "",
                                    IsoWPType type = Efficiency);
        StatusCode setIParticleCutsFrom(xAOD::Type::ObjectType ObjType);
        StatusCode addCutToWP(IsolationWP* wp, std::string key, const xAOD::Iso::IsolationType t, const std::string expression,
                              const xAOD::Iso::IsolationType isoCutRemap);
        StatusCode addCutToWP(IsolationWP* wp, std::string key, const xAOD::Iso::IsolationType t, const std::string expression);

        // Clearing, for very special use
        void clearPhotonWPs();
        void clearElectronWPs();
        void clearMuonWPs();
        void clearObjWPs();

    private:
        // same interface for xAOD::IParticle and StrObj -> use  template
        template <typename T> void evaluateWP(const T& x, const std::vector<IsolationWP*>& WP, asg::AcceptData& accept) const;
        void clearWPs(std::vector<IsolationWP*>& WP);

        Gaudi::Property<std::string> m_muWPname{this, "MuonWP", "Undefined", "Working point for muon"};
        Gaudi::Property<std::string> m_elWPname{this, "ElectronWP", "Undefined", "Working point for electron"};
        Gaudi::Property<std::string> m_phWPname{this, "PhotonWP", "Undefined", "Working point for photon"};
        Gaudi::Property<std::vector<std::string>> m_muWPvec{this, "MuonWPVec", {}, "Vector of working points for muon"};
        Gaudi::Property<std::vector<std::string>> m_elWPvec{this, "ElectronWPVec", {}, "Vector of working points for electron"};
        Gaudi::Property<std::vector<std::string>> m_phWPvec{this, "PhotonWPVec", {}, "Vector of working points for photon"};

        Gaudi::Property<std::string> m_muWPKey{this, "MuonKey", "/Muons/DFCommonGoodMuon/mu_cutValues_", "path of the cut map for muon"};
        Gaudi::Property<std::string> m_elWPKey{this, "ElectronKey", "/ElectronPhoton/LHTight/el_cutValues_",
                                               "path of the cut map for electron"};
        Gaudi::Property<std::string> m_phWPKey{this, "PhotonKey", "/ElectronPhoton/LHTight/el_cutValues_",
                                               "path of the cut map for photon"};

        /// input file
        Gaudi::Property<std::string> m_calibFileName{this, "CalibFileName", "", " The config to use"};
        std::unique_ptr<TFile> m_calibFile{nullptr};

        /// internal use
        std::vector<IsolationWP*> m_muWPs;
        std::vector<IsolationWP*> m_elWPs;
        std::vector<IsolationWP*> m_phWPs;
        std::vector<IsolationWP*> m_objWPs;

        /// AcceptInfo's
        asg::AcceptInfo m_photonAccept{"IsolationSelectionToolPhotonAcceptInfo"};
        asg::AcceptInfo m_electronAccept{"IsolationSelectionToolElectronAcceptInfo"};
        asg::AcceptInfo m_muonAccept{"IsolationSelectionToolMuonAcceptInfo"};
        asg::AcceptInfo m_objAccept{"IsolationSelectionToolObjAcceptInfo"};

        /// Iparticle interface
        std::vector<IsolationWP*>* m_iparWPs;
        asg::AcceptInfo* m_iparAcceptInfo{nullptr};

        // for cut interpolation
        Gaudi::Property<bool> m_doInterpM{this, "doCutInterpolationMuon", false, "flag to perform cut interpolation, muon"};
        Gaudi::Property<bool> m_doInterpE{this, "doCutInterpolationElec", true, "flag to perform cut interpolation, electron"};
        std::shared_ptr<Interp3D> m_Interp{nullptr};
    };
}  // namespace CP

#endif
