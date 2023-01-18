/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERCOND_H
#define INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERCOND_H

//Trk
#include "TrkDetDescrInterfaces/IGeometryBuilderCond.h"
#include "TrkDetDescrInterfaces/ILayerProviderCond.h"
#include "TrkDetDescrUtils/BinningType.h"
#include "TrkGeometry/TrackingGeometry.h"
//InDet
#include "StagedTrackingGeometryBuilderImpl.h"
// Athena
#include "CxxUtils/checker_macros.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
// STL
#include <vector>
#include <string>

#ifndef TRKDETDESCR_TAKESMALLERBIGGER
#define TRKDETDESCR_TAKESMALLERBIGGER
#define takeSmaller(current,test) current = current < test ? current : test
#define takeBigger(current,test)  current = current > test ? current : test
#define takeSmallerBigger(cSmallest, cBiggest, test) takeSmaller(cSmallest, test); takeBigger(cBiggest, test)
#endif


namespace Trk {
 class Layer;
 class Material;
}

namespace InDet {
struct LayerSetupCond
{

  // the layer cache
  std::vector<Trk::Layer*> negativeLayers;
  std::vector<Trk::Layer*> centralLayers;
  std::vector<Trk::Layer*> positiveLayers;

  // center information
  double minRadiusCenter;
  double maxRadiusCenter;
  double zExtendCenter;
  int binningCenter;

  // endcap information
  bool buildEndcap;
  double minRadiusEndcap;
  double maxRadiusEndcap;
  double minZextendEndcap;
  double maxZextendEndcap;
  int binningEndcap;

  // full setup information
  double zSector;
  double rMin;
  double rMax;
  double zMax;

  std::string identification;
  int colorCode;

  LayerSetupCond(const std::string& idName,
                 int cCode,
                 const std::vector<Trk::Layer*>& negLayers,
                 const std::vector<Trk::Layer*>& cenLayers,
                 const std::vector<Trk::Layer*>& posLayers,
                 double minRc,
                 double maxRc,
                 double zC,
                 int binC,
                 bool bec = false,
                 double minRe = 0.,
                 double maxRe = 0.,
                 double zMinE = 0.,
                 double zMaxE = 0.,
                 int binE = 0)
    : negativeLayers(negLayers)
    , centralLayers(cenLayers)
    , positiveLayers(posLayers)
    , minRadiusCenter(minRc)
    , maxRadiusCenter(maxRc)
    , zExtendCenter(zC)
    , binningCenter(binC)
    , buildEndcap(bec)
    , minRadiusEndcap(minRe)
    , maxRadiusEndcap(maxRe)
    , minZextendEndcap(zMinE)
    , maxZextendEndcap(zMaxE)
    , binningEndcap(binE)
    , identification(idName)
    , colorCode(cCode)
  {
    rMin =
      minRadiusCenter < minRadiusEndcap ? minRadiusCenter : minRadiusEndcap;
    rMax =
      maxRadiusCenter > maxRadiusEndcap ? maxRadiusCenter : maxRadiusEndcap;
    zMax = zExtendCenter > maxZextendEndcap ? zExtendCenter : maxZextendEndcap;
    zSector =
      buildEndcap ? 0.5 * (zExtendCenter + minZextendEndcap) : zExtendCenter;
  }
};

  /** @class StagedTrackingGeometryBuilderCond

      New Geometry builder that adapts to different layer setups

      Only a few parameters are not automated:
       - m_outwardsFraction: this defines how much you orient yourself on the next bigger layer
                             if you wrap an outer volume around an inner 0.5 would lead to a boundary fully in bewteen
                            1. at the outer boundary, 0. at the inner boundary

      @author Andreas.Salzburger@cern.ch

    */

  class ATLAS_NOT_THREAD_SAFE StagedTrackingGeometryBuilderCond : //const_cast
    public extends<StagedTrackingGeometryBuilderImpl, Trk::IGeometryBuilderCond> {

    public:
      /** Constructor */
      StagedTrackingGeometryBuilderCond(const std::string&,const std::string&,const IInterface*);

      /** Destructor */
      virtual ~StagedTrackingGeometryBuilderCond() = default;

      /** AlgTool initialize method.*/
      virtual StatusCode initialize() override final;

      /** TrackingGeometry Interface methods */
      virtual
      std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
                                                              const EventContext& ctx,
                                                              Trk::TrackingVolume* tVol,
                                                              SG::WriteCondHandle<Trk::TrackingGeometry>& whandle
                                                              ) const override final;
      /** The unique signature */
      virtual Trk::GeometrySignature geometrySignature() const override { return Trk::ID; }

    private:
      /** Private helper method, estimates the overal dimensions */
      LayerSetupCond estimateLayerSetup(
                                        const std::string& idName,
                                        size_t ils,
                                        const std::vector<Trk::Layer*>& negLayers,
                                        const std::vector<Trk::Layer*>& centralLayers,
                                        const std::vector<Trk::Layer*>& posLayers,
                                        double maxR,
                                        double maxZ) const;

      /** Private helper method to check if a sector is compatible with the cache */
      bool setupFitsCache(
                          LayerSetupCond& layerSetup,
                          std::vector<InDet::LayerSetupCond>& layerSetupCache) const;

      /** Private helper method to flush the cache into the id volumes - return
       * volume is the one to be provided */
      Trk::TrackingVolume* createFlushVolume
      ATLAS_NOT_THREAD_SAFE(std::vector<InDet::LayerSetupCond>& layerSetupCache,
                            double innerRadius,
                            double& outerRadius,
                            double extendZ) const;

      using StagedTrackingGeometryBuilderImpl::packVolumeTriple;
      /** Private helper method, creates and packs a triple containing of NegEndcap-Barrel-PosEndcap layers
          - in case of a ring layout the subvolumes are created and the rMax is adapted
         */
      Trk::TrackingVolume* packVolumeTriple
      ATLAS_NOT_THREAD_SAFE(LayerSetupCond& layerSetup,
                            double rMin,
                            double& rMax,
                            double zMin,
                            double zPosCentral) const;

      /** Private helper method for merging of rings with z-overlap */
      virtual Trk::Layer* mergeDiscLayers(std::vector<Trk::Layer*>& dlays) const override final;

      // Configurable Properties
      // helper tools for the geometry building
      ToolHandleArray<Trk::ILayerProviderCond>       m_layerProviders;          //!< Helper Tools for the Layer creation, includes beam pipe builder
  };

} // end of namespace

#endif //INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDERCOND_H
