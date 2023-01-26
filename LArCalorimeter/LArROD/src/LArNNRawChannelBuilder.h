/*
   Copyright (C) 2023 CERN for the benefit of the ATLAS collaboration
 */

#ifndef LARROD_LARNNRAWCHANNELBUILDER_H
#define LARROD_LARNNRAWCHANNELBUILDER_H


#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"
#include "StoreGate/ReadCondHandle.h"

#include "LArRawEvent/LArDigitContainer.h"
#include "LArRawEvent/LArRawChannelContainer.h"

#include "LArElecCalib/ILArPedestal.h"
#include "LArRawConditions/LArADC2MeV.h"
#include "LArCabling/LArOnOffIdMapping.h"
#include "AthenaPoolUtilities/AthenaAttributeList.h"

#include "LArRawConditions/LArDSPThresholdsComplete.h"
#include "LArElecCalib/ILArOFC.h"
#include "LArElecCalib/ILArShape.h"

#include <vector>


//Event classes
class LArDigitContainer;
class LArRawChannelContainer;

class LArOnlineID;


class LArNNRawChannelBuilder : public AthReentrantAlgorithm {

public:
LArNNRawChannelBuilder(const std::string& name, ISvcLocator* pSvcLocator);

StatusCode initialize() override;
StatusCode execute(const EventContext& ctx) const override;


private:
//Event input:
SG::ReadHandleKey<LArDigitContainer>m_digitKey{this, "LArDigitKey", "FREE",
                                               "SG Key of LArDigitContaiiner"};
//Event output:
SG::WriteHandleKey<LArRawChannelContainer>m_rawChannelKey{this, "LArRawChannelKey", "LArRawChannels",
                                                          "SG key of the output LArRawChannelContainer"};
//Conditions input:
SG::ReadCondHandleKey<ILArPedestal>m_pedestalKey{this, "PedestalKey", "LArPedestal", "SG Key of Pedestal conditions object"};
SG::ReadCondHandleKey<LArADC2MeV>m_adc2MeVKey{this, "ADC2MeVKey", "LArADC2MeV", "SG Key of ADC2MeV conditions object"};
SG::ReadCondHandleKey<LArOnOffIdMapping>m_cablingKey{this, "CablingKey", "LArOnOffIdMap", "SG Key of LArOnOffIdMapping object"};
SG::ReadCondHandleKey<ILArOFC> m_ofcKey{this,"OFCKey","LArOFC","SG Key of OFC conditions object"};
SG::ReadCondHandleKey<ILArShape> m_shapeKey{this,"ShapeKey","LArShape","SG Key of Shape conditions object"}; 
SG::ReadCondHandleKey<LArDSPThresholdsComplete> m_run1DSPThresholdsKey{this, "Run1DSPThresholdsKey","", "SG Key for thresholds to compute time and quality, run 1"};
SG::ReadCondHandleKey<AthenaAttributeList> m_run2DSPThresholdsKey{this, "Run2DSPThresholdsKey","", "SG Key for thresholds to compute time and quality, run 2"};

//The following matters only in the MC case, when we have a 32 sample shapes
Gaudi::Property<int>m_firstSample{this, "firstSample", 0, "first of the 32 sampels of the MC shape to be used"};

//Identifier helper
const LArOnlineID* m_onlineId = nullptr;

Gaudi::Property<std::string>m_nn_json{this, "NNJsonPath", "", "Path to json containing the lwtnn network"};
Gaudi::Property<std::string>m_input_node{this, "NetworkInputNode", "", "Name of the input node"};
Gaudi::Property<std::string>m_network_output{this, "NetworkOutputNode", "", "Name of the output node"};

//This flag decides, wheter to use DB or constant threshold
Gaudi::Property<bool> m_useDBFortQ{this,"useDB",true,"Use DB for cut on t,Q"};

};


#endif
