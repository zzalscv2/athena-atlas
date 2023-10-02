/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISOLATIONTOOL_CALOISOLATIONTOOL_H
#define ISOLATIONTOOL_CALOISOLATIONTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgTools/ToolHandle.h"
#include "AsgDataHandles/ReadHandleKey.h"

#ifndef XAOD_ANALYSIS
// #include "GaudiKernel/ToolHandle.h"
#include "ParticlesInConeTools/ICaloClustersInConeTool.h"
#include "ParticlesInConeTools/IFlowElementsInConeTool.h"
#include "RecoToolInterfaces/IParticleCaloCellAssociationTool.h"
#include "RecoToolInterfaces/IParticleCaloExtensionTool.h"
#include "TrkParametersIdentificationHelpers/TrackParametersIdHelper.h"
#include "CaloUtils/CaloClusterProcessor.h"
#include "TrkCaloExtension/CaloExtensionCollection.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloDetDescr/CaloDetDescrManager.h"
#endif // XAOD_ANALYSIS

#include "IsolationCorrections/IIsolationCorrectionTool.h"
#include "RecoToolInterfaces/ICaloCellIsolationTool.h"
#include "RecoToolInterfaces/ICaloTopoClusterIsolationTool.h"
#include "RecoToolInterfaces/INeutralEFlowIsolationTool.h"
#include "RecoToolInterfaces/IsolationCommon.h"

#include "xAODTracking/TrackParticle.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODBase/IParticle.h"
#include "xAODPrimitives/IsolationType.h"
#include "xAODEgamma/Egamma.h"
#include "xAODMuon/Muon.h"
#include "xAODEventShape/EventShape.h"
#include "xAODPFlow/FlowElement.h"
#include "xAODPFlow/FlowElementContainer.h"

#include <vector>

#include <map>
#include <TGraph.h>

namespace xAOD {
  
  class CaloIsolationTool: 
    virtual public ICaloTopoClusterIsolationTool,
    virtual public ICaloCellIsolationTool,
    virtual public INeutralEFlowIsolationTool,
    public asg::AsgTool {
    ASG_TOOL_CLASS3( CaloIsolationTool, ICaloTopoClusterIsolationTool, ICaloCellIsolationTool, INeutralEFlowIsolationTool )
  public:
      CaloIsolationTool(const std::string& name);
      ~CaloIsolationTool(void); // destructor
      
      virtual StatusCode initialize() override;
      virtual StatusCode finalize()
#ifndef XAOD_ANALYSIS
        override
#endif
        ;
      
      /**ICaloTopoClusterIsolationTool interface: */    
      virtual
      bool caloTopoClusterIsolation( CaloIsolation& result, 
				     const IParticle& tp, 
				     const std::vector<Iso::IsolationType>& cones, 
				     const CaloCorrection& corrections, 
				     const CaloClusterContainer* container = 0) const override; 
      using ICaloTopoClusterIsolationTool::caloTopoClusterIsolation;

      ///// *ICaloCellIsolationTool interface: */    
      virtual
      bool caloCellIsolation(CaloIsolation& result, const IParticle& particle, 
            const std::vector<Iso::IsolationType>& cones, 
            const CaloCorrection& corrections, 
            const CaloCellContainer* container = 0) const override;
      using ICaloCellIsolationTool::caloCellIsolation;
   
      
      /**INeutralEFlowIsolationTool interface: */    
      virtual
      bool neutralEflowIsolation( CaloIsolation& result, 
				  const IParticle& tp, 
				  const std::vector<Iso::IsolationType>& cones, 
				  const CaloCorrection& corrections) const override;

      using INeutralEFlowIsolationTool::neutralEflowIsolation;

  private:
      /** map to the orignal particle */
      // This never seems to have more than one entry???
      typedef std::map<const IParticle*, const IParticle*> derefMap_t;

      /** cast for Muon (etcone muon) */
      bool caloCellIsolation( CaloIsolation& result,
#ifndef XAOD_ANALYSIS
			      const Muon& muon,
#endif
			      const std::vector<Iso::IsolationType>& cones, const CaloCorrection& corrections
#ifndef XAOD_ANALYSIS
			      , double coneCoreSize
			      , const derefMap_t& derefMap
#endif
			      ) const;

      /** cast for egamma (etcone egamma)*/    
      bool caloCellIsolation( CaloIsolation& result, const Egamma& tp, const std::vector<Iso::IsolationType>& cones, const CaloCorrection& corrections
#ifndef XAOD_ANALYSIS
      , const CaloCellContainer* container
#endif
      ) const;

      /** cast for TrackParticle (topoetcone muon)*/
      bool caloTopoClusterIsolation(
        CaloIsolation& result,
        const TrackParticle& tp,
        const std::vector<Iso::IsolationType>& cones,
        const CaloCorrection& corrections,
        const CaloClusterContainer* container,
        double coneCoreSize,
        derefMap_t& derefMap) const;

      /** cast for egamma (topoetcone egamma)*/
      bool caloTopoClusterIsolation(
        CaloIsolation& result,
        const Egamma& tp,
        const std::vector<Iso::IsolationType>& cones,
        const CaloCorrection& corrections,
        const CaloClusterContainer* container,
        double coreConeSize) const;

      /** cast for egamma (pflowetcone egamma)*/
      bool neutralEflowIsolation(CaloIsolation& result,
                                 const Egamma& eg,
                                 const std::vector<Iso::IsolationType>& cones,
                                 const CaloCorrection& corrections,
                                 double coneCoreSize) const ;

      /** cast for egamma (pflowetcone egamma)*/
      bool neutralEflowIsolation(CaloIsolation& result,
                                 const TrackParticle& tp,
                                 const std::vector<Iso::IsolationType>& cones,
                                 const CaloCorrection& corrections,
                                 double coneCoreSize,
                                 derefMap_t& derefMap) const;

      // etcone computation for TrackParticle
#ifndef XAOD_ANALYSIS
      bool etConeIsolation( CaloIsolation& result, const TrackParticle& tp, 
			    const std::vector<Iso::IsolationType>& isoTypes, 
			    const CaloCellContainer* container,
                            double coneCoreSize,
                            const derefMap_t& derefMap) const;
#endif

      // etcone computation for TrackParticle
#ifndef XAOD_ANALYSIS
      bool etConeIsolation( CaloIsolation& result, const Muon& muon,
                            const std::vector<Iso::IsolationType>& isoTypes,
                            double coneCoreSize,
                            const derefMap_t& derefMap) const;
#endif
      
      // etcone computation for Egamma
#ifndef XAOD_ANALYSIS
      bool etConeIsolation( CaloIsolation& result, const Egamma& eg, 
			    const std::vector<Iso::IsolationType>& isoTypes, 
			    const CaloCellContainer* container ) const;
#endif
			    
			    
      // topoetcone computation (common for TrackParticle and Egamma)
      bool topoConeIsolation(CaloIsolation& result, float eta, float phi, 
			     std::vector<float>& coneSizes,
                             bool coreEMonly,
			     const CaloClusterContainer* container,
                             const CaloCluster* fwdClus,
                             const Egamma* egObj,
                             double coneCoreSize) const;

      // sum of topo cluster in a cone
      bool topoClustCones(CaloIsolation& result,
                          float eta,
                          float phi,
                          std::vector<float>& m_coneSizes,
                          const std::vector<const CaloCluster*>& clusts) const;

      /// Correct the topo cluster isolation using sum of topo cluster in core
      /// region.
      bool correctIsolationEnergy_TopoCore(
        CaloIsolation& result,
        float eta,
        float phi,
        float dEtaMax_core,
        float dPhiMax_core,
        float dR2Max_core,
        const std::vector<const CaloCluster*>& clusts,
        bool onlyEM,
        const CaloCluster* fwdClus,
        const Egamma* egObj) const;

      // pflow etcone computation (common for TrackParticle and Egamma)
      bool pflowConeIsolation(CaloIsolation& result,
                              float eta,
                              float phi,
                              std::vector<float>& m_coneSizes,
                              bool coreEMonly,
                              const FlowElementContainer* container,
                              double coneCoreSize,
			      const Egamma *egObj = nullptr) const;

      // sum of pt of pflow objects in a cone
      bool pflowObjCones(CaloIsolation& result,
                         float eta,
                         float phi,
                         std::vector<float>& m_coneSizes,
                         const std::vector<const FlowElement*>& clusts) const;

      /// Correct the pflow isolation using sum of pflow objects in core region.
      bool correctIsolationEnergy_pflowCore(
        CaloIsolation& result,
        float eta,
        float phi,
        float detaMax,
        float dphiMax,
        float dR2Max,
        const std::vector<const FlowElement*>& clusts,
        bool onlyEM = false,
	const Egamma *egObj = nullptr) const;

      // core eg 5x7 egamma subtraction
      bool correctIsolationEnergy_Eeg57(
        CaloIsolation& result,
        const std::vector<Iso::IsolationType>& isoTypes,
        const Egamma* eg) const;
      // core for muon subtraction
      bool correctIsolationEnergy_MuonCore(CaloIsolation& result,
                                           const TrackParticle& tp,
                                           const derefMap_t& derefMap) const;

      // helper to get eta,phi of muon extrap
      bool GetExtrapEtaPhi(const TrackParticle* tp, float& eta, float& phi,
                           derefMap_t& derefMap) const;

      // pt correction (egamma)
      bool PtCorrection (CaloIsolation& result, 
			 const Egamma& eg, 
			 const std::vector<Iso::IsolationType>& isoTypes) const;

      // Correction for the underlying event
      bool EDCorrection(CaloIsolation& result, 
			const std::vector<Iso::IsolationType>& isoTypes,
			float eta,
			const std::string& type,
                        const CaloCluster* fwdClus) const;

      // init result structure
      static void initresult(CaloIsolation& result, const CaloCorrection& corrlist, unsigned int typesize) ;

      /** get reference particle */
      const IParticle* getReferenceParticle(const IParticle& particle) const;

      // add the calo decoration -- FIXME! Change to use standard caching
      void decorateTrackCaloPosition(const IParticle& particle, float eta, float phi) const;

#ifndef XAOD_ANALYSIS
      ToolHandle<Rec::IParticleCaloCellAssociationTool> m_assoTool {this, 
	  "ParticleCaloCellAssociationTool", 
	  "Rec::ParticleCaloCellAssociationTool/ParticleCaloCellAssociationTool"};

      ToolHandle<Trk::IParticleCaloExtensionTool> m_caloExtTool {this,
	  "ParticleCaloExtensionTool",
	  "Trk::ParticleCaloExtensionTool/ParticleCaloExtensionTool"};
      Trk::TrackParametersIdHelper  m_parsIdHelper;

      // clusters in cone tool
      ToolHandle<ICaloClustersInConeTool> m_clustersInConeTool {this,
	  "ClustersInConeTool",
	  "xAOD::CaloClustersInConeTool/CaloClustersInConeTool"}; 
      
      // pflow objects in cone tool
      ToolHandle<IFlowElementsInConeTool> m_pflowObjectsInConeTool {this,
	  "FlowElementsInConeTool", ""}; 
      
      /** @brief  Property: calo cluster filling tool */
      ToolHandle<CaloClusterProcessor> m_caloFillRectangularTool {this,
	  "CaloFillRectangularClusterTool", "",
	  "Handle of the CaloFillRectangularClusterTool"};

      /** Property: Use cached caloExtension if avaliable. */
      Gaudi::Property<bool> m_useCaloExtensionCaching {this, 
	  "UseCaloExtensionCaching", true, 
	  "Use cached caloExtension if avaliable."};

      /** The input calorimeter extensions */
      SG::ReadHandleKey<CaloExtensionCollection> m_caloExtensionKey{
        this, "InputCaloExtension", "", "The calorimeter extensions of the tracks"};

      /** CaloDetDescrManager from ConditionStore */
      SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this,"CaloDetDescrManager", "CaloDetDescrManager"};
#endif // XAOD_ANALYSIS

      /** @brief Tool for pt-corrected isolation calculation (new)*/
      ToolHandle<CP::IIsolationCorrectionTool> m_IsoLeakCorrectionTool {this,
	  "IsoLeakCorrectionTool", "",
	  "Handle on the leakage correction tool"};

      /** @brief vector of calo-id to treat*/
      Gaudi::Property<std::vector<int> > m_EMCaloNums {this,
	  "EMCaloNums", {}, "list of EM calo to treat"};

      /** @brief vector of calo-id to treat*/
      Gaudi::Property<std::vector<int> > m_HadCaloNums {this,
	  "HadCaloNums", {}, "list of Had calo to treat"};

      /** Topo Calo cluster location in event store */
      std::string m_CaloCalTopoCluster; 
      
      /** Property: Use TopoClusters at the EM scale. */
      Gaudi::Property<bool> m_useEMScale {this,
	  "UseEMScale", true,
	  "Use TopoClusters at the EM scale."};

      /** Property: do the ED corrections to topoisolation */
      Gaudi::Property<bool> m_doEnergyDensityCorrection {this,
	  "doEnergyDensityCorrection", true, 
	  "Correct isolation variables based on energy density estimations"};

      /** Property: save only requested corrections (trigger usage mainly) */
      Gaudi::Property<bool> m_saveOnlyRequestedCorrections {this,
	  "saveOnlyRequestedCorrections", false, 
	  "save only requested corrections (trigger usage mainly)"};

      /** Property: exclude tile scintillator*/
      Gaudi::Property<bool> m_ExcludeTG3 {this,
	  "ExcludeTG3", true, "Exclude the TileGap3 cells"};

      /** Property: Name of the central topocluster energy-density container. */ 
      SG::ReadHandleKey<EventShape> m_tpEDCentral {this,
	  "TopoClusterEDCentralContainer", "TopoClusterIsoCentralEventShape", 
	  "Name of TopoCluster ED Central"};

      /** Property: Name of the forward topocluster energy-density container. */ 
      SG::ReadHandleKey<EventShape> m_tpEDForward {this,
	  "TopoClusterEDForwardContainer", "TopoClusterIsoForwardEventShape", 
	  "Name of TopoCluster ED Forward"};

      /** Property: Name of the central neutral energy flow energy-density container. */ 
      SG::ReadHandleKey<EventShape> m_efEDCentral {this,
	  "EFlowEDCentralContainer", "NeutralParticleFlowIsoCentralEventShape", 
	  "Name of energy flow ED Central"};

      /** Property: Name of the forward neutral energy flow energy-density container. */ 
      SG::ReadHandleKey<EventShape> m_efEDForward {this,
	  "EFlowEDForwardContainer", "NeutralParticleFlowIsoForwardEventShape", 
	  "Name of energy flow ED Forward"};

      /** Property: The size of the coneCore core energy calculation. */
      Gaudi::Property<double> m_coneCoreSizeEg {this,
	  "coneCoreSizeEg", 0.1,  
	  "size of the coneCore core energy correction for egamma objects"};

      Gaudi::Property<double> m_coneCoreSizeMu {this,
	  "coneCoreSizeMu", 0.05, 
	  "size of the coneCore core energy correction for muons"};

      /** map to the orignal particle */
      std::map<const IParticle*, const IParticle*> m_derefMap;
      
      /** Property: Initialize read Handles. Default True. For HLT these need to be off. */
      Gaudi::Property<bool> m_InitializeReadHandles {this,
        "InitializeReadHandles", true,
        "Initialize all ReadHandles."};

      /** Property: need to know if this is MC (from rec.doTruth) for eta dep pileup corr */
      Gaudi::Property<bool> m_isMC {this, "isMC", false, "is MC"};

      /** Property: use pileup dependent correction*/
      Gaudi::Property<bool> m_useEtaDepPU {this,
	  "UseEtaDepPUCorr", true, "Use the eta dependent pileup correction"};

      /** name of the root file for the eta dependant pileup correction */
      Gaudi::Property<std::string> m_puZetaCorrectionFileName {this,
	  "EtaDependentPileupCorrectionFileName", "IsolationCorrections/v4/zetas.root",
	  "File name for the eta dependant pileup correction to isolation"};
      Gaudi::Property<std::string> m_puZetaMCCorrectionFileName {this,
	  "EtaDependentPileupMCCorrectionFileName", "IsolationCorrections/v4/zetas_correction.root",
	  "File name for the eta dependant pileup correction to isolation, small mc correction"};

      /** map of the zeta corrections (one / cone size) */
      std::map<Iso::IsolationType,std::unique_ptr<TGraph>> m_puZetaCorrection;
      std::map<Iso::IsolationType,std::unique_ptr<TGraph>> m_puZetaMCCorrection;

#ifdef XAOD_ANALYSIS // particlesInCone tool will not be avaible. Write our own...
      bool particlesInCone( float eta, float phi, float dr, std::vector<const CaloCluster*>& clusts ) const;
      bool particlesInCone( float eta, float phi, float dr, std::vector<const FlowElement*>& clusts ) const;
#endif // XAOD_ANALYSIS
      float Phi_mpi_pi(float x) const { 
        while (x >= M_PI) x -= 2.*M_PI;
        while (x < -M_PI) x += 2.*M_PI;
        return x;
      }
    };
  
}	// end of namespace

#endif


