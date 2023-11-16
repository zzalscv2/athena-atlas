/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
 Holds the info related to a full station
 -----------------------------------------
 ***************************************************************************/

#include "MuonReadoutGeometry/MuonStation.h"

#include <iomanip>
#include <utility>


#include "MuonReadoutGeometry/MdtReadoutElement.h"
#include "MuonReadoutGeometry/MuonReadoutElement.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"
#include "EventPrimitives/EventPrimitivesToStringConverter.h"
#include "CxxUtils/inline_hints.h"
namespace MuonGM {

    MuonStation::MuonStation(std::string_view stName, 
                             double Ssize, double Rsize, double Zsize, 
                             double LongSsize, double LongRsize, double LongZsize, 
                             int zi, int fi, bool descratzneg) :       
        AthMessaging{"MuonStation"},
        m_statname(stName) {
        m_Ssize = Ssize;
        m_Rsize = Rsize;
        m_Zsize = Zsize;
        m_LongSsize = LongSsize;
        m_LongRsize = LongRsize;
        m_LongZsize = LongZsize;
        m_descratzneg = descratzneg;
        m_statEtaIndex = zi;
        m_statPhiIndex = fi;
    }

    MuonStation::~MuonStation() = default;

    void MuonStation::setNominalAmdbLRSToGlobal(HepGeom::Transform3D xf) {
        m_amdbl_to_global = std::make_unique<HepGeom::Transform3D>(xf);
        ATH_MSG_DEBUG("setNominalAmdbLRSToGlobal: stationName/Jff/Jzz " << getStationType() << " " << getPhiIndex() << " "<< getEtaIndex() 
                    << " Origin of AmdbLocalFrame= " << Amg::toString((*m_amdbl_to_global) * HepGeom::Point3D<double>(0., 0., 0.)));
    }
    void MuonStation::setBlineFixedPointInAmdbLRS(double s0, double z0, double t0) {
       
        ATH_MSG_DEBUG("Station " << getStationType() << " at zi/fi " << getEtaIndex() << "/" << getPhiIndex()
                    << " setting fixed point for B-lines at s0,z0,t0 =     " << s0 << " " << z0 << " " << t0 );
        

        m_BlineFixedPointInAmdbLRS.setX(s0);
        m_BlineFixedPointInAmdbLRS.setY(z0);
        m_BlineFixedPointInAmdbLRS.setZ(t0);

        ATH_MSG_DEBUG("setBlineFixedPointInAmdbLRS: stationName/Jff/Jzz " << getStationType() << " " << getPhiIndex() << " "
                << getEtaIndex() << " nominal(i.e. from-station-envelop-only) B-line fixed point " 
                << Amg::toString(m_BlineFixedPointInAmdbLRS));

    }

    HepGeom::Point3D<double> MuonStation::getBlineFixedPointInAmdbLRS() const {
        // needed to update the station-level BlineFixedPoint with data from second multilayer
        return m_BlineFixedPointInAmdbLRS;
    }

    HepGeom::Point3D<double> MuonStation::getUpdatedBlineFixedPointInAmdbLRS() const { return m_BlineFixedPointInAmdbLRS; }

    void MuonStation::updateBlineFixedPointInAmdbLRS() {
        if (!m_firstRequestBlineFixedP) return;        
        // Before correction m_BlineFixedPointInAmdbLRS has a z set at the edge of
        // lowest-z tube of the first layer of one of the two multilayers.
        // For endcap A, endcap C, and barrel A, this is correct, given the tube staggering
        // For barrel side C, given the tube staggering, the z should be at the
        // edge at the second layer, i.e. the z should be corrected by a half tube
        // pitch. Correction is thus computed only for barrel side C.
        if (barrel() && (getEtaIndex() < 0)) {
            for (auto&[jobId, alignPair] :m_REwithAlTransfInStation) {
                const MuonReadoutElement* muonRE = alignPair.first;
                
                if (muonRE->detectorType() !=Trk::DetectorElemType::Mdt) {
                    continue;
                }
                const MdtReadoutElement* mdtRE = dynamic_cast<const MdtReadoutElement*>(muonRE);
                // Correct for tube staggering on barrel side C
                double shiftInZ = -0.5 * mdtRE->tubePitch();
                // in addition, correct for 35microm glue width incorrectly applied
                double multilayerRealSize{0};
                for (int ilayer = 1; ilayer <= 2; ++ilayer) {
                    double val{0.};
                    bool wellDefined = mdtRE->getWireFirstLocalCoordAlongZ(ilayer, val);
                    if (!wellDefined) {
                       ATH_MSG_WARNING("getUpdatedBlineFixedPointInAmdbLRS: stationName/Jff/Jzz " << getStationType()
                                << " " << getPhiIndex() << " " << getEtaIndex()
                                << " cannot get wire coordinates for second tube layer");
                        val = 0.;
                    }
                    if ((ilayer == 1) || (val > multilayerRealSize)) multilayerRealSize = val;
                }
                multilayerRealSize += (mdtRE->getNtubesperlayer() - 1) * mdtRE->tubePitch();
                multilayerRealSize += mdtRE->outerTubeRadius();  // last tube: no glue width
                shiftInZ += mdtRE->getZsize() - multilayerRealSize;

                m_BlineFixedPointInAmdbLRS.setY(m_BlineFixedPointInAmdbLRS.y() + shiftInZ);
                ATH_MSG_DEBUG("getUpdatedBlineFixedPointInAmdbLRS: stationName/Jff/Jzz " << getStationType() << " "
                        << getPhiIndex() << " " << getEtaIndex() << " shiftInZ = " << shiftInZ << " re-set B-line fixed point "
                        <<Amg::toString(m_BlineFixedPointInAmdbLRS));
                break;
            }
        }
        m_firstRequestBlineFixedP = false;
    }

    void MuonStation::setDeltaAmdbLRS(HepGeom::Transform3D xf) {
        m_delta_amdb_frame = std::make_unique<HepGeom::Transform3D>(xf);
        ATH_MSG_DEBUG("Station " << getStationType() << " at zi/fi " << getEtaIndex() << "/" << getPhiIndex()
                << " adding Aline     " << std::endl
                 << "  native_to_amdbl computed from A-line " << Amg::toString(*m_native_to_amdbl) << std::endl 
                << "Station  amdbl_to_global " << endmsg << Amg::toString(*m_amdbl_to_global));
        m_transform->setDelta(Amg::CLHEPTransformToEigen(m_native_to_amdbl->inverse() * (*m_delta_amdb_frame) * (*m_native_to_amdbl)));
    }

    void MuonStation::setDelta_fromAline(double tras, double traz, double trat, double rots, double rotz, double rott) {
        // store here the angles of A-line
        m_rots = rots;
        m_rotz = rotz;
        m_rott = rott;

        HepGeom::Transform3D delta_amdb = HepGeom::Transform3D::Identity;
        if (std::abs(tras) + std::abs(traz) + std::abs(trat) + (std::abs(rots) + std::abs(rotz) + std::abs(rott)) * 1000. > 0.01) {
            // compute the delta transform in the local AMDB frame
            delta_amdb = HepGeom::TranslateX3D(tras) * HepGeom::TranslateY3D(traz) * HepGeom::TranslateZ3D(trat) *
                         HepGeom::RotateX3D(rots) * HepGeom::RotateY3D(rotz) * HepGeom::RotateZ3D(rott);
            m_hasALines = true;
        }

        // store the delta transform in the local AMDB frame
        setDeltaAmdbLRS(delta_amdb);

        ATH_MSG_DEBUG("Station " << getStationType() << " at zi/fi " << getEtaIndex() << "/" << getPhiIndex()
                << " adding Aline     " << setiosflags(std::ios::fixed) << std::setprecision(6) << std::setw(12) 
                << tras << " " << traz << " " << trat << " " << rots << " " << rotz << " " << rott << std::endl
                << "  delta_amdb computed from A-line " << Amg::toString(delta_amdb));
    }

    const MuonReadoutElement* MuonStation::getMuonReadoutElement(int jobIndex) const {
        std::map<int, pairRE_AlignTransf>::const_iterator itr = m_REwithAlTransfInStation.find(jobIndex);
        return itr !=m_REwithAlTransfInStation.end() ? itr->second.first : nullptr;
    }

    MuonReadoutElement* MuonStation::getMuonReadoutElement(int jobIndex) {
        std::map<int, pairRE_AlignTransf>::const_iterator itr = m_REwithAlTransfInStation.find(jobIndex);
        return itr !=m_REwithAlTransfInStation.end() ?  itr->second.first : nullptr;
    }

    GeoAlignableTransform* MuonStation::getComponentAlTransf(int jobIndex) const {
        std::map<int, pairRE_AlignTransf>::const_iterator itr = m_REwithAlTransfInStation.find(jobIndex);
        return itr != m_REwithAlTransfInStation.end() ? itr->second.second : nullptr;
    }

    void MuonStation::addMuonReadoutElementWithAlTransf(MuonReadoutElement* a, GeoAlignableTransform* ptrsf, int jobIndex) {
        ATH_MSG_DEBUG("addMuonReadoutElementWithAlTransf for station " << getStationName() << " at zi/fi = " << getEtaIndex()
                << "/" << getPhiIndex() << " adding new component with Alignable transf... " << a->getStationName()
                << " job ondex = " << jobIndex );
        m_REwithAlTransfInStation[jobIndex] = std::make_pair(a, ptrsf);

        ATH_MSG_DEBUG("addMuonReadoutElementWithAlTransf for station " << getStationName() << " at zi/fi = " << getEtaIndex()
                  << "/" << getPhiIndex() << " added new component - now size of map is  " << m_REwithAlTransfInStation.size());
    }

    void MuonStation::setDelta_fromAline_forComp(int jobindex, double tras, double traz, double trat, double rots, double rotz,
                                                 double rott) {
        GeoAlignableTransform* parentToChild = getComponentAlTransf(jobindex);
        if (!parentToChild) {
            ATH_MSG_WARNING( "setDelta_fromAline_forComp: WARNING: component for index " << jobindex
                    << " not found in MuonStation named " << getStationName() << " at zi/fi = " << getEtaIndex() << "/" << getPhiIndex());
            return;
        }
        if (std::abs(tras) + std::abs(traz) + std::abs(trat) + (std::abs(rots) + std::abs(rotz) + std::abs(rott)) * 1000. < 0.01) {
            ATH_MSG_DEBUG("setDelta_fromAline_forComp: A-line ignored --- too small (translations < 10microns & rotations <10microrad)");
            return;
        }

        //////////////////// this is what happens for a full station :    m_transform->setDelta( m_native_to_amdbl->inverse() *
        ///(*m_delta_amdb_frame) * (*m_native_to_amdbl) );

        HepGeom::Transform3D parentToChildT = Amg::EigenTransformToCLHEP(parentToChild->getTransform());
        HepGeom::Transform3D delta_amdb = HepGeom::TranslateX3D(tras) * HepGeom::TranslateY3D(traz) * HepGeom::TranslateZ3D(trat) *
                                          HepGeom::RotateX3D(rots) * HepGeom::RotateY3D(rotz) * HepGeom::RotateZ3D(rott);
        // The station to component transform is static and must be computed in terms of "nominal geometry parameters"; fixing here bug
        // 87693 - SS 9/11/2011 HepGeom::Point3D<double> thisREcenter = getMuonReadoutElement(jobindex)->center();
        HepGeom::Point3D<double> thisREnominalCenter = Amg::EigenTransformToCLHEP(getMuonReadoutElement(jobindex)->defTransform()) * HepGeom::Point3D<double>(0., 0., 0.);
        double Rcomp = thisREnominalCenter.perp() - (getMuonReadoutElement(jobindex)->getRsize()) / 2.;
        double DZcomp = std::abs(thisREnominalCenter.z()) - std::abs(((*m_amdbl_to_global) * HepGeom::Point3D<double>(0., 0., 0)).z()) -
                        std::abs((getMuonReadoutElement(jobindex)->getZsize()) / 2.);

        HepGeom::Transform3D childToLocAmdbStation = (*m_native_to_amdbl) * parentToChildT;
        HepGeom::Transform3D locAmdbStatToLocAmdbComp = HepGeom::Transform3D::Identity;
        // the following line is needed to go for scenario B in last slide of
        // http://www.fisica.unisalento.it/~spagnolo/allow_listing/TGC_Alines/TGC-ALines_2011_03_01.pdf COMMENT next line            to go
        // for scenario A in last slide of http://www.fisica.unisalento.it/~spagnolo/allow_listing/TGC_Alines/TGC-ALines_2011_03_01.pdf
        if (getStationType()[0] == 'T') locAmdbStatToLocAmdbComp = HepGeom::TranslateY3D(-Rcomp) * HepGeom::TranslateZ3D(-DZcomp);
        HepGeom::Transform3D childToLocAmdbComponent = locAmdbStatToLocAmdbComp * childToLocAmdbStation;

        ATH_MSG_DEBUG("setDelta_fromAline_forComp: stationName/Jff/Jzz " << getStationType() << " " << getPhiIndex() << " "
                << getEtaIndex() << " Job " << jobindex << " Origin of component/station AmdbLocalFrame= "
                <<Amg::toString((*m_amdbl_to_global) * locAmdbStatToLocAmdbComp.inverse() * HepGeom::Point3D<double>(0., 0., 0.)) << " / "
                << Amg::toString((*m_amdbl_to_global) * HepGeom::Point3D<double>(0., 0., 0.)));

        parentToChild->setDelta(Amg::CLHEPTransformToEigen(childToLocAmdbComponent.inverse() * delta_amdb * childToLocAmdbComponent));
        ATH_MSG_DEBUG("setDelta_fromAline_forComp2:stationName/Jff/Jzz " << getStationType() << " " << getPhiIndex() << " "
                << getEtaIndex() << " Job " << jobindex << " Origin of component/station AmdbLocalFrame= "
                << Amg::toString((*m_amdbl_to_global) * locAmdbStatToLocAmdbComp.inverse() * HepGeom::Point3D<double>(0., 0., 0.)) << " / "
                << Amg::toString((*m_amdbl_to_global) * HepGeom::Point3D<double>(0., 0., 0.)));

        ATH_MSG_DEBUG("Station " << getStationType() << " at zi/fi " << getEtaIndex() << "/" << getPhiIndex()
                    << " adding Aline     " << tras << " " << traz << " " << trat << " " << rots << " " << rotz << " " << rott
                    << " for component with index =" << jobindex << std::endl 
                    << "  delta_amdb computed from A-line " <<Amg::toString(delta_amdb));
    }

    void MuonStation::clearCache() {   
        ATH_MSG_DEBUG("n. of RE in this station is " << m_REwithAlTransfInStation.size());
        for (auto& [jobId, readAlignPair] : m_REwithAlTransfInStation) {
            ATH_MSG_DEBUG("Clearing cache .... for RE ... iteration n. " << jobId);
            MuonReadoutElement* re = readAlignPair.first;
            if (!re) {
                ATH_MSG_WARNING(" in MuonStation:clearCache " << getStationType() << " at zi/fi " << getEtaIndex() << "/"
                        << getPhiIndex() << " trying to get a not existing RE (iteration n. )   " << jobId << " RE is null, skipping" );
                continue;
            }
            re->clearCache();
            ATH_MSG_DEBUG("cache cleared ");
        }
    }

    void MuonStation::refreshCache() {
        ATH_MSG_DEBUG("n. of RE in this station is " << m_REwithAlTransfInStation.size());
        for (auto& [jobId, readAlignPair] : m_REwithAlTransfInStation) {
            ATH_MSG_DEBUG("refreshCache cache .... for RE ... iteration n. " << jobId);
            MuonReadoutElement* re = readAlignPair.first;
            if (!re) {
                ATH_MSG_WARNING(" in MuonStation:refreshCache " << getStationType() << " at zi/fi " << getEtaIndex() << "/"
                        << getPhiIndex() << " trying to get a not existing RE (iteration n. )   " << jobId << " RE is null, skipping" );
                continue;
            }
            re->refreshCache();
        }
    }

    void MuonStation::fillCache() {
        for (auto& [jobId, readAlignPair] : m_REwithAlTransfInStation) {
            ATH_MSG_DEBUG("fillCache cache .... for RE ... iteration n. " << jobId);
            MuonReadoutElement* re = readAlignPair.first;
            if (!re) {
                ATH_MSG_WARNING(" in MuonStation:fillCache " << getStationType() << " at zi/fi " << getEtaIndex() << "/"
                        << getPhiIndex() << " trying to get a not existing RE (iteration n. )   " << jobId << " RE is null, skipping" );
                continue;
            }
            re->fillCache();
        }
    }

    void MuonStation::setBline(const BLinePar* bline) {
        m_hasBLines = true;
        for (auto& [jobId, readAlignPair] : m_REwithAlTransfInStation) {
            ATH_MSG_DEBUG("fillCache cache .... for RE ... iteration n. " << jobId);
            MuonReadoutElement* re = readAlignPair.first;
            if (!re) {                
                ATH_MSG_WARNING(" in setBLine " << getStationType() << " at zi/fi " << getEtaIndex() << "/" << getPhiIndex()
                             << " trying to get a null MuonReadoutElement, skipping");
                continue;
            }
            if (re->detectorType() !=Trk::DetectorElemType::Mdt) {
                continue;
            }
            MdtReadoutElement* mdt = dynamic_cast<MdtReadoutElement*>(re);
            mdt->setBLinePar(bline);
        }
    }

    void MuonStation::clearBLineCache() {
        for (auto& [jobId, readAlignPair] : m_REwithAlTransfInStation) {
            MuonReadoutElement* re = readAlignPair.first;
            if (!re) {
               ATH_MSG_WARNING(" in MuonStation:clearBLineCache " << getStationType() << " at zi/fi " << getEtaIndex() << "/"
                            << getPhiIndex() << " trying to get a not existing RE (iteration n. )   " << jobId << " RE is null, skipping");
                continue;
            }
            if (re->detectorType() !=Trk::DetectorElemType::Mdt) {
                continue;
            }
            MdtReadoutElement* mdt = dynamic_cast<MdtReadoutElement*>(re);
            mdt->clearBLineCache();           
        }
    }
    void MuonStation::fillBLineCache() {
        for (auto& [jobId, readAlignPair] : m_REwithAlTransfInStation) {
            MuonReadoutElement* re = readAlignPair.first;
            if (!re) {
                ATH_MSG_WARNING(" in MuonStation:fillBLineCache " << getStationType() << " at zi/fi " << getEtaIndex() << "/"
                        << getPhiIndex() << " trying to get a non existing RE, skipping "<<jobId);
                continue;
            }
            if (re->detectorType() !=Trk::DetectorElemType::Mdt) {
                continue;
            }
            MdtReadoutElement* mdt = dynamic_cast<MdtReadoutElement*>(re);
            mdt->fillBLineCache();            
        }
    }

#if defined(FLATTEN)
    // We compile this package with optimization, even in debug builds; otherwise,
    // the heavy use of Eigen makes it too slow.  However, from here we may call
    // to out-of-line Eigen code that is linked from other DSOs; in that case,
    // it would not be optimized.  Avoid this by forcing all Eigen code
    // to be inlined here if possible.
    ATH_FLATTEN
#endif
    double
    MuonStation::RsizeMdtStation() const {
        if (getStationName()[0] == 'T' || getStationName()[0] == 'C') return 0.;  // TGC and CSC stations
        double Rsize = 0.;

        Amg::Vector3D RposFirst{Amg::Vector3D::Zero()}, Rpos{Amg::Vector3D::Zero()};
        bool first = true;
        int nmdt = 0;
        ATH_MSG_DEBUG("RsizeMdtStation for " << getStationType() << " at zi/fi " << getEtaIndex() << "/" << getPhiIndex()
                << " nRE = " << nMuonReadoutElements());
        
        for (int j = 1; j < 30; ++j) {
            const MuonReadoutElement* activeComponent = getMuonReadoutElement(j);
            if (!activeComponent) continue;
            if (activeComponent->detectorType() !=Trk::DetectorElemType::Mdt) {
                continue;
            }
            ++nmdt;
            Rsize += activeComponent->getRsize() / 2.;
            Rpos = activeComponent->toParentStation().translation();
            if (first) {
                RposFirst = Rpos;
                first = false;
            } else {
                if (barrel())
                    Rsize += std::abs(Rpos.x() - RposFirst.x());
                else
                    Rsize += std::abs(Rpos.y() - RposFirst.y());
            }            
        }
        if (nmdt == 1) Rsize = 2. * Rsize;
        return Rsize;
    }
    double MuonStation::ZsizeMdtStation() const {
        if (getStationName()[0] == 'T' || getStationName()[0] == 'C') return 0.;  // TGC and CSC stations
        double Zsize = 0.;

        Amg::Vector3D ZposFirst{Amg::Vector3D::Zero()}, Zpos{Amg::Vector3D::Zero()};
        bool first = true;
        int nmdt = 0;


        ATH_MSG_DEBUG("ZsizeMdtStation for " << getStationType() << " at zi/fi " << getEtaIndex() << "/" << getPhiIndex()
                << " nRE = " << nMuonReadoutElements());

         for (int j = 1; j < 30; ++j) {
            const MuonReadoutElement* activeComponent = getMuonReadoutElement(j);
            if (!activeComponent) continue;
            if (activeComponent->detectorType() !=Trk::DetectorElemType::Mdt) {
                continue;
            }
            ++nmdt;

            Zsize += activeComponent->getZsize() / 2.;
            Zpos = activeComponent->toParentStation() * Amg::Vector3D(0., 0., 0.);
            if (first) {
                ZposFirst = Zpos;
                first = false;
            } else {
                if (barrel())
                    Zsize += std::abs(Zpos.z() - ZposFirst.z());
                else
                    Zsize += std::abs(Zpos.x() - ZposFirst.x());
            }        
        }
        if (nmdt == 1) Zsize = 2. * Zsize;

        return Zsize;
    }

    bool MuonStation::barrel() const {
        return getStationName()[0] == 'B';
    }
    bool MuonStation::endcap() const { return !barrel(); }

    const MdtAsBuiltPar* MuonStation::getMdtAsBuiltParams() const {
        ATH_MSG_WARNING("No Mdt AsBuilt parameters for chamber " << getStationName());
        return m_XTomoData;
    }

    void MuonStation::setMdtAsBuiltParams(const MdtAsBuiltPar* xtomo) { m_XTomoData = xtomo; }

}  // namespace MuonGM
