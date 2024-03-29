/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetTrackingGeometry/TRT_LayerBuilderImpl.h"
#include "InDetTrackingGeometry/TRT_OverlapDescriptor.h"
//Trk
#include "TrkSurfaces/DiscBounds.h"
#include "TrkSurfaces/RectangleBounds.h"
#include "TrkGeometry/BinnedLayerMaterial.h"
#include "TrkGeometry/CylinderLayer.h"
#include "TrkGeometry/DiscLayer.h"
#include "TrkDetDescrUtils/BinnedArrayArray.h"
#include "TrkDetDescrUtils/BinnedArray1D.h"
#include "TrkDetDescrUtils/BinnedArray2D.h"
// GeoPrimitives
#include "GeoPrimitives/GeoPrimitivesHelpers.h"
// InDetDD
#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"
#include "TRT_ReadoutGeometry/TRT_Numerology.h"
#include "TRT_ReadoutGeometry/TRT_BarrelElement.h"
#include "TRT_ReadoutGeometry/TRT_EndcapElement.h"
#include "InDetIdentifier/TRT_ID.h"

namespace {
  template<class T>
  class PtrVectorWrapper
  {
  public:
    PtrVectorWrapper()
      : m_ptr(new std::vector<T*>)
    {}

    ~PtrVectorWrapper()
    {
      if (m_ptr) {
        for (const T* elm : *m_ptr) {
          delete elm;
        }
        m_ptr->clear();
      }
    }
    std::vector<T*>& operator*() { return *m_ptr; }
    const std::vector<const T*>& operator*() const { return *m_ptr; }

    std::vector<T*>* operator->() { return m_ptr.get(); }
    const std::vector<const T*>* operator->() const { return m_ptr.get(); }

    std::vector<T*>* release() { return m_ptr.release(); }

  private:
    std::unique_ptr<std::vector<T*>> m_ptr;
  };

}

// constructor
InDet::TRT_LayerBuilderImpl::TRT_LayerBuilderImpl(const std::string& t, const std::string& n, const IInterface* p) :
  AthAlgTool(t,n,p)
{
}


std::unique_ptr<const std::vector<Trk::CylinderLayer*> >
InDet::TRT_LayerBuilderImpl::cylindricalLayersImpl(const InDetDD::TRT_DetElementContainer* trtContainer) const{
  ATH_MSG_DEBUG( "Building cylindrical layers for the TRT " );
  PtrVectorWrapper<Trk::CylinderLayer> barrelLayers;

  // get Numerology and Id HElper
  const InDetDD::TRT_Numerology* trtNums = trtContainer->getTRTNumerology();

  // get the TRT ID Helper
  const TRT_ID* trtIdHelper = nullptr;
  if (detStore()->retrieve(trtIdHelper, "TRT_ID").isFailure()) {
    ATH_MSG_ERROR("Could not get TRT ID helper");
    return nullptr;
  }

  int    nBarrelRings  = trtNums->getNBarrelRings();
  int    nBarrelPhiSectors = trtNums->getNBarrelPhi();
  double layerPhiStep      = 2*M_PI/nBarrelPhiSectors;

  int nTotalBarrelLayers = 0;

  // get the overall dimensions
  double rMin = 10e10;
  double rMax = 0.;

  double layerZmax = 0.;
  double layerZmin = 10e10;

  // pre-loop for overall layer numbers & some ordering ---------------------------------------
  for (int ring=0; ring < nBarrelRings; ring++) {
    // the number of barrel layers
    int nBarrelLayers = trtNums->getNBarrelLayers(ring);
    nTotalBarrelLayers += nBarrelLayers;
    // loop over layers
    for (int layer=0; layer < nBarrelLayers; layer++){
      for (int phisec=0; phisec <nBarrelPhiSectors; ++phisec)
        {
          for (int iposneg=0; iposneg<2; ++iposneg){
            // get the element
            const InDetDD::TRT_BarrelElement* trtbar = trtContainer->getBarrelDetElement(iposneg, ring, phisec, layer); // TODO share this line

            // get overall dimensions only one time
            const Trk::PlaneSurface*    elementSurface = dynamic_cast<const Trk::PlaneSurface*>(&(trtbar->surface()));
            if (!elementSurface) {
              ATH_MSG_WARNING( "elementSurface: dynamic_cast to Trk::PlaneSurface failed - skipping ... ring/layer/phisec/iposneg = " << ring << "/" << layer << "/" << phisec << "/" << iposneg );
              continue;
            }
            const Trk::RectangleBounds* elementBounds  = dynamic_cast<const Trk::RectangleBounds*>(&(trtbar->bounds()));
            if (!elementBounds) {
              ATH_MSG_WARNING( "elementBounds: dynamic_cast to Trk::RectangleBounds failed - skipping ... ring/layer/phisec/iposneg = " << ring << "/" << layer << "/" << phisec << "/" << iposneg );
              continue;
            }
            double elementZcenter = (elementSurface->center()).z();
            double elementZmin    = std::abs(elementZcenter - elementBounds->halflengthY());
            double elementZmax    = std::abs(elementZcenter + elementBounds->halflengthY());
            // take what you need
            takeSmaller(layerZmin, elementZmin); takeBigger(layerZmax, elementZmax);
            // get the radial dimensions
            double currentR = trtbar->center().perp();
            takeSmallerBigger(rMin,rMax,currentR);
          }
        }
    }
  }

  if (nTotalBarrelLayers==0) {
    ATH_MSG_WARNING( "nTotalBarrelLayers = 0 ... aborting and returning 0 !" );
    return nullptr;
  }

  // calculate delta(R) steps and delta(R)
  double rDiff           = std::abs(rMax-rMin);
  double rStep           = rDiff/(m_modelBarrelLayers+1);
  double layerHalflength = layerZmax;

  // prepare the material
  if ( std::abs(rDiff) <= 0.1 ) {
    return nullptr;
  }

  //  fix the positions where the layers are - these are used for the model geometry and the complex geometry ---------------
  std::vector<double> layerRadii;
  layerRadii.reserve(m_modelBarrelLayers);
  for (unsigned int ilay = 1; ilay <= m_modelBarrelLayers; ++ilay)
    layerRadii.push_back(rMin+ilay*rStep-0.5*m_layerThickness);
  // these are the layer iterators
  auto layerRadiusIter    = layerRadii.begin();
  auto layerRadiusIterEnd = layerRadii.end();

  // (A) model geometry section
  if (m_modelGeometry && !m_registerStraws){

    ATH_MSG_VERBOSE( " -> " << layerRadii.size() << " cylindrical barrel layers between " << rMin << " and " << rMax << " ( at step "<< rStep << " )");

    // create the layers
    for ( ; layerRadiusIter != layerRadiusIterEnd; ++layerRadiusIter ) {
      // ----- prepare the BinnedLayerMaterial -----------------------------------------------------
      Trk::BinnedLayerMaterial* layerMaterial = nullptr;
      // -- material with 1D binning
      Trk::BinUtility layerBinUtility1DZ(m_barrelLayerBinsZ,-layerHalflength, layerHalflength, Trk::open, Trk::binZ);
      if (m_barrelLayerBinsPhi==1){
        // no binning in phi
        layerMaterial =new Trk::BinnedLayerMaterial(layerBinUtility1DZ);
      } else { // -- material with 2D binning : Rphi*Z optimized for cylinder layer
        Trk::BinUtility layerBinUtility2DRPhiZ(m_barrelLayerBinsPhi,
                                               -(*layerRadiusIter)*M_PI,
                                               (*layerRadiusIter)*M_PI,
                                               Trk::closed,
                                               Trk::binRPhi);
        layerBinUtility2DRPhiZ += layerBinUtility1DZ;
        layerMaterial =new Trk::BinnedLayerMaterial(layerBinUtility2DRPhiZ);
      }
      // Barrel layers are centered around (0,0,0) by definition
      barrelLayers->push_back(new Trk::CylinderLayer(new Trk::CylinderBounds(*layerRadiusIter,layerHalflength),
                                                     *layerMaterial,
                                                     m_layerThickness));
      ATH_MSG_VERBOSE( " --> Creating a layer at radius : " << *layerRadiusIter );
      delete layerMaterial;
    }
  } else {

    // (B) complex geometry section
    float nMaterialLayerStep  = 1.*nTotalBarrelLayers/m_modelBarrelLayers;
    // complex geo should build same # of mat. layers as model geo; counter to check this:
    unsigned int cMaterialLayerCount = 0;
    // inclusive layer counter over all rings, used to determine mat. layer position
    unsigned int cLayer=0;

    // loop over rings
    ATH_MSG_VERBOSE("TRT Barrel has " << nBarrelRings << " rings.");

    for (int ring=0; ring < nBarrelRings; ring++){

      int nBarrelLayers = trtNums->getNBarrelLayers(ring);
      ATH_MSG_VERBOSE("-> Ring " << ring << " has " << nBarrelLayers << " barrel layers.");
      // loop over layers
      for (int layer=0; layer < nBarrelLayers; layer++){

        // ----------------------------------------------------------------------------------
        ATH_MSG_VERBOSE("--> Layer " << layer << " is being built with " << nBarrelPhiSectors << " secors in phi.");

        // increase inclusive layer counter for next material layer
        ++cLayer;

        // set layer dimensions radius
        double layerRadius         =  0.;
        double layerRadiusMin      =  10e10;
        double layerRadiusMax      =  0.;
        double layerPhiMin         =  10.;
        double layerPhiMax         = -10;

        // per phi sector we make a 2D binnin in phi-z
        std::vector< std::pair<Trk::BinnedArray<Trk::Surface>*, Amg::Vector3D >  > layerSectorArrays;
        Amg::Vector3D layerSectorPosition(0.,0.,0.);

        // the sector approaching surfaces
        std::vector< std::pair< Trk::SharedObject<const Trk::ApproachSurfaces>, Amg::Vector3D > > layerApproachSurfaces;

        // layer sector arrays
        for (int phisec=0; phisec < nBarrelPhiSectors; phisec++){
          // ----------------------------------------------------------------------------------
          ATH_MSG_VERBOSE("---> Sector " << phisec << " gahtering the details.");
          // -------------- a phi sector (expands in +/- z) -----------------------------------

          // order the straws onto layers
          std::vector< Trk::SurfaceOrderPosition > strawsPerPhiSecLayer;
          // get the min an max phi, the min and max z
          double phiMin       =  10.;
          double phiMax       = -10.;
          // sector stuff
          int    sectorStraws = 0;
          // positive and negative sector
          for (int posneg=0; posneg<2; ++posneg){
            // sort the elements
            const InDetDD::TRT_BarrelElement* currentElement = trtContainer->getBarrelDetElement(posneg, ring, phisec, layer); // TODO share this line
            // get overall dimensions only one time
            const Trk::PlaneSurface*    elementSurface = dynamic_cast<const Trk::PlaneSurface*>(&(currentElement->surface()));
            if (!elementSurface) {
              ATH_MSG_WARNING( "elementSurface: dynamic_cast to Trk::PlaneSurface failed - skipping ... ring/layer/phisec/posneg = " << ring << "/" << layer << "/" << phisec << "/" << posneg );
              continue;
            }

            // create teh approach surfaces --------------------------------------------------------------------------------------------------
            // getTransformFromRotTransl(Amg::RotationMatrix3D rot, Amg::Vector3D transl_vec )
            Trk::ApproachSurfaces* aSurfaces         = new Trk::ApproachSurfaces;
            const Amg::Transform3D& elementTransform = elementSurface->transform();
            const Amg::Vector3D&    elementCenter    = elementSurface->center();
            const Amg::Vector3D&    elementNormal    = elementSurface->normal();
            Amg::RotationMatrix3D   elementRotation  = elementTransform.rotation();
            // outer / inner
            Amg::Vector3D outerCenter(elementCenter+(0.5*m_layerThickness+m_layerStrawRadius)*elementNormal);
            Amg::Vector3D innerCenter(elementCenter-(0.5*m_layerThickness+m_layerStrawRadius)*elementNormal);

            // assign the layer sector position for the straw array ordering
            layerSectorPosition = elementSurface->center();

            // now register the two surfaces
            aSurfaces->push_back(new Trk::PlaneSurface(Amg::Transform3D(Amg::getTransformFromRotTransl(elementRotation, innerCenter))));
            aSurfaces->push_back(new Trk::PlaneSurface(Amg::Transform3D(Amg::getTransformFromRotTransl(elementRotation, outerCenter))));

            // now register it to for building the array
            layerApproachSurfaces.emplace_back( Trk::SharedObject<const Trk::ApproachSurfaces>(aSurfaces),elementCenter);
            // screen output
            ATH_MSG_VERBOSE("---> Sector " << phisec << " - posneg - " << posneg << " - with central phi = " << elementSurface->center().phi() );
            // sector phi centers
            takeSmallerBigger(layerPhiMin,layerPhiMax,elementSurface->center().phi());

            // loop over straws, fill them and find the phi boundaries
            for (unsigned int istraw=0; istraw<currentElement->nStraws(); ++istraw)
              {
                Identifier strawId = trtIdHelper->straw_id(currentElement->identify(), istraw);
                const Trk::Surface* currentStraw = &(currentElement->surface(strawId));
                // get the phi values
                double currentPhi = currentStraw->center().phi();
                if (phisec == m_barrelSectorAtPiBoundary && currentPhi < 0.){
                  currentPhi  = M_PI + currentPhi;
                  currentPhi += M_PI;
                }
                // the layer radius
                takeSmallerBigger(layerRadiusMin,layerRadiusMax,currentStraw->center().perp());
                takeSmallerBigger(phiMin, phiMax, currentPhi);
                // make the ordering position
                Amg::Vector3D strawOrderPos(currentStraw->center());
                /*
                 * The above line was using the nodel (not delete option for the old shared object
                 * now that SharedObject is a shared_ptr typedef do the same with empty deleter
                 */
                // Something like
                // Trk::SharedObject<Trk::Surface>  =
                // std::make_shared<Trk::Surface>(.....)) could be fine
                //
                // As things are now
                // 1) Notice that basically we couple the DetElement owned
                // surface to the Tracking Geometry passing a no-op deleter
                // (no delete happens) to the shared_ptr(SharedObject is
                // typedef of shared_ptr)
                // 2) The const_cast here make the
                // code non MT safe. For now we handle this by being careful
                // on lifetimes and non-re-entrant TG construction.
                Trk::SharedObject<Trk::Surface> sharedSurface(const_cast<Trk::Surface*>(currentStraw),
                                                              Trk::do_not_delete<Trk::Surface>);
                strawsPerPhiSecLayer.emplace_back(sharedSurface, strawOrderPos);
                // and record
                ++sectorStraws;
              } // loop over straws done
          }  // loop over posneg done
          // show the phiMin/phiMax to the screen
          // prepare the
          // fix to CID 24918
          if (!sectorStraws) {
            return nullptr;
          }
          double deltaPhi  = (phiMax-phiMin);
          double phiStep   = deltaPhi/(0.5*sectorStraws-1);
          ATH_MSG_VERBOSE("---> Sector " << phisec << " - with " << 0.5*sectorStraws << " straws - straw phiMin/phiMax (step) = " << phiMin << " / " << phiMax << " (" << phiStep << ")");
          // phi min / phi max
          phiMin -= 0.5*phiStep;
          phiMax += 0.5*phiStep;
          // correct for the +pi/-pi module
          // now create the BinUtility
          Trk::BinUtility* layerStrawPhiZUtility     = new Trk::BinUtility(sectorStraws/2,phiMin,phiMax,Trk::open, Trk::binPhi);
          (*layerStrawPhiZUtility)  += Trk::BinUtility(2,-layerZmax, layerZmax, Trk::open, Trk::binZ);
          // create the 2D BinnedArray
          Trk::BinnedArray2D<Trk::Surface>* layerStrawPhiSector = new Trk::BinnedArray2D<Trk::Surface>(strawsPerPhiSecLayer,layerStrawPhiZUtility);
          ATH_MSG_VERBOSE("---> Sector " << phisec << " - BinnedArray for straws prepared for " << strawsPerPhiSecLayer.size() << " straws.");
          // fill the array
          layerSectorArrays.emplace_back(layerStrawPhiSector, layerSectorPosition);
          // ---------------- enf of phi sector ----------------------------------------------------
        } // loop over PhiSectors done

        // build the mean of the layer Radius
        layerRadius = 0.5*(layerRadiusMin+layerRadiusMax)+0.5*m_layerStrawRadius;

        bool assignMaterial = false;
        if (cLayer==(unsigned)int((cMaterialLayerCount+1)*nMaterialLayerStep)) {
          assignMaterial      = true;
          ++cMaterialLayerCount;
          ATH_MSG_VERBOSE( "--> Creating a material+straw layer at radius  : " << layerRadius );
        } else
          ATH_MSG_VERBOSE( "--> Creating a straw          layer at radius  : " << layerRadius );

        // now order the plane layers to sit on cylindrical layers
        Trk::CylinderBounds* barrelLayerBounds = new Trk::CylinderBounds(layerRadius, layerHalflength);

        // ---- correct phi -------------------------------------------------------------------
        ATH_MSG_VERBOSE("    prepare approach description with " << nBarrelPhiSectors << " barrel sectors.");
        ATH_MSG_VERBOSE("    min phi / max phi detected  : " << layerPhiMin << " / " << layerPhiMax );
        double layerPhiMinCorrected = layerPhiMin-0.5*layerPhiStep;
        double layerPhiMaxCorrected = layerPhiMax+0.5*layerPhiStep;
        // catch if the minPhi falls below M_PI
        if (layerPhiMinCorrected < -M_PI){
          layerPhiMinCorrected += layerPhiStep;
          layerPhiMaxCorrected += layerPhiStep;
        }
        ATH_MSG_VERBOSE("    min phi / max phi corrected : " << layerPhiMinCorrected << " / " << layerPhiMaxCorrected );

        // the sector surfaces
        Trk::BinUtility* layerSectorBinUtility = new Trk::BinUtility(nBarrelPhiSectors,layerPhiMinCorrected,layerPhiMaxCorrected,Trk::closed,Trk::binPhi);
        auto strawArray = std::make_unique<Trk::BinnedArrayArray<Trk::Surface>>(layerSectorArrays, layerSectorBinUtility );

        ATH_MSG_VERBOSE("--> Layer " << layer << " has been built with " << strawArray->arrayObjects().size() << " straws.");

        // ApproachDescriptor
        // build a BinUtility for the ApproachDescritptor
        Trk::BinUtility* aDescriptorBinUtility = new Trk::BinUtility(nBarrelPhiSectors,layerPhiMinCorrected,layerPhiMaxCorrected,Trk::closed,Trk::binPhi);
        (*aDescriptorBinUtility) += Trk::BinUtility(2,-layerHalflength,layerHalflength,Trk::open, Trk::binZ);

        auto aDescriptorBinnedArray = std::make_unique<Trk::BinnedArray2D<const Trk::ApproachSurfaces>> (layerApproachSurfaces, aDescriptorBinUtility);

        // build an approach surface
        auto approachSurface = std::make_unique<Trk::CylinderSurface> (barrelLayerBounds->clone());
        Trk::ApproachDescriptor* aDescritpor =
          new Trk::ApproachDescriptor(std::move(aDescriptorBinnedArray),
                                      std::move( approachSurface));

        // do not give every layer material properties
        if (assignMaterial) {
          // ----- prepare the BinnedLayerMaterial -----------------------------------------------------
          Trk::BinnedLayerMaterial* layerMaterial = nullptr;
          // -- material with 1D binning
          Trk::BinUtility layerBinUtilityZ(m_barrelLayerBinsZ, -layerHalflength, layerHalflength, Trk::open, Trk::binZ );
          if (m_barrelLayerBinsPhi==1){
            layerMaterial =new Trk::BinnedLayerMaterial(layerBinUtilityZ);
          } else { // -- material with 2D binning: RPhiZ binning
            Trk::BinUtility layerBinUtilityRPhiZ(m_barrelLayerBinsPhi,
                                                 -layerRadius*M_PI, layerRadius*M_PI,
                                                 Trk::closed,
                                                 Trk::binRPhi);
            layerBinUtilityRPhiZ += layerBinUtilityZ;
            layerMaterial =new Trk::BinnedLayerMaterial(layerBinUtilityRPhiZ);
          }

          barrelLayers->push_back(new Trk::CylinderLayer(barrelLayerBounds,
                                                         std::move(strawArray),
                                                         *layerMaterial,
                                                         m_layerThickness,
                                                         std::make_unique<InDet::TRT_OverlapDescriptor>(trtIdHelper),
                                                         aDescritpor));
          delete layerMaterial;

        } else
          barrelLayers->push_back(new Trk::CylinderLayer(barrelLayerBounds,
                                                         std::move(strawArray),
                                                         m_layerThickness,
                                                         std::make_unique<InDet::TRT_OverlapDescriptor>(trtIdHelper),
                                                         aDescritpor));
      } // loop over layers
    } // loop over rings

    ATH_MSG_VERBOSE(" Built number of TRT barrel material layers: " << cMaterialLayerCount);
    // In Complex geo # of material layers should match the expected # of layers,
    // else a mis-match in layer and material map index occurs.
    // This mis-match will results layers getting incorrect material properties.
    if (cMaterialLayerCount!=m_modelBarrelLayers) {
      ATH_MSG_WARNING(" Complex geo built incorrect # of TRT barrel material layers: "
                      << cMaterialLayerCount <<  " / " <<  m_modelBarrelLayers);
    }
  }// complex geometry

  // return what you have
  return std::unique_ptr<const std::vector<Trk::CylinderLayer*> > (barrelLayers.release());
}


std::unique_ptr<const std::vector<Trk::DiscLayer*> >
InDet::TRT_LayerBuilderImpl::discLayersImpl(const InDetDD::TRT_DetElementContainer* trtContainer) const
{
  ATH_MSG_DEBUG( "Building disc-like layers for the TRT " );

  const InDetDD::TRT_Numerology* trtNums = trtContainer->getTRTNumerology();
  // get the TRT ID Helper
  const TRT_ID* trtIdHelper = nullptr;
  if (detStore()->retrieve(trtIdHelper, "TRT_ID").isFailure()) {
    ATH_MSG_ERROR("Could not get TRT ID helper");
    return nullptr;
  }
  unsigned int nEndcapWheels = trtNums->getNEndcapWheels();
  unsigned int nEndcapPhiSectors = trtNums->getNEndcapPhi();

  // total layer numbers
  int numTotalLayers = 0;

  // zMin / zMax
  double zMin = 10e10;
  double zMax = 0.;

  const Trk::DiscBounds* sectorDiscBounds = nullptr;

  // preloop for overall numbers
  for (unsigned int iwheel=0; iwheel<nEndcapWheels; ++iwheel)
    {
      unsigned int nEndcapLayers = trtNums->getNEndcapLayers(iwheel);
      numTotalLayers += nEndcapLayers;
      for (unsigned int ilayer = 0; ilayer<nEndcapLayers; ++ilayer){
        const InDetDD::TRT_EndcapElement* sectorDiscElement = trtContainer->getEndcapDetElement(0, iwheel, ilayer, 0); // TODO share this line

        // get a reference element for dimensions
        if (!sectorDiscBounds){
          const Trk::SurfaceBounds& sectorSurfaceBounds = sectorDiscElement->bounds();
          sectorDiscBounds = dynamic_cast<const Trk::DiscBounds*>(&sectorSurfaceBounds);
        }

        double currentZ = std::abs(sectorDiscElement->center().z());
        takeSmallerBigger(zMin,zMax,currentZ);
      }
    }
  if (numTotalLayers==0) {
    ATH_MSG_WARNING( "numTotalLayers = 0 ... aborting and returning 0 !" );
    return nullptr;
  }

  if (!sectorDiscBounds) {
    ATH_MSG_WARNING( "fullDiscBounds do not exist ... aborting and returning 0 !" );
    return nullptr;
  }
  auto fullDiscBounds = std::make_unique<Trk::DiscBounds>(sectorDiscBounds->rMin(), sectorDiscBounds->rMax());

  PtrVectorWrapper<Trk::DiscLayer> endcapLayers;

  // the BinUtility for the material
  std::unique_ptr<Trk::BinnedLayerMaterial> layerMaterial;
  // -- material with 1D binning
  Trk::BinUtility layerBinUtilityR(m_endcapLayerBinsR,
                                   fullDiscBounds->rMin(),
                                   fullDiscBounds->rMax(),
                                   Trk::open,
                                   Trk::binR);
  if (m_barrelLayerBinsPhi==1)
    layerMaterial = std::make_unique<Trk::BinnedLayerMaterial>(layerBinUtilityR);
  else { // -- material with 2D binning
    Trk::BinUtility layerBinUtilityPhi(m_barrelLayerBinsPhi,
                                       -M_PI, M_PI,
                                       Trk::closed,
                                       Trk::binPhi);
    // make it rPhi now
    layerBinUtilityR += layerBinUtilityPhi;
    layerMaterial = std::make_unique<Trk::BinnedLayerMaterial>(layerBinUtilityR);
  }

  // global geometry statistics
  double zDiff      = std::abs(zMax-zMin);
  double zStep      = zDiff/(m_modelEndcapLayers+1);

  // loop for surface ordering
  int maxendcaps=2;
  if (m_endcapConly) maxendcaps=1;

  for (int iposneg=0; iposneg<maxendcaps; ++iposneg){

    // fill the positions of the disc layers
    std::vector<double> zPositions;
    zPositions.reserve(m_modelEndcapLayers);

    double stepdir = iposneg ? 1. : -1.;
    double zStart = stepdir*zMin;

    ATH_MSG_VERBOSE( " -> Creating " << m_modelEndcapLayers << " disc-layers on each side between "
                     << zMin << " and " << zMax << " ( at step "<< zStep << " )");

    // take a different modelling for the layers - use these layers for the model geometry and the real geometry
    for (unsigned int izpos = 1; izpos <= m_modelEndcapLayers; ++izpos){
      zPositions.push_back(zStart + stepdir * double(izpos) * zStep - 0.5 * m_layerThickness);
    }

    std::vector<double>::const_iterator zPosIter    = zPositions.begin();
    std::vector<double>::const_iterator zPosIterEnd = zPositions.end();

    // (a) simplified geometry
    if (m_modelGeometry){
      // build the layers actually
      for ( ; zPosIter != zPosIterEnd; ++zPosIter){
        ATH_MSG_VERBOSE( "  --> Creating a layer at z pos    : " << (*zPosIter) );
        Amg::Transform3D zPosTrans =
          Amg::Transform3D(Amg::Translation3D(0., 0., (*zPosIter)));
        endcapLayers->push_back(new Trk::DiscLayer(zPosTrans,
                                                   fullDiscBounds->clone(),
                                                   *layerMaterial,
                                                   m_layerThickness));
      }

    } else {
      // (b) complex geometry
      float nMaterialLayerStep  = 1.*numTotalLayers/m_modelEndcapLayers;
      // inclusive layer counter over all wheels
      unsigned int cLayer = 0;
      // complex geo should build same # of mat. layers as model geo; counter to check this:
      unsigned int  cMaterialLayerCount = 0;

      // complex geometry - needs a little bit of joggling
      for (unsigned int iwheel=0; iwheel<nEndcapWheels; ++iwheel)
        {
          // do the loop per side
          unsigned int nEndcapLayers = trtNums->getNEndcapLayers(iwheel);
          for (unsigned int ilayer = 0; ilayer < nEndcapLayers; ++ilayer){

            // increase inclusive layer counter for next material layer
            ++cLayer;

            // count the straws;
            int numberOfStraws = 0;

            // check if dynamic cast worked
            if (fullDiscBounds){
              // get a reference element for dimensions
              const InDetDD::TRT_EndcapElement* sectorDiscElement = trtContainer->getEndcapDetElement(iposneg, iwheel, ilayer, 0); // TODO share this line

              // take the position, but not the rotation (the rotation has to be standard)
              Amg::Vector3D fullDiscPosition(sectorDiscElement->surface().transform().translation());
              double discZ = fullDiscPosition.z();

              // check if we need to build a straw layer or not
              bool assignMaterial = false;
              if (cLayer == (unsigned)int((cMaterialLayerCount+1)*nMaterialLayerStep)) {
                assignMaterial      = true;
                ++cMaterialLayerCount;
                ATH_MSG_VERBOSE( "--> Creating a material+straw layer at z-pos   : " << discZ );
              } else {
                ATH_MSG_VERBOSE( "--> Creating a straw          layer at z-pos   : " << discZ );
              }

              // order the straws onto layers
              std::vector< Trk::SurfaceOrderPosition > strawPerEndcapLayer;

              // the layer thickness - for approaching surfaces
              double zMin = 10e10;
              double zMax = -10e10;

              for (unsigned int iphisec=0; iphisec<nEndcapPhiSectors; ++iphisec){
                ATH_MSG_VERBOSE("Building sector " << iphisec << " of endcap wheel " << iwheel );
                const InDetDD::TRT_EndcapElement* currentElement = trtContainer->getEndcapDetElement(iposneg, iwheel, ilayer, iphisec); // TODO share this line
                unsigned int nstraws = currentElement->nStraws();
                for (unsigned int istraw=0; istraw<nstraws; istraw++){
                  Identifier strawId = trtIdHelper->straw_id(currentElement->identify(), istraw);
                  const Trk::Surface* currentStraw = &(currentElement->surface(strawId));
                  Amg::Vector3D strawOrderPos(currentStraw->center());
                  // get the z position
                  double zPos = currentStraw->center().z();
                  takeSmaller(zMin,zPos);
                  takeBigger(zMax,zPos);
                  // Something like
                  // Trk::SharedObject<Trk::Surface>  =
                  // std::make_shared<Trk::Surface>(currentElement)) could be fine
                  //
                  // As things are now
                  // 1) Notice that basically we couple the DetElement owned
                  // surface to the Tracking Geometry passing a no-op deleter
                  // (no delete happens) to the shared_ptr(SharedObject is
                  // typedef of shared_ptr)
                  // 2) The const_cast here make the
                  // code non MT safe. For now we handle this by being careful
                  // on lifetimes and non-re-entrant TG construction.
                  Trk::SharedObject<Trk::Surface> sharedSurface(const_cast<Trk::Surface*>(currentStraw),
                                                                [](Trk::Surface*) {});
                  strawPerEndcapLayer.emplace_back(sharedSurface, strawOrderPos);
                  ++numberOfStraws;
                }
              }
              // fix to CID 11326
              if (!numberOfStraws){
                return nullptr;
              }
              Trk::BinUtility* currentBinUtility = new Trk::BinUtility(numberOfStraws, -M_PI, M_PI, Trk::closed, Trk::binPhi);
              auto strawArray = std::make_unique<Trk::BinnedArray1D<Trk::Surface>>(strawPerEndcapLayer, currentBinUtility);
              Trk::DiscLayer* currentLayer = nullptr;

              // redefine the discZ
              discZ = 0.5*(zMin+zMax);
              Amg::Transform3D fullDiscTransform = Amg::Transform3D(Amg::Translation3D(0.,0.,discZ));

              ATH_MSG_VERBOSE("TRT Disc being build at z Position " << discZ << " ( from " << zMin << " / " << zMax << " )");

              // create the approach offset
              auto aSurfaces = std::make_unique<Trk::ApproachSurfaces>();
              // get the position of the approach surfaces
              const Amg::Vector3D aspPosition(0.,0.,zMin-m_layerStrawRadius);
              const Amg::Vector3D asnPosition(0.,0.,zMax+m_layerStrawRadius);

              // create new surfaces
              Amg::Transform3D  asnTransform = Amg::Transform3D(Amg::Translation3D(asnPosition));
              Amg::Transform3D  aspTransform = Amg::Transform3D(Amg::Translation3D(aspPosition));
              // order in an optimised way for collision direction
              if (discZ > 0.){
                aSurfaces->push_back( new Trk::DiscSurface(asnTransform, fullDiscBounds->clone()) );
                aSurfaces->push_back( new Trk::DiscSurface(aspTransform, fullDiscBounds->clone()) );
              } else {
                aSurfaces->push_back( new Trk::DiscSurface(aspTransform, fullDiscBounds->clone()) );
                aSurfaces->push_back( new Trk::DiscSurface(asnTransform, fullDiscBounds->clone()) );
              }
              // approach descriptor
              Trk::ApproachDescriptor* aDescriptor = new Trk::ApproachDescriptor(std::move(aSurfaces),false);

              // do not give every layer material properties
              if (assignMaterial)
                currentLayer = new Trk::DiscLayer(fullDiscTransform,
                                                  fullDiscBounds->clone(),
                                                  std::move(strawArray),
                                                  *layerMaterial,
                                                  m_layerThickness,
                                                  std::make_unique<InDet::TRT_OverlapDescriptor>(trtIdHelper),
                                                  aDescriptor);
              else if (!m_modelGeometry)
                currentLayer = new Trk::DiscLayer(fullDiscTransform,
                                                  fullDiscBounds->clone(),
                                                  std::move(strawArray),
                                                  m_layerThickness,
                                                  std::make_unique<InDet::TRT_OverlapDescriptor>(trtIdHelper),
                                                  aDescriptor);

              if (currentLayer) endcapLayers->push_back(currentLayer);
            } // end of sectorDiscBounds if
          } // end of layer loop
        } // end of wheel loop

      ATH_MSG_VERBOSE(" Built # of TRT material layers: " << cMaterialLayerCount << "in ispos: " << iposneg << "ring");
      // # of material layers should match the expected # of layers,
      // else a mis-match in layer and material map index occurs.
      // This mis-match will results layers getting incorrect material properties.
      if (cMaterialLayerCount != m_modelEndcapLayers) {
        ATH_MSG_WARNING(" Built incorrect # of TRT material layers: "
                        << cMaterialLayerCount <<  " / " << m_modelEndcapLayers <<  "in ispos" << iposneg << "ring" );
      }

    } // model/real geometry
  } // end of posneg loop

  return std::unique_ptr<const std::vector<Trk::DiscLayer*> > (endcapLayers.release());
}
