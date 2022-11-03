/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "LRTElectronAnalysisTools/ElectronLRTOverlapRemovalTool.h"
#include "AsgTools/AsgToolConfig.h"

#include "CxxUtils/phihelper.h"

#include <algorithm>

namespace CP
{

    ElectronLRTOverlapRemovalTool::ElectronLRTOverlapRemovalTool(const std::string &name) : asg::AsgTool(name)
    {
    }

    ///////////////////////////////////////////////////////////////////
    // Initialisation
    ///////////////////////////////////////////////////////////////////
    StatusCode ElectronLRTOverlapRemovalTool::initialize()
    {

        if (!m_isDAOD)
        {
            if (m_electronLLHTool.empty())
            {
                asg::AsgToolConfig config("AsgElectronLikelihoodTool/ElectronLHSelectorVeryLooseNoPix");
                ATH_CHECK(config.setProperty("ConfigFile", "ElectronPhotonSelectorTools/trigger/rel22_20210611/ElectronLikelihoodVeryLooseTriggerConfig_NoPix.conf"));
                ATH_CHECK(config.makePrivateTool(m_electronLLHTool));
            }

            ATH_MSG_DEBUG("Retrieving electron selection tool");
            ATH_CHECK(m_electronLLHTool.retrieve());
        } 

        return StatusCode::SUCCESS;
    }


    //////////////////////////////////////////////////////////////////
    // Check if electron passes ID
    //////////////////////////////////////////////////////////////////
    bool ElectronLRTOverlapRemovalTool::electronPassesID(const xAOD::Electron *electron, std::string IDWorkingPoint) const
    {

        if (m_isDAOD)
        {
            SG::AuxElement::ConstAccessor<char> DFCommonElectronsWP(IDWorkingPoint);
            return bool(DFCommonElectronsWP(*electron) );
        } else
        {
            return bool(m_electronLLHTool->accept(electron));
        }

    }


    //////////////////////////////////////////////////////////////////////
    // Check overlap between the electron collections and save pointer of duplicates
    // This removes the LRT electron in favor of prompt
    //////////////////////////////////////////////////////////////////////
    void ElectronLRTOverlapRemovalTool::checkOverlap(const xAOD::ElectronContainer &promptElectronCol,
                                                     const xAOD::ElectronContainer &LRTElectronCol,
                                                     std::set<const xAOD::Electron *> &ElectronsToRemove) const
    {

        // Loop over lrt electrons to remove those that do not pass ID.
        // Needed in case there are no prompt electrons passing ID
        if (m_strategy == 0){
            for (const xAOD::Electron *LRTElectron : LRTElectronCol)
            {
                if (!electronPassesID(LRTElectron,m_IDWorkingPoint))  ElectronsToRemove.insert(LRTElectron);
            }
        }
        else if (m_strategy == 1){ 
            for (const xAOD::Electron *LRTElectron : LRTElectronCol)
            {
                if (!electronPassesID(LRTElectron,"DFCommonElectronsLHVeryLooseNoPix"))  ElectronsToRemove.insert(LRTElectron);
            }
        }


        // loop over prompt electrons
        for (const xAOD::Electron *promptElectron : promptElectronCol)
        {
            const ElementLink promptClusterLink = promptElectron->caloClusterLink(0);
            const xAOD::CaloCluster_v1 *prompt_cluster = (*promptClusterLink);

            // Skip electrons that do not pass ID threshold
            if (!electronPassesID(promptElectron,m_IDWorkingPoint))
            {
                ElectronsToRemove.insert(promptElectron);
                continue;
            }        

            // loop over lrt electrons
            for (const xAOD::Electron *LRTElectron : LRTElectronCol)
            {
                const ElementLink LRTClusterLink = LRTElectron->caloClusterLink(0);
                const xAOD::CaloCluster_v1 *lrt_cluster = (*LRTClusterLink);

                // Skip LRT electrons that do not pass ID threshold
                if (!electronPassesID(LRTElectron,m_IDWorkingPoint)) continue;

                // check that clusters exist (necessary? copied from MuonSpec overlap, but all electrons have clusters...)
                // TODO: This should then fall back to delta R if clusters are missing
                if (!lrt_cluster and !prompt_cluster)
                    continue;

                // matching based on hottest cell of cluster
                //  as in ambiguity res for el/ph 

                const double prompt_elEta0 = prompt_cluster->eta0();
                const double prompt_elPhi0 = prompt_cluster->phi0();

                const double lrt_elEta0 = lrt_cluster->eta0();
                const double lrt_elPhi0 = lrt_cluster->phi0();

                if (prompt_elEta0 == lrt_elEta0 && prompt_elPhi0 == lrt_elPhi0) 
                {
                     if (m_strategy == 0){
                        ATH_MSG_DEBUG("Found a Calo cluster shared by LRT electron and prompt electron !");

                        // Save pointer to LRT electrons failing overlap
                        // This removes the LRT electron in favor of prompt
                        ElectronsToRemove.insert(LRTElectron);
                    }
                    else if (m_strategy == 1){ //use tighter electron, if both equally tight use std collection
                    
                        ATH_MSG_INFO("Implementing overlap removal strategy 1");
                        if (electronPassesID(promptElectron,"DFCommonElectronsLHTightNoPix")){
                            ElectronsToRemove.insert(LRTElectron);
                        }
                        else if (electronPassesID(promptElectron,"DFCommonElectronsLHMediumNoPix") ) {
                            if (electronPassesID(LRTElectron,"DFCommonElectronsLHTightNoPix") ){
                                ElectronsToRemove.insert(promptElectron);
                            }
                            else ElectronsToRemove.insert(LRTElectron);
                        }
                        else if (electronPassesID(promptElectron,"DFCommonElectronsLHLooseNoPix") ) {
                            if (electronPassesID(LRTElectron,"DFCommonElectronsLHMediumNoPix") ){
                                ElectronsToRemove.insert(promptElectron);
                            }
                            else ElectronsToRemove.insert(LRTElectron);
                        }
                        else if (electronPassesID(promptElectron,"DFCommonElectronsLHVeryLooseNoPix") ) {
                            if (electronPassesID(LRTElectron,"DFCommonElectronsLHLooseNoPix") ){
                                ElectronsToRemove.insert(promptElectron);
                            }
                            else ElectronsToRemove.insert(LRTElectron);
                        }
                    }
                }
            } // end lrt loop
        }   // end prompt loop
    }

} // end namespace CP
