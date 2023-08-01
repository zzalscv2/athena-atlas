/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/MuonDetectorFactory001.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "GeoGenericFunctions/Variable.h"
#include "GeoModelInterfaces/StoredMaterialManager.h"
#include "GeoModelKernel/GeoAlignableTransform.h"
#include "GeoModelKernel/GeoBox.h"
#include "GeoModelKernel/GeoDefinitions.h"
#include "GeoModelKernel/GeoFullPhysVol.h"
#include "GeoModelKernel/GeoIdentifierTag.h"
#include "GeoModelKernel/GeoLogVol.h"
#include "GeoModelKernel/GeoMaterial.h"
#include "GeoModelKernel/GeoNameTag.h"
#include "GeoModelKernel/GeoPcon.h"
#include "GeoModelKernel/GeoPerfUtils.h"
#include "GeoModelKernel/GeoPgon.h"
#include "GeoModelKernel/GeoPhysVol.h"
#include "GeoModelKernel/GeoSerialTransformer.h"
#include "GeoModelKernel/GeoShapeShift.h"
#include "GeoModelKernel/GeoShapeSubtraction.h"
#include "GeoModelKernel/GeoShapeUnion.h"
#include "GeoModelKernel/GeoTransform.h"
#include "GeoModelKernel/GeoTrap.h"
#include "GeoModelKernel/GeoTrd.h"
#include "GeoModelKernel/GeoTube.h"
#include "GeoPrimitives/CLHEPtoEigenConverter.h"
#include "IdDictDetDescr/IdDictManager.h"
#include "MuonGeoModel/DBReader.h"
#include "MuonGeoModel/FPVMAP.h"
#include "MuonGeoModel/MYSQL.h"
#include "MuonGeoModel/MuonChamber.h"
#include "MuonGeoModel/MuonSystemDescription.h"
#include "MuonGeoModel/RDBReaderAtlas.h"
#include "MuonGeoModel/Station.h"
#include "MuonGeoModel/StationSelector.h"
#include "MuonReadoutGeometry/GenericCSCCache.h"
#include "MuonReadoutGeometry/GenericMDTCache.h"
#include "MuonReadoutGeometry/GenericRPCCache.h"
#include "MuonReadoutGeometry/GenericTGCCache.h"
#include "MuonReadoutGeometry/MuonStation.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"
#include "StoreGate/StoreGateSvc.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

typedef std::stringstream my_sstream;
typedef std::ostringstream my_osstream;

// The objects for mapping plane indexes in MuonSystem to the record index in RDBRecordset
typedef std::map<int, unsigned int, std::less<int>> muonsysIndMap;

using namespace GeoGenfun;
using namespace GeoXF;

#define skip_chambers false
#define skip_toroids false
#define skip_shielding false
#define skip_caloSaddle true
#define useAssemblies false

namespace MuonGM {

    MuonDetectorFactory001::MuonDetectorFactory001(StoreGateSvc *pDetStore):
        m_muon (std::make_unique< MuonSystemDescription>("MuonSystem")),
        m_pDetStore{pDetStore} {
        m_muon->barrelInnerRadius = 4.30 * Gaudi::Units::m;
        m_muon->innerRadius = 0.07 * Gaudi::Units::m;
        m_muon->outerRadius = 13.00 * Gaudi::Units::m;
        m_muon->endcapFrontFace = 6.74 * Gaudi::Units::m;
        m_muon->length = 22.03 * Gaudi::Units::m;
        m_muon->barreLength = 6.53 * Gaudi::Units::m;
        m_muon->barrelInterRadius = 3.83 * Gaudi::Units::m;
        m_muon->extraZ = 12.9 * Gaudi::Units::m;
        m_muon->extraR = 12.5 * Gaudi::Units::m;

        MsgStream log(Athena::getMessageSvc(), "MuonGeoModel");
        log << MSG::INFO << "MuonDetectorFactory - constructor "
            << " MuonSystem OuterRadius " << m_muon->outerRadius << " Length " << m_muon->length << endmsg;
    }

    MuonDetectorFactory001::~MuonDetectorFactory001() =default;

    const MuonDetectorManager *MuonDetectorFactory001::getDetectorManager() const { return m_manager; }
    MuonDetectorManager *MuonDetectorFactory001::getDetectorManager() { return m_manager; }

    void MuonDetectorFactory001::create(GeoPhysVol *world) {
        MsgStream log(Athena::getMessageSvc(), "MuGM:MuonFactory");

        int mem = 0;
        float cpu = 0;
        int umem = 0;
        float ucpu = 0;
        std::ofstream geoModelStats;
        if (m_dumpMemoryBreakDown) {
            geoModelStats.open("MuonGeoModelStatistics_MuonDetectorFactory");

            mem = GeoPerfUtils::getMem();
            cpu = int(GeoPerfUtils::getCpu() / 100.);

            geoModelStats << "At MuonDetectorFactory::create entry point: \t SZ= " << mem << " Kb \t Time = " << cpu << " seconds " << std::endl;

            umem = GeoPerfUtils::getMem();
            ucpu = int(GeoPerfUtils::getCpu() / 100.);
            geoModelStats << "At MuonDetectorFactory::resetting to 0:     \t SZ= " << umem - mem << " Kb \t Time = " << ucpu - cpu << " seconds " << std::endl;
            mem = umem;
            cpu = ucpu;
        } // if (m_dumpMemoryBreakDown) {

        if (!m_manager)
            m_manager = new  MuonDetectorManager();

        // check consistency of flags coming from the tool
        m_includeCutouts = 1;
        m_includeCutoutsBog = 1;
        log << MSG::INFO << "MuonLayout set to <" << m_layout << "> = Development version for DC3 - infrastructures " << endmsg;
        log << MSG::INFO << "                   BOG cutouts are activated " << m_includeCutoutsBog << " , all other cutouts are disabled " << m_includeCutouts << endmsg;

        m_manager->setCachingFlag(m_caching);
        m_manager->setCacheFillingFlag(m_cacheFillingFlag);
        // set here the flag defining the geometry granularity
        // minimalgeo = 1 => The geo tree is built up to the Detector Level (Full PhysVol)
        //                     no internal structure of the Detector is built
        // minimalgeo = 0 => The geo tree is built in full details
        m_manager->setMinimalGeoFlag(m_minimalGeoFlag);
        
        m_manager->setGeometryVersion(m_layout);
        m_manager->set_DBMuonVersion(m_DBMuonVersion);

        log << MSG::INFO << "Manager created for geometry version " << m_manager->geometryVersion() << " from DB MuonVersion <" << m_manager->get_DBMuonVersion() << ">" << endmsg;

        // here create the MYSQL singleton and assign to it the geometry version
        MYSQL::LockedMYSQL mysql = MYSQL::GetPointer();
        mysql->setGeometryVersion(m_layout);
        mysql->set_amdb_from_RDB(m_rdb == 1);

 
        StatusCode sc = StatusCode::SUCCESS;

        
        log << MSG::INFO << "MDTIDHELPER retrieved from DetStore" << endmsg;
        log << MSG::INFO << "RPCIDHELPER retrieved from DetStore" << endmsg;
        log << MSG::INFO << "TGCIDHELPER retrieved from DetStore" << endmsg;
        
        if (m_hasCSC) {
            log << MSG::INFO << "CSCIDHELPER retrieved from DetStore" << endmsg;
        }
        if (m_hasSTgc) {
            log << MSG::INFO << "STGCIDHELPER retrieved from DetStore" << endmsg;
        }
        if (m_hasMM) {
            log << MSG::INFO << "MMIDHELPER retrieved from DetStore" << endmsg;
        }

        if (m_dumpMemoryBreakDown) {
            umem = GeoPerfUtils::getMem();
            ucpu = int(GeoPerfUtils::getCpu() / 100.);
            geoModelStats << "At MuonDetectorFactory::IdHelpers loaded    \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                          << " \t Delta T =" << ucpu - cpu << std::endl;
            mem = umem;
            cpu = ucpu;
        }

        log << MSG::INFO << " **************** MuonDetectorFactory001 ************************" << endmsg;
        log << " *** Start building the Muon Geometry Tree **********************" << endmsg;

        std::string OracleTag = m_DBkey;
        std::string OracleNode = m_DBnode;

        if (log.level() <= MSG::DEBUG)
            log << MSG::DEBUG << "calling RDBReaderAtlas with m_altAsciiDBMap" << endmsg;

        std::unique_ptr<RDBReaderAtlas> dbr =std::make_unique<RDBReaderAtlas>(m_pDetStore, m_pRDBAccess, OracleTag, OracleNode, m_altAsciiDBMap);


        // set here the flag deciding whether to include cutouts:
        // m_includeCutouts = 1 => include cutouts
        // m_includeCutouts = 0 => no cutouts
        m_manager->setCutoutsFlag(m_includeCutouts);
        m_manager->setCutoutsBogFlag(m_includeCutoutsBog);

        dbr->setGeometryVersion(m_layout);
        dbr->setManager(getDetectorManager());
        sc = dbr->ProcessDB(*mysql);
        if (sc != StatusCode::SUCCESS) {
            log << MSG::ERROR << " FAILURE in DB access; Muon node will not be built" << endmsg;
            return;
        }
       
        if (m_dumpMemoryBreakDown) {
            umem = GeoPerfUtils::getMem();
            ucpu = int(GeoPerfUtils::getCpu() / 100.);
            geoModelStats << "At MuonDetectorFactory::DB read             \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                          << " \t Delta T =" << ucpu - cpu << std::endl;
            mem = umem;
            cpu = ucpu;
        }

        // now store detector-generic-descriptors into the manager
        const RPC *r = dynamic_cast<const RPC*>(mysql->GetATechnology("RPC0"));
        GenericRPCCache rpcCache;
        rpcCache.stripSeparation = r->stripSeparation;
        rpcCache.stripPanelThickness = r->stripPanelThickness;
        rpcCache.rpcLayerThickness = r->rpcLayerThickness;
        rpcCache.centralSupPanelThickness = r->centralSupPanelThickness;
        rpcCache.GasGapThickness = r->GasGapThickness;
        rpcCache.frontendBoardWidth = r->frontendBoardWidth;
        m_manager->setGenericRpcDescriptor(rpcCache);

        const MDT *mdtobj = dynamic_cast<const MDT*>(mysql->GetATechnology("MDT0"));
        GenericMDTCache mdtCache;
        mdtCache.innerRadius = mdtobj->innerRadius;
        mdtCache.outerRadius = mdtobj->innerRadius + mdtobj->tubeWallThickness;

        m_manager->setGenericMdtDescriptor(mdtCache);

        const TGC *t = dynamic_cast<const TGC*>(mysql->GetATechnology("TGC0"));
        GenericTGCCache tgcCache;
        tgcCache.frame_h = t->frame_h;
        tgcCache.frame_ab = t->frame_ab;
        tgcCache.nlayers = t->nlayers;

        m_manager->setGenericTgcDescriptor(tgcCache);

        GenericCSCCache cscCache;
        cscCache.dummy1 = 0;
        cscCache.dummy2 = 0;
        m_manager->setGenericCscDescriptor(cscCache);

        StoredMaterialManager *theMaterialManager;
        if (StatusCode::SUCCESS != m_pDetStore->retrieve(theMaterialManager, "MATERIALS")) {
            return;
        } else {
            log << MSG::INFO << " theMaterialManager retrieven successfully from the DetStore" << endmsg;
        }

        const GeoMaterial *m4 = theMaterialManager->getMaterial("std::Air");
        GeoLogVol *l4;
        GeoPcon *c4 = new GeoPcon(0, 360 * Gaudi::Units::deg);

        if (m_isAmdcDb) {
            // NOTE: the following hard coded values are ONLY needed for local validation studies of new amdb layouts
            // (when no values from the geometry database are retrieved, the values correspond to ATLAS-R2-2016-01-00-01)
            log << MSG::INFO << " Using hardcoded envelope dimesions from MuonSystem-15 table" << endmsg;
            c4->addPlane(-26046., 1050., 1500);
            c4->addPlane(-23001., 1050., 1500);
            c4->addPlane(-23001., 1050., 2750);
            c4->addPlane(-22030., 1050., 2750);
            c4->addPlane(-22030., 436.7, 12650);
            c4->addPlane(-18650., 436.7, 12650);
            c4->addPlane(-18650., 279., 13400);
            c4->addPlane(-12900., 279., 13400);
            c4->addPlane(-12900., 70., 14200);
            c4->addPlane(-6783., 70., 14200);
            c4->addPlane(-6783., 420., 14200);
            c4->addPlane(-6736., 420., 14200);
            c4->addPlane(-6736., 3800., 14200);
            c4->addPlane(-6550., 3800., 14200);
            c4->addPlane(-6550., 4255., 14200);
            c4->addPlane(-4000., 4255., 14200);
            c4->addPlane(-4000., 4255., 13000);
            c4->addPlane(4000., 4255., 13000);
            c4->addPlane(4000., 4255., 14200);
            c4->addPlane(6550., 4255., 14200);
            c4->addPlane(6550., 3800., 14200);
            c4->addPlane(6736., 3800., 14200);
            c4->addPlane(6736., 420., 14200);
            c4->addPlane(6783., 420., 14200);
            c4->addPlane(6783., 70., 14200);
            c4->addPlane(12900., 70., 14200);
            c4->addPlane(12900., 279., 13400);
            c4->addPlane(18650., 279., 13400);
            c4->addPlane(18650., 436.7, 12650);
            c4->addPlane(22030., 436.7, 12650);
            c4->addPlane(22030., 1050., 2750);
            c4->addPlane(23001., 1050., 2750);
            c4->addPlane(23001., 1050., 1500);
            c4->addPlane(26046., 1050., 1500);
        } else {

            //--- --- --- CREATE ENVELOPE --- --- ---
            // First try to get data from the GeomDB
            IRDBRecordset_ptr muonSysRec = m_pRDBAccess->getRecordsetPtr("MuonSystem", OracleTag, OracleNode);

            // -- Next two lines allow to use MuonSystem-00 by default instead of hardwired numbers
            //    even for geometry tags where MuonSystem was not collected
            if (muonSysRec->size() == 0) {
                muonSysRec = m_pRDBAccess->getRecordsetPtr("MuonSystem", "MuonSystem-00");
                log << MSG::INFO << "MuonSystem description from default node in GeomDB, i.e. MuonSystem-00" << endmsg;
            } else {
                log << MSG::INFO << "MuonSystem description from OracleTag=<" << OracleTag << "> and node=<" << OracleNode << ">" << endmsg;
            }

            // --- Envelope from DB ....
            if (muonSysRec->size() != 0) {
                // Data retrieved
                muonsysIndMap indmap;
                muonsysIndMap::const_iterator iter;
                const IRDBRecord *currentRecord;

                // First fill the contents of muonsysIndMap
                for (unsigned int ind = 0; ind < muonSysRec->size(); ind++) {
                    int key = (*muonSysRec)[ind]->getInt("PLANE_ID");
                    indmap[key] = ind;
                }

                // Create the polycone
                for (unsigned int ind = 0; ind < indmap.size(); ind++) {
                    iter = indmap.find(ind);

                    if (iter == indmap.end()) {
                        throw std::runtime_error("Error in MuonDetectorFactory, missing plane in MuonSystem");
                    } else {
                        currentRecord = (*muonSysRec)[(*iter).second];
                        c4->addPlane(currentRecord->getDouble("ZPLANE"), currentRecord->getDouble("RMIN"), currentRecord->getDouble("RMAX"));
                    }
                }
            } else { // ... end if  Envelope from DB ---
                // Muon System node is not present, go for handcoded version
                log << MSG::INFO << "MuonSystem description not available in GeomDB - using hard-wired description" << endmsg;

                double ir = m_muon->barrelInnerRadius;
                double pir = m_muon->innerRadius;
                double orad = m_muon->outerRadius;
                double l = m_muon->length;
                double eff = m_muon->endcapFrontFace;

                double extraR = m_muon->extraR;
                double extraZ = m_muon->extraZ;

                c4->addPlane(-l, pir, extraR);
                c4->addPlane(-extraZ, pir, extraR);
                c4->addPlane(-extraZ, pir, orad);

                c4->addPlane(-eff, pir, orad);
                c4->addPlane(-eff, ir, orad);
                c4->addPlane(+eff, ir, orad);
                c4->addPlane(+eff, pir, orad);

                c4->addPlane(extraZ, pir, orad);
                c4->addPlane(extraZ, pir, extraR);
                c4->addPlane(l, pir, extraR);
            } // ... end if  Envelope from DB ---
        }     // end if not m_isAmdcDb

        l4 = new GeoLogVol("MuonSys", c4, m4);

        GeoPhysVol *p4 = new GeoPhysVol(l4);

        // Cannot (yet) cope with this:
        // G4UserLimits *ul=new G4UserLimits;
        // ul->SetMaxAllowedStep(5*Gaudi::Units::cm);
        // lv->GetLogicalVolume()->SetUserLimits(ul);

        m_manager->addTreeTop(p4); // This is the top!
        log << MSG::INFO << " TreeTop added to the Manager" << endmsg;
        log << MSG::INFO << " Muon Layout " << m_layout << endmsg;

        std::vector<std::string> slist;
        if (m_selectedStations.empty())
            slist.push_back("*");
        else
            slist = m_selectedStations;

        // create the fullphysvol map to allow cloning and save memory
        std::unique_ptr<FPVMAP> savemem = std::make_unique< FPVMAP>();

        int nstat_ss = 0;
        int ntpos_ss = 0;
        int npos_ss = 0;
        int nAssemblies = 0;
        if (m_enableFineClashFixing > 0) {
            log << MSG::INFO << "Fine Clash Fixing enabled:  (should be ON/OFF for Simulation/Reconstruction)" << endmsg;
        } else {
            log << MSG::INFO << "Fine Clash Fixing disabled: (should be ON/OFF for Simulation/Reconstruction)" << endmsg;
        }

        StationSelector sel(*mysql, slist);
        StationSelector::StationIterator it;

        for (it = sel.begin(); it != sel.end(); ++it) {
            Station *station = (*it).second;
            std::string stname(station->GetName(), 0, 3);
            if (m_selectedStations.size() <= 0) {
                if (log.level() <= MSG::VERBOSE) {
                    log << MSG::VERBOSE << "Processing Stations named <" << station->GetName() << "> " << nstat_ss << " built until now" << endmsg;
                }
            }

            if ((skip_chambers) && (stname.substr(0, 1) != "X"))
                continue;

            bool isAssembly = false;
            if (station->GetNrOfCutouts() > 0 && stname.substr(0, 1) != "T")
                isAssembly = true;

            // BIR have multilayers of diff. length and overall station volume clashes with toroids
            if (stname == "BIR")
                isAssembly = true;

            MuonChamber l(*mysql, station); // here is where we start to create a MuonChamber with all readoutelements
            l.setFPVMAP(savemem.get());
            l.setFineClashFixingFlag(m_enableFineClashFixing);

            PositionIterator pit;
            AlignPosIterator apit;
            AlignPos ap;
            npos_ss = 0;
            for (pit = station->begin(); pit != station->end(); pit++) {
                int zi = (*pit).second.zindex;
                int fi = (*pit).second.phiindex;
                int sign = 1;
                if (zi)
                    sign = zi / abs(zi);

                // if there's a selection of eta location, check if this chamber is selected
                if (m_selectedStEta.size() > 0) {
                    bool selectedEta = false;
                    for (auto i = m_selectedStEta.begin(); i < m_selectedStEta.end(); i++) {
                        if (zi == *i) {
                            selectedEta = true;
                            break;
                        }
                    }

                    if (!selectedEta)
                        continue;
                }

                // if there's a selection of phi location, check if this chamber is selected
                if (m_selectedStPhi.size() > 0) {
                    bool selectedPhi = false;
                    for (auto i = m_selectedStPhi.begin(); i < m_selectedStPhi.end(); i++) {
                        if ((fi + 1) == *i) {
                            selectedPhi = true;
                            break;
                        }
                    }

                    if (!selectedPhi)
                        continue;
                }

                // total number of stations positioned in the layout
                ntpos_ss++;
                // total number of locations for this chamber
                npos_ss++;

                if ((m_selectedStPhi.size() + m_selectedStEta.size()) > 0) {
                    log << MSG::INFO << "Build selected Station <" << station->GetName() << "> at Jzz = " << zi << " Jff = " << fi + 1 << " ******* " << nstat_ss
                        << " stat.types built until now" << endmsg;
                }

                // here define GeoNameTag
                GeoNameTag *nm = new GeoNameTag(station->GetName() + "_station");
                // here build the physical volume (tree) associated to the chamber
                bool is_mirrored = ((*pit).second).isMirrored;
                int mirsign = 0;
                if (zi < 0 && !is_mirrored)
                    mirsign = 1;
                if (stname == "CSL")
                    isAssembly = true;

                // CSL because coffin shape of the station mother volume
                GeoVPhysVol *pv = l.build(*theMaterialManager, *mysql, getDetectorManager(), zi, fi, is_mirrored, isAssembly);
                if (isAssembly)
                    nAssemblies++;

                // here define GeoIdentifierTag
                if (useAssemblies || isAssembly) {
                    int sideC = 0;
                    if (zi < 0)
                        sideC = 1;
                    int geoid = (sideC * 10000 + mirsign * 1000 + abs(zi) * 100 + fi + 1) * 100000;
                    if (log.level() <= MSG::DEBUG) {
                        log << MSG::DEBUG << "Adding Station with nameTag=<" << station->GetName() + "_station"
                            << "> and geoId " << geoid << endmsg;
                    }
                    p4->add(new GeoIdentifierTag(geoid));
                } else {
                    int geoid = (mirsign * 1000 + abs(zi) * 100 + fi + 1) * sign;
                    p4->add(new GeoIdentifierTag(geoid));
                    if (log.level() <= MSG::DEBUG) {
                        log << MSG::DEBUG << "Adding Station with nameTag=<" << station->GetName() + "_station"
                            << "> and geoId " << geoid << " for zi/fi = " << zi << "/" << fi << endmsg;
                    }
                }

                // here define the GeoAlignableTransform associated to the chamber
                // nominal transform first
                GeoAlignableTransform *xf = new GeoAlignableTransform(station->getNominalTransform(*mysql, (*pit).second));

                // add tag, transform and physicalvolume associated to the chamber to the mother-volume
                p4->add(nm);
                p4->add(xf);
                p4->add(pv);

                // alignment issues and readout geometry for station
                MuonStation *mst = m_manager->getMuonStation(station->GetName(), zi, fi + 1);
                if (!mst) {
                    log << MSG::WARNING << "For Station with nameTag=<" << station->GetName() << "> at zi/fi = " << zi << "/" << fi
                        << " no MuonStation found => no possibility to align" << endmsg;
                    continue;
                }

                mst->setTransform(xf);
                GeoTrf::Transform3D tsz_to_szt = GeoTrf::RotateZ3D(-90 * Gaudi::Units::degree) * GeoTrf::RotateY3D(-90 * Gaudi::Units::degree);

                mst->setNativeToAmdbLRS(Amg::EigenTransformToCLHEP(tsz_to_szt * station->native_to_tsz_frame(*mysql, (*pit).second)));

                mst->setNominalAmdbLRSToGlobal(Amg::EigenTransformToCLHEP(station->tsz_to_global_frame(*mysql, (*pit).second) * tsz_to_szt.inverse()));

                // find correct alignment information for this position
                // xf->setDelta(DummyAline); // just in case we don't find one

                // apit = station->FindAlignPos(zi,fi);
                // number of A-lines for this station

                int nAlines = station->CountAlignPos(zi, fi);
                // nAlines=-1;
                if (nAlines == 0) {
                    if (log.level() <= MSG::DEBUG) {
                        log << MSG::DEBUG << "No A-lines(-> AlignedPositions) for this station in the static Geometry DB; setting to def." << endmsg;
                    }
                    ap.tras = ap.traz = ap.trat = ap.rots = ap.rotz = ap.rott = 0.;
                    ap.jobindex = 0;
                    mst->setDelta_fromAline(ap.tras, ap.traz, ap.trat, ap.rots, ap.rotz, ap.rott);
                } else if (nAlines > 0) {
                    AlignPosIterator alast;
                    AlignPosIterator afirst = station->getFirstAlignPosInRange(zi, fi, alast);

                    for (AlignPosIterator acurrent = afirst; acurrent != alast; ++acurrent) {
                        ap = acurrent->second;
                        // ap.rots = 0.001356;
                        // ap.rotz =-0.001113;
                        // ap.rott = 0.001067;

                        if (ap.phiindex != fi || ap.zindex != zi) {
                            log << MSG::ERROR << "Inconsistent AlignedPosition found in the static Geometry DB: aligPos.fi, zi = " << ap.phiindex << ", " << ap.zindex
                                << " for station " << station->GetName() << " at fi/zi = " << fi << "/" << zi << " AlignPos indices fi/zi " << fi << "/" << zi << endmsg;
                        }

                        if (ap.jobindex == 0) {
                            // station
                            if (log.level() <= MSG::DEBUG) {
                                log << MSG::DEBUG << "Going to set delta from A-line for station" << stname << " at zi/fi " << zi << "/" << fi << endmsg;
                                log << MSG::DEBUG << "A-line is: " << ap.tras << " " << ap.traz << " " << ap.trat << " " << ap.rots << " " << ap.rotz << " " << ap.rott << endmsg;
                            }
                            mst->setDelta_fromAline(ap.tras, ap.traz, ap.trat, ap.rots, ap.rotz, ap.rott);
                        } else {
                            // station component
                            if (log.level() <= MSG::DEBUG) {
                                log << MSG::DEBUG << "Going to set delta from A-line for component " << ap.jobindex << " of station " << stname << " at zi/fi " << zi << "/" << fi
                                    << endmsg;
                                log << MSG::DEBUG << "A-line is: " << ap.tras << " " << ap.traz << " " << ap.trat << " " << ap.rots << " " << ap.rotz << " " << ap.rott << endmsg;
                            }
                            mst->setDelta_fromAline_forComp(ap.jobindex, ap.tras, ap.traz, ap.trat, ap.rots, ap.rotz, ap.rott);
                        }
                    }
                }
            } // end loop on positions

            // number of stations realised in the layout
            if (npos_ss > 0)
                nstat_ss++;
        } // for ( it = sel.begin(); it != sel.end(); it++ ) {

        log << MSG::INFO << " **************** MuonDetectorFactory001 ****************************" << endmsg << " *** The Muon Chamber Geometry Tree is built with " << endmsg
            << " *** " << p4->getNChildVols() << " child volumes " << endmsg << " *** " << savemem->NDetectors() << " independent elements and " << endmsg << " *** "
            << savemem->NDetectorsReused() << " elements cloned or shared " << endmsg << " *** " << nstat_ss << " kinds of stations" << endmsg << " *** " << ntpos_ss
            << " stations with alignable transforms" << endmsg << " *** " << nAssemblies << " stations are described as Assemblies" << endmsg << " *** "
            << m_manager->nMuonStation() << " MuonStations " << endmsg << " *** \t " << m_manager->nMdtRE() << " MDT Readout Elements \t " << m_manager->nMdtDE()
            << " MDT Detector Elements " << endmsg << " *** \t " << m_manager->nCscRE() << " CSC Readout Elements \t " << m_manager->nCscDE() << " CSC Detector Elements " << endmsg
            << " *** \t " << m_manager->nRpcRE() << " RPC Readout Elements \t " << m_manager->nRpcDE() << " RPC Detector Elements " << endmsg << " *** \t " << m_manager->nTgcRE()
            << " TGC Readout Elements \t " << m_manager->nTgcDE() << " TGC Detector Elements " << endmsg << " ********************************************************************"
            << endmsg;

        if (m_dumpMemoryBreakDown) {
            umem = GeoPerfUtils::getMem();
            ucpu = int(GeoPerfUtils::getCpu() / 100.);
            geoModelStats << "At MuonDetectorFactory::active geo done     \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                          << " \t Delta T =" << ucpu - cpu << std::endl;
            mem = umem;
            cpu = ucpu;
        }

      
        // delete the station and technology map
        delete mysql.get();

        if (m_dumpMemoryBreakDown) {
            umem = GeoPerfUtils::getMem();
            ucpu = int(GeoPerfUtils::getCpu() / 100.);
            geoModelStats << "At MuonDetectorFactory::released access.mem \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                          << " \t Delta T =" << ucpu - cpu << std::endl;
            mem = umem;
            cpu = ucpu;
        }

        GeoNameTag *ntg = new GeoNameTag("Muon");
        world->add(ntg);
        world->add(p4);

        log << MSG::INFO << " *** Inert Material built according to DB switches and config. ****** " << endmsg;
        log << MSG::INFO << " *** The Muon Geometry Tree has " << p4->getNChildVols() << " child vol.s in total ********" << endmsg
            << " ********************************************************************\n"
            << endmsg;

        if (m_dumpMemoryBreakDown) {
            umem = GeoPerfUtils::getMem();
            ucpu = int(GeoPerfUtils::getCpu() / 100.);
            geoModelStats << "At MuonDetectorFactory::inert  geo done     \t SZ= " << umem << " Kb \t Time = " << ucpu << " seconds  ---- \t DeltaM = " << umem - mem
                          << " \t Delta T =" << ucpu - cpu << std::endl;
            mem = umem;
            cpu = ucpu;

            geoModelStats.close();
        }
    } // MuonDetectorFactory001::create

} // namespace MuonGM
