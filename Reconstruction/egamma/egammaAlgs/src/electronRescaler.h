/*
   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef EGAMMAALGS_ELECTRONRESCALER_H
#define EGAMMAALGS_ELECTRONRESCALER_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"

#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "xAODEgamma/ElectronFwd.h"
#include "xAODEgamma/ElectronContainer.h"
#include <memory>


//NEVER USE THIS ALGORITHM 
//The only purpose is to demonstrate the AODFix functionality

class electronRescaler : public AthReentrantAlgorithm
{
public:

    electronRescaler(const std::string& name, ISvcLocator* pSvcLocator);

    StatusCode initialize() override final;
    StatusCode finalize() override final;
    StatusCode execute(const EventContext& ctx) const override final;

private:

    // Read/Write handlers

    /** @brief Name of the electron output collection*/
    SG::WriteHandleKey<xAOD::ElectronContainer> m_electronOutputKey {this,
        "OutputName", "Electrons",
        "Name of Electron Container to be created"};

    /** @brief Name of the photon output collection */
    SG::ReadHandleKey<xAOD::ElectronContainer> m_electronInputKey {this,
        "InputName", "old_Electrons",
        "Name of Electron container to be read in"};

    Gaudi::Property<float> m_scaleValue{this,"ScaleValue",0.01,"Scale to be applied on Electron pt"};

};

#endif
