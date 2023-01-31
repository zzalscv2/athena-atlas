/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDER_H
#define INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDER_H

//Trk
#include "TrkDetDescrInterfaces/IGeometryBuilder.h"
#include "TrkDetDescrInterfaces/ILayerProvider.h"
#include "TrkDetDescrUtils/BinningType.h"
// Athena
#include "StagedTrackingGeometryBuilderImpl.h"
// Gaudi
#include "GaudiKernel/ToolHandle.h"
// STL
#include <vector>
#include <string>

#include "CxxUtils/checker_macros.h"

#ifndef TRKDETDESCR_TAKESMALLERBIGGER
#define TRKDETDESCR_TAKESMALLERBIGGER
#define takeSmaller(current,test) current = current < test ? current : test
#define takeBigger(current,test)  current = current > test ? current : test
#define takeSmallerBigger(cSmallest, cBiggest, test) takeSmaller(cSmallest, test); takeBigger(cBiggest, test)
#endif


namespace Trk {
 class TrackingGeometry;
 class Layer;
 class Material;
}

namespace InDet {

  /** @struct LayerSetup
       - helps understanding how to pack the layers into a volume compound
   */
  struct LayerSetup {

    // the layer cache
    std::vector<Trk::Layer*> negativeLayers;
    std::vector<Trk::Layer*> centralLayers;
    std::vector<Trk::Layer*> positiveLayers;

    // center information
    double minRadiusCenter;
    double maxRadiusCenter;
    double zExtendCenter;
    int    binningCenter;

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
    int         colorCode;

    LayerSetup(const std::string& idName,
               int cCode,
               const std::vector<Trk::Layer*>& negLayers,
               const std::vector<Trk::Layer*>& cenLayers,
               const std::vector<Trk::Layer*>& posLayers,
               double minRc, double maxRc, double zC, int binC,
               bool bec=false, double minRe=0., double maxRe=0., double zMinE=0., double zMaxE=0., int binE = 0) :
      negativeLayers(negLayers),
      centralLayers(cenLayers),
      positiveLayers(posLayers),
      minRadiusCenter(minRc),
      maxRadiusCenter(maxRc),
      zExtendCenter(zC),
      binningCenter(binC),
      buildEndcap(bec),
      minRadiusEndcap(minRe),
      maxRadiusEndcap(maxRe),
      minZextendEndcap(zMinE),
      maxZextendEndcap(zMaxE),
      binningEndcap(binE),
      identification(idName),
      colorCode(cCode)
    {
        rMin     = minRadiusCenter < minRadiusEndcap ? minRadiusCenter : minRadiusEndcap;
        rMax     = maxRadiusCenter > maxRadiusEndcap ? maxRadiusCenter : maxRadiusEndcap;
        zMax     = zExtendCenter > maxZextendEndcap ? zExtendCenter : maxZextendEndcap;
        zSector  = buildEndcap ? 0.5*(zExtendCenter+minZextendEndcap) : zExtendCenter;

    }

  };


  /** @class StagedTrackingGeometryBuilder

      New Geometry builder that adapts to different layer setups

      Only a few parameters are not automated:
       - m_outwardsFraction: this defines how much you orient yourself on the next bigger layer
                             if you wrap an outer volume around an inner 0.5 would lead to a boundary fully in bewteen
                            1. at the outer boundary, 0. at the inner boundary

      @author Andreas.Salzburger@cern.ch

    */

  class ATLAS_NOT_THREAD_SAFE StagedTrackingGeometryBuilder : // const_cast
    public extends <StagedTrackingGeometryBuilderImpl, Trk::IGeometryBuilder> {


    public:
      /** Constructor */
      StagedTrackingGeometryBuilder(const std::string&,const std::string&,const IInterface*);

      /** Destructor */
      virtual ~StagedTrackingGeometryBuilder() = default;

      /** AlgTool initialize method.*/
      virtual StatusCode initialize() override final;

      /** TrackingGeometry Interface methods */
      virtual
        std::unique_ptr<Trk::TrackingGeometry> trackingGeometry(
                                                                Trk::TrackingVolume* tvol = 0
                                                                ) const override final;
      /** The unique signature */
      virtual Trk::GeometrySignature geometrySignature() const override final { return Trk::ID; }

    private:
      /** Private helper method, estimates the overal dimensions */
      LayerSetup estimateLayerSetup(
                                    const std::string& idName,
                                    size_t ils,
                                    const std::vector<Trk::Layer*>& negLayers,
                                    const std::vector<Trk::Layer*>& centralLayers,
                                    const std::vector<Trk::Layer*>& posLayers,
                                    double maxR,
                                    double maxZ) const;

      /** Private helper method to check if a sector is compatible with the cache */
      bool setupFitsCache(
                          LayerSetup& layerSetup,
                          std::vector<InDet::LayerSetup>& layerSetupCache) const;

      /** Private helper method to flush the cache into the id volumes - return
       * volume is the one to be provided */
      Trk::TrackingVolume* createFlushVolume(
      std::vector<InDet::LayerSetup>& layerSetupCache,
      double innerRadius,
      double& outerRadius,
      double extendZ) const;

      using StagedTrackingGeometryBuilderImpl::packVolumeTriple;
      /** Private helper method, creates and packs a triple containing of
         NegEndcap-Barrel-PosEndcap layers
          - in case of a ring layout the subvolumes are created and the rMax is
         adapted
         */
      Trk::TrackingVolume* packVolumeTriple(
      LayerSetup& layerSetup,
      double rMin,
      double& rMax,
      double zMin,
      double zPosCentral) const;

      // helper tools for the geometry building
      ToolHandleArray<Trk::ILayerProvider>           m_layerProviders;          //!< Helper Tools for the Layer creation, includes beam pipe builder
  };

} // end of namespace

#endif //INDETTRACKINGGEOMETRY_STAGEDTRACKINGGEOMETRYBUILDER_H
