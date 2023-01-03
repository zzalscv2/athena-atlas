#ifndef MUONSTGC_CNVTOOLS_PADTRIG_ROD_DECODER_H
#define MUONSTGC_CNVTOOLS_PADTRIG_ROD_DECODER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "MuonSTGC_CnvTools/IPadTrig_ROD_Decoder.h"

namespace Muon {
class PadTrig_ROD_Decoder : public IPadTrig_ROD_Decoder, public AthAlgTool 
{

 public:
  PadTrig_ROD_Decoder(const std::string& type, const std::string& name, const IInterface* parent);

  // Convert the given ROBFragment to an NSW_PadTriggerData object, and store it
  // in the RDO container at the appropriate hash ID, if no collection is found
  // at that hash ID.
  StatusCode fillCollection(const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment, NSW_PadTriggerDataContainer& rdo) const override;

 protected:
  // Properties
  Gaudi::Property<bool> m_pfebBCIDs{this,"pFEBBCIDs", false, "Are pFEB BCIDs present?"};
  Gaudi::Property<bool> m_toTPData{this, "toTPData", false,  "Is to-TP data present?"};
  Gaudi::Property<bool> m_channelMapping{this, "channelMapping", false, "Are pFEB channels re-mapped?"};
  
};
}  // namespace Muon

#endif  // MUONSTGC_CNVTOOLS_PADTRIG_ROD_DECODER_H
