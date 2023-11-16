#ifndef MUONALIGNERRORTOOL_ALIGNMENTERRORTESTALG_H
#define MUONALIGNERRORTOOL_ALIGNMENTERRORTESTALG_H

#include <GaudiKernel/ToolHandle.h>

#include "AthenaBaseComps/AthAlgorithm.h"
#include "TrkToolInterfaces/ITrkAlignmentDeviationTool.h"
#include "TrkTrack/TrackCollection.h"

namespace MuonAlign {

class AlignmentErrorTestAlg : public AthAlgorithm {
 public:
  AlignmentErrorTestAlg(const std::string& name, ISvcLocator* pSvcLocator);
  ~AlignmentErrorTestAlg() override = default;

  StatusCode initialize() override;
  StatusCode execute() override;

 private:
  ToolHandle<Trk::ITrkAlignmentDeviationTool> m_alignmentErrorTool{
      this, "alignmentErrorTool", "MuonAlign::AlignmentErrorTool"};
  SG::ReadHandleKey<::TrackCollection> m_trackCollection{
      this, "trackCollection", "MuonSpectrometerTracks"};
};

}  // namespace MuonAlign

#endif
