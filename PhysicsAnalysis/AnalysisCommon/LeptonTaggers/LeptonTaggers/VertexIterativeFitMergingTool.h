// This is -*- c++ -*-

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef PROMPT_VERTEXITERATIVEFITMERGINGTOOL_H
#define PROMPT_VERTEXITERATIVEFITMERGINGTOOL_H

/**********************************************************************************
 * @Package: LeptonTaggers
 * @Class  : VertexIterativeFitMergingTool
 * @Author : Fudong He
 * @Author : Rustem Ospanov
 *
 * @Brief  :
 *
 *  Merge the input vertices and output merged vertices.
 *
 **********************************************************************************/

// Local
#include "LeptonTaggers/PromptUtils.h"
#include "IVertexMergingTool.h"
#include "IVertexFittingTool.h"

// Athena
#include "AthenaBaseComps/AthAlgorithm.h"
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ITHistSvc.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"

// xAOD
#include "xAODTracking/Vertex.h"
#include "xAODTracking/TrackParticle.h"

// ROOT
#include "TH1.h"


namespace Prompt
{
  //======================================================================================================
  // Two track vertex
  //
  struct TwoTrackVtx
  {
    TwoTrackVtx():vertex(nullptr), trackId0(0), trackId1(0), vertexFitProb(1000.0), sumTrackPt(0.0) {}

    std::unique_ptr<xAOD::Vertex> vertex;
    const xAOD::TrackParticle                 *trackId0;
    const xAOD::TrackParticle                 *trackId1;

    double                                     vertexFitProb;
    double                                     sumTrackPt;
  };

  //=============================================================================
  struct SortTwoTrackVtxBySumTrackPt
  {
    bool operator()(const TwoTrackVtx &lhs, const TwoTrackVtx &rhs) { return lhs.sumTrackPt > rhs.sumTrackPt; }
  };

  //=============================================================================
  struct SortTracksByPt
  {
    bool operator()(const xAOD::TrackParticle *lhs, const xAOD::TrackParticle *rhs) { return lhs->pt() > rhs->pt(); }
  };


  //=============================================================================
  struct SortTwoTrackVtxByDistToSeed
  {
    explicit SortTwoTrackVtxByDistToSeed(const xAOD::Vertex *seed_):seed(seed_) {}

    bool operator()(const TwoTrackVtx &lhs, const TwoTrackVtx &rhs)
    {
      return Prompt::getDistance(seed, lhs.vertex.get()) < Prompt::getDistance(seed, rhs.vertex.get());
    }

    const xAOD::Vertex *seed;
  };

  //======================================================================================================
  // Vertex Merging Tool
  //
  class VertexIterativeFitMergingTool: public AthAlgTool, virtual public IVertexMergingTool
  {
  public:

    VertexIterativeFitMergingTool(const std::string &name,
                const std::string &type,
                const IInterface  *parent);

    virtual StatusCode initialize() override;

    virtual MergeResult mergeInitVertices(
      const FittingInput &input,
      const xAOD::TrackParticle *tracklep,
      std::vector<std::unique_ptr<xAOD::Vertex>> &initVtxs,
      const std::vector<const xAOD::TrackParticle *> &selectedTracks
    ) override;

  private:

    bool mergeIteratively2TrackVtxs(const FittingInput &input,
            MergeResult &result,
            const VtxType vtxType);

    std::unique_ptr<xAOD::Vertex> fitSeedVertexCluster(
      const FittingInput &input,
      std::unique_ptr<xAOD::Vertex> seedVtx,
      const VtxType vtxType,
      std::vector<TwoTrackVtx> &others
    );

    std::unique_ptr<xAOD::Vertex> fitSeedPlusOtherVertex(
      const FittingInput &input,
      const xAOD::Vertex *seedVtx,
      const xAOD::Vertex *otherVtx,
      const VtxType vtxType
    );

    bool passVertexSelection(const xAOD::Vertex *vtx) const;

    unsigned removeMerged2TrackVertexes(const xAOD::Vertex *mergedVtx,
          std::vector<TwoTrackVtx> &vtxs);

    void plotVertexDistances(const std::vector<TwoTrackVtx> &others);

    std::vector<const xAOD::TrackParticle *> getTracksWithoutVertex(
      const std::vector<std::unique_ptr<xAOD::Vertex>> &passVtxs,
      const std::vector<const xAOD::TrackParticle *> &selectedTracks
    );

    std::vector<std::unique_ptr<xAOD::Vertex>> fit2TrackVertexes(
      const FittingInput &input,
      std::vector<const xAOD::TrackParticle *> &selectedTracks,
      const VtxType vtxType
    );

    StatusCode makeHist(TH1 *&h, const std::string &key, int nbin, double xmin, double xmax);

    //
    // Properties:
    //
    ToolHandle<Prompt::IVertexFittingTool> m_vertexFitterTool {
      this, "VertexFittingTool",
      "Prompt::VertexFittingTool/VertexFittingTool"
    };
    ServiceHandle<ITHistSvc> m_histSvc {
      this, "THistSvc", "THistSvc/THistSvc"
    };

    Gaudi::Property<double> m_minFitProb {this, "minFitProb", 0.01,
      "minimum fit probability requirement for a vertex"
    };
    Gaudi::Property<double> m_minCandOverSeedFitProbRatio {
      this, "minCandOverSeedFitProbRatio", 0.2,
      "minimum requirement of the fit probability of new merged vertex / fit probability of seed vertex"
    };
    Gaudi::Property<unsigned> m_maxExtraTracks {
      this, "maxExtraTracks", 10,
      "maximum number of tracks without good lepton+track vertex that we will used for further fitting of vertexes without lepton"
    };

    Gaudi::Property<std::string> m_outputStream {
      this, "outputStream", ""
    };

    //
    // Development histograms
    //
    TH1                                                   *m_histNvtx2TrkInit;
    TH1                                                   *m_histNvtx2TrkPass;
    TH1                                                   *m_histNvtx2TrkUnmerged;
    TH1                                                   *m_histNvtxMerged;

    TH1                                                   *m_histNewVtxFitChi2;
    TH1                                                   *m_histNewVtxFitProb;

    TH1                                                   *m_histNewVtxFitDistToCurr;
    TH1                                                   *m_histNewVtxFitDistToSeed;
    TH1                                                   *m_histNewVtxFitDistToSeedPass;
    TH1                                                   *m_histNewVtxFitDistToSeedFail;

    TH1                                                   *m_histNewVtxFitProbCandOverSeed;
    TH1                                                   *m_histNewVtxFitProbCandOverSeedPass;
    TH1                                                   *m_histNewVtxFitProbCandOverSeedFail;
    TH1                                                   *m_histNewVtxFitProbCandOverSeed3Trk;
    TH1                                                   *m_histNewVtxFitProbCandOverSeed3TrkPass;

    TH1                                                   *m_histVtx2TrkPairDist;
    TH1                                                   *m_histVtx2trkPairDistZoom;
    TH1                                                   *m_histVtx2TrkPairSig1;
    TH1                                                   *m_histVtx2TrkPairSig2;

    TH1                                                   *m_histSelectedTrackCountAll;
    TH1                                                   *m_histSelectedTrackCountMatch2Vtx;
    TH1                                                   *m_histSelectedTrackCountWithout2Vtx;

    TH1                                                   *m_histVtxWithoutLepton2TrkNTrack;
    TH1                                                   *m_histVtxWithoutLepton2TrkNPass;
    TH1                                                   *m_histVtxWithoutLepton2TrkNPassUnmerged;
    TH1                                                   *m_histVtxWithoutLepton2TrkNMerged;
  };
}

#endif
