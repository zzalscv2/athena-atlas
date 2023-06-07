#ifndef MUONSTGC_CNVTOOLS_IPADTRIG_ROD_DECODER_H
#define MUONSTGC_CNVTOOLS_IPADTRIG_ROD_DECODER_H

#include "ByteStreamData/RawEvent.h"
#include "GaudiKernel/IAlgTool.h"
#include "MuonRDO/NSW_PadTriggerDataContainer.h"

namespace Muon {

// IAlgTool which facilitates conversion from Pad Trigger ROBFragments to RDO.
class IPadTrig_ROD_Decoder : virtual public IAlgTool {
 public:
  virtual ~IPadTrig_ROD_Decoder() = default;
  // InterfaceID for this AlgTool
  DeclareInterfaceID(Muon::IPadTrig_ROD_Decoder, 1, 0);
  // Fill the given Pad Trigger RDO container with data from the given
  // ROBFragments.
  virtual StatusCode fillCollection(
      const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment& fragment,
      NSW_PadTriggerDataContainer& rdo) const = 0;
};
}  // namespace Muon
#endif  // MUONSTGC_CNVTOOLS_IPADTRIG_ROD_DECODER_H