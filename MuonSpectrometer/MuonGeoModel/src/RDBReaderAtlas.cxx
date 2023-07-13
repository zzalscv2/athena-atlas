/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonGeoModel/RDBReaderAtlas.h"

#include "AmdcDb/AmdcDb.h"
#include "GaudiKernel/SystemOfUnits.h"
#include "MuonGeoModel/MdtComponent.h"
#include "MuonGeoModel/StationSelector.h"
#include "MuonGeoModel/TGC_Technology.h"
#include "MuonReadoutGeometry/GlobalUtilities.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "MuonReadoutGeometry/TgcReadoutParams.h"
#include "RDBAccessSvc/IRDBAccessSvc.h"
#include "RDBAccessSvc/IRDBRecord.h"
#include "RDBAccessSvc/IRDBRecordset.h"

namespace MuonGM {

    RDBReaderAtlas::RDBReaderAtlas(StoreGateSvc *pDetStore, IRDBAccessSvc *pRDBAccess, const std::string& geoTag, const std::string& geoNode, bool dumpAlinesFromOracle,
                                   bool useCscInternalAlinesFromOracle, bool dumpCscInternalAlinesFromOracle, const std::map<std::string, std::string> *asciiFileDBMap)
        : DBReader(pDetStore), m_controlCscIntAlines(0), m_dhdbam(nullptr), m_dbam(nullptr), m_dhatyp(nullptr), m_atyp(nullptr), m_dhasmp(nullptr), m_asmp(nullptr),
          m_dhalmn(nullptr), m_almn(nullptr), m_dhaptp(nullptr), m_aptp(nullptr), m_dhwrpc(nullptr), m_wrpc(nullptr), m_dhwtgc(nullptr), m_wtgc(nullptr), m_dhacut(nullptr),
          m_acut(nullptr), m_dhalin(nullptr), m_alin(nullptr), m_dhwmdt(nullptr), m_wmdt(nullptr), m_dhwcsc(nullptr), m_wcsc(nullptr), m_dhwrpcall(nullptr), m_wrpcall(nullptr),
          m_dhwtgcall(nullptr), m_wtgcall(nullptr), m_dhwded(nullptr), m_wded(nullptr), m_dhwsup(nullptr), m_wsup(nullptr), m_dhwspa(nullptr), m_wspa(nullptr), m_dhwchv(nullptr),
          m_wchv(nullptr), m_dhwcro(nullptr), m_wcro(nullptr), m_dhwcmi(nullptr), m_wcmi(nullptr), m_dhwlbi(nullptr), m_wlbi(nullptr), m_dhaszt(nullptr), m_aszt(nullptr),
          m_dhiacsc(nullptr), m_iacsc(nullptr), m_dhxtomo(nullptr), m_xtomo(nullptr), m_geoTag(geoTag), m_geoNode(geoNode), m_pRDBAccess(pRDBAccess),
          m_useICSCAlines(useCscInternalAlinesFromOracle) {
        m_msgSvc = Athena::getMessageSvc();
        MsgStream log(m_msgSvc, "MuGM:RDBReadAtlas");
        m_SCdbaccess = StatusCode::FAILURE;

        AmdcDb *theAmdcDb = dynamic_cast<AmdcDb *>(m_pRDBAccess);
        if (theAmdcDb) {
            log << MSG::INFO << "You are now using tables provided by the AmdcDb!!" << endmsg;
        } else {
            log << MSG::INFO << "Start retriving dbObjects with tag = <" << geoTag << "> node <" << geoNode << ">" << endmsg;
        }
        // here putting RDB data in private "objects" form
        std::unique_ptr<IRDBQuery> dbdata;
        if (theAmdcDb) {
            m_dhatyp = new DblQ00Atyp(theAmdcDb);
        } else {
            m_dhatyp = new DblQ00Atyp(m_pRDBAccess, geoTag, geoNode);
        }
        m_atyp = m_dhatyp->data();

        if (theAmdcDb) {
            m_dhasmp = new DblQ00Asmp(theAmdcDb);
        } else {
            m_dhasmp = new DblQ00Asmp(m_pRDBAccess, geoTag, geoNode);   
        }
        m_asmp = m_dhasmp->data();

        if (theAmdcDb) {
            m_dhalmn = new DblQ00Almn(theAmdcDb);
        } else {
            m_dhalmn = new DblQ00Almn(m_pRDBAccess, geoTag, geoNode);   
        }
        m_almn = m_dhalmn->data();

        if (theAmdcDb) {
            m_dhaptp = new DblQ00Aptp(theAmdcDb);
        } else {
            m_dhaptp = new DblQ00Aptp(m_pRDBAccess, geoTag, geoNode);   
        }
        m_aptp = m_dhaptp->data();

        if (theAmdcDb) {
            m_dhacut = new DblQ00Acut(theAmdcDb);
        } else {
            m_dhacut = new DblQ00Acut(m_pRDBAccess, geoTag, geoNode);   
        }
        m_acut = m_dhacut->data();

        if (theAmdcDb) {
            m_dhalin = new DblQ00Alin(theAmdcDb);
        } else {
            m_dhalin = new DblQ00Alin(m_pRDBAccess, geoTag, geoNode);   
        }
        m_alin = m_dhalin->data();

        if (theAmdcDb) {
            m_dhdbam = new DblQ00Dbam(theAmdcDb);
        } else {
            m_dhdbam = new DblQ00Dbam(m_pRDBAccess, geoTag, geoNode);   
        }
        m_dbam = m_dhdbam->data();

        if (theAmdcDb) {
            m_dhwrpc = new DblQ00Awln(theAmdcDb);
        } else {
            m_dhwrpc = new DblQ00Awln(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wrpc = m_dhwrpc->data();

        if (theAmdcDb) {
            m_dhwtgc = new DblQ00Atln(theAmdcDb);
        } else {
            m_dhwtgc = new DblQ00Atln(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wtgc = m_dhwtgc->data();

        if (theAmdcDb) {
            m_dhwmdt = new DblQ00Wmdt(theAmdcDb);
        } else {
            m_dhwmdt = new DblQ00Wmdt(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wmdt = m_dhwmdt->data();

        if (theAmdcDb) {
            m_dhwcsc = new DblQ00Wcsc(theAmdcDb);
        } else {
            m_dhwcsc = new DblQ00Wcsc(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wcsc = m_dhwcsc->data();

        if (theAmdcDb) {
            m_dhwrpcall = new DblQ00Wrpc(theAmdcDb);
        } else {
            m_dhwrpcall = new DblQ00Wrpc(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wrpcall = m_dhwrpcall->data();

        if (theAmdcDb) {
            m_dhwtgcall = new DblQ00Wtgc(theAmdcDb);
        } else {
            m_dhwtgcall = new DblQ00Wtgc(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wtgcall = m_dhwtgcall->data();

        if (theAmdcDb) {
            m_dhwspa = new DblQ00Wspa(theAmdcDb);
        } else {
            m_dhwspa = new DblQ00Wspa(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wspa = m_dhwspa->data();

        if (theAmdcDb) {
            m_dhwded = new DblQ00Wded(theAmdcDb);
        } else {
            m_dhwded = new DblQ00Wded(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wded = m_dhwded->data();

        if (theAmdcDb) {
            m_dhwsup = new DblQ00Wsup(theAmdcDb);
        } else {
            m_dhwsup = new DblQ00Wsup(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wsup = m_dhwsup->data();

        // Mdt AsBuilt parameters
        if (theAmdcDb) {
            log << MSG::INFO << "skipping XtomoData" << endmsg;
        } else {
	  std::unique_ptr<IRDBQuery> xtomoData;
  	    if (m_pRDBAccess->getRecordsetPtr("XtomoData", geoTag, geoNode)->size()!=0) {
	        m_dhxtomo= new DblQ00Xtomo(pRDBAccess, geoTag, geoNode);
	        log << MSG::INFO << "XtomoData table found in Oracle" << endmsg;  
	    }
	    else {
	        m_dhxtomo= new DblQ00Xtomo();
	        log << MSG::INFO << "No XtomoData table in Oracle" << endmsg;	    
	     }
        }
        if (m_dhxtomo)
            m_xtomo = m_dhxtomo->data();

        // ASZT
        if (asciiFileDBMap != nullptr && asciiFileDBMap->find("ASZT") != asciiFileDBMap->end()) {

            log << MSG::INFO << "getting aszt from ascii file - named <" << asciiFileDBMap->find("ASZT")->second << ">" << endmsg;
            log << MSG::INFO << "Ascii aszt input has priority over A-lines in ORACLE; A-lines from Oracle will not be read" << endmsg;
            // dbdata = 0;
            m_dhaszt = new DblQ00Aszt(asciiFileDBMap->find("ASZT")->second);

            if (m_dhaszt->size() == 0) {
                log << MSG::ERROR << "Couldn't read ASZT from ascii file!" << endmsg;
            } else {
                log << MSG::INFO << "N. of lines read = " << m_dhaszt->size() << endmsg;
            }
        }

        if (m_dhaszt == nullptr || m_dhaszt->size() == 0) {
            log << MSG::INFO << "No Ascii aszt input found: looking for A-lines in ORACLE" << endmsg;

            if (theAmdcDb) {
                m_dhaszt = new DblQ00Aszt(theAmdcDb);
            } else {
  	        if (m_pRDBAccess->getRecordsetPtr("ASZT",geoTag,geoNode)->size()==0) {
                    m_dhaszt = new DblQ00Aszt();
                    log << MSG::INFO << "No ASZT table in Oracle" << endmsg;
                } else {
                    log << MSG::INFO << "ASZT table found in Oracle" << endmsg;
                    m_dhaszt = new DblQ00Aszt(m_pRDBAccess, geoTag, geoNode);   
                    log << MSG::INFO << "ASZT size is " << m_dhaszt->size() << endmsg;
                }
            }
        } else {
            log << MSG::INFO << "ASZT table in Oracle, if any, will not be read" << endmsg;
        }
        if (m_dhaszt)
            m_aszt = m_dhaszt->data();

        //
        if (dumpAlinesFromOracle && m_dhaszt) {
            log << MSG::DEBUG << "writing ASZT values to file" << endmsg;
            m_dhaszt->WriteAsztToAsciiFile("aszt_fromAscii_or_Oracle.txt");
        }

        // Internal CSC Alignment parameters
        if (asciiFileDBMap != nullptr && asciiFileDBMap->find("IACSC") != asciiFileDBMap->end()) {

            log << MSG::INFO << "getting iacsc from ascii file - named <" << asciiFileDBMap->find("IACSC")->second << ">" << endmsg;
            log << MSG::INFO << "Ascii iacsc input has priority over A-lines in ORACLE; A-lines from Oracle will not be read" << endmsg;
            // dbdata = 0;
            m_dhiacsc = new DblQ00IAcsc(asciiFileDBMap->find("IACSC")->second);
            if (m_dhiacsc->size() == 0) {
                log << MSG::ERROR << "Couldn't read IACSC from ascii file!" << endmsg;
            } else {
                log << MSG::INFO << "N. of lines read = " << m_dhiacsc->size() << endmsg;
            }
        }
        if (m_dhiacsc == nullptr || m_dhiacsc->size() == 0) {
            log << MSG::INFO << "No Ascii iacsc input found: looking for A-lines in ORACLE" << endmsg;
            if (theAmdcDb) {
                log << MSG::INFO << "skipping ISZT" << endmsg;
                m_dhiacsc = nullptr;
            } else {
	      if (m_pRDBAccess->getRecordsetPtr("IZST", geoTag,geoNode)->size()==0) {
                    m_dhiacsc = new DblQ00IAcsc();
                    log << MSG::INFO << "No ISZT table in Oracle" << endmsg;
                } else {
		  log << MSG::INFO << "ISZT table found in Oracle" << endmsg;
		  m_dhiacsc = new DblQ00IAcsc(m_pRDBAccess, geoTag, geoNode);                   }
            }
        } else {
            log << MSG::INFO << "ISZT table in Oracle, if any, will not be read" << endmsg;
        }
        if (m_dhiacsc)
            m_iacsc = m_dhiacsc->data();

        //
        if (dumpCscInternalAlinesFromOracle && m_dhiacsc) {
            log << MSG::DEBUG << "writing ISZT values to file" << endmsg;
            m_dhiacsc->WriteIAcscToAsciiFile("IAcsc_fromAscii_or_Oracle.txt");
        }

        if (theAmdcDb) {
            m_dhwchv = new DblQ00Wchv(theAmdcDb);
        } else {
            m_dhwchv = new DblQ00Wchv(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wchv = m_dhwchv->data();

        if (theAmdcDb) {
            m_dhwcro = new DblQ00Wcro(theAmdcDb);
        } else {
            m_dhwcro = new DblQ00Wcro(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wcro = m_dhwcro->data();

        if (theAmdcDb) {
            m_dhwcmi = new DblQ00Wcmi(theAmdcDb);
        } else {
            m_dhwcmi = new DblQ00Wcmi(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wcmi = m_dhwcmi->data();

        if (theAmdcDb) {
            m_dhwlbi = new DblQ00Wlbi(theAmdcDb);
        } else {
             m_dhwlbi = new DblQ00Wlbi(m_pRDBAccess, geoTag, geoNode);   
        }
        m_wlbi = m_dhwlbi->data();

        // everything fetched
        m_SCdbaccess = StatusCode::SUCCESS;
        log << MSG::INFO << "Access granted for all dbObjects needed by muon detectors" << endmsg;
    }

    StatusCode RDBReaderAtlas::ProcessDB(MYSQL& mysql) {
        MsgStream log(m_msgSvc, "MuGM:RDBReadAtlas");
        // Check access to the database (from the constructor)
        if (m_SCdbaccess == StatusCode::FAILURE) {
            return m_SCdbaccess;
        }

        // set GeometryVersion in MYSQL
        mysql.setGeometryVersion(getGeometryVersion());
        // set LayoutName read from amdb
        mysql.setLayoutName(m_dbam[0].amdb);
        // set NovaVersion     in MYSQL
        mysql.setNovaVersion(m_dbam[0].version);
        // set AmdbVersion     in MYSQL
        mysql.setNovaReadVersion(m_dbam[0].nvrs);

        // Process Stations and components
        MuonGM::ProcessStations(mysql, m_dhalmn, m_almn, m_dhatyp, m_atyp, m_dhwmdt, m_wmdt);

        // Process Technologies
        ProcessTechnologies(mysql);

        // Process Positions
        MuonGM::ProcessPositions(mysql, m_dhaptp, m_aptp);

        // Process Cutouts
        if (getGeometryVersion().substr(0, 1) != "P") {
            MuonGM::ProcessCutouts(mysql, m_dhacut, m_acut, m_dhalin, m_alin, m_dhatyp, m_atyp);
        }

        // Process Alignements
        if (m_dhaszt && m_dhaszt->size() > 0) {
            MuonGM::ProcessAlignements(mysql, m_dhaszt, m_aszt);
        }

        // Process TgcReadout
        RDBReaderAtlas::ProcessTGCreadout(mysql);

        // Process CSC Internal Alignements
        if (m_dhiacsc && m_dhiacsc->size() > 0 && m_useICSCAlines) {
            ProcessCscInternalAlignments();
        }

        //
        log << MSG::INFO << "Intermediate Objects built from primary numbers" << endmsg;

        return m_SCdbaccess;
    }
    RDBReaderAtlas::~RDBReaderAtlas() {
        delete m_dhdbam;
        delete m_dhatyp;
        delete m_dhasmp;
        delete m_dhaszt;
        delete m_dhiacsc;
        delete m_dhalmn;
        delete m_dhaptp;
        delete m_dhwmdt;
        delete m_dhwrpc;
        delete m_dhwrpcall;
        delete m_dhwcsc;
        delete m_dhwtgc;
        delete m_dhwtgcall;
        delete m_dhalin;
        delete m_dhacut;
        delete m_dhwded;
        delete m_dhwspa;
        delete m_dhwsup;
        delete m_dhwchv;
        delete m_dhwcro;
        delete m_dhwcmi;
        delete m_dhwlbi;
        delete m_dhxtomo;
    }

    void RDBReaderAtlas::ProcessCscInternalAlignments() {
        MsgStream log(m_msgSvc, "RDBReaderAtlas::ProcessCscInternalAlignments");

        for (unsigned int ipos = 0; ipos < m_dhiacsc->size(); ++ipos) {
            std::string name = std::string(m_iacsc[ipos].type, 0, 3);
            int jff = m_iacsc[ipos].jff;
            int jzz = m_iacsc[ipos].jzz;
            int job = m_iacsc[ipos].job;             // JOB POSITION
            int wireLayer = m_iacsc[ipos].wireLayer; // WIRE LAYER
            float tras = 0.;
            float traz = 0.;
            float trat = 0.;
            float rots = 0.;
            float rotz = 0.;
            float rott = 0.;
            // here use m_controlCscIntAlines;
            if (m_controlCscIntAlines >= 111111) {
                tras = m_iacsc[ipos].tras; // S TRANSLATION MM
                traz = m_iacsc[ipos].traz; // Z TRANSLATION MM
                trat = m_iacsc[ipos].trat; // T TRANSLATION MM
                rots = m_iacsc[ipos].rots; // S ROTATION
                rotz = m_iacsc[ipos].rotz; // Z ROTATION
                rott = m_iacsc[ipos].rott; // T ROTATION
            } else {
                if (m_controlCscIntAlines % 10 != 0) {
                    rott = m_iacsc[ipos].rott; // T ROTATION
                }
                if (int(m_controlCscIntAlines / 10) % 10 != 0) {
                    rotz = m_iacsc[ipos].rotz;
                }
                if (int(m_controlCscIntAlines / 100) % 10 != 0) {
                    rots = m_iacsc[ipos].rots; // T ROTATION
                }
                if (int(m_controlCscIntAlines / 1000) % 10 != 0) {
                    trat = m_iacsc[ipos].trat; // T ROTATION
                }
                if (int(m_controlCscIntAlines / 10000) % 10 != 0) {
                    traz = m_iacsc[ipos].traz; // T ROTATION
                }
                if (int(m_controlCscIntAlines / 100000) % 10 != 0) {
                    tras = m_iacsc[ipos].tras; // T ROTATION
                }
            }
            ALinePar myPar;
            myPar.setParameters(tras, traz, trat, rots, rotz, rott);
            log << MSG::VERBOSE<<name<<","<<jff<<","<<jzz<<","<<job<<","<<wireLayer<<" "<<endmsg;
        }

        return;
    }

    void RDBReaderAtlas::ProcessTechnologies(MYSQL& mysql) {
        MsgStream log(m_msgSvc, "MuGM:ProcTechnol.s");
        // here loop over station-components to init technologies at each new entry
        std::vector<std::string> slist;
        slist.push_back("*");
        StationSelector sel(mysql, slist);
        StationSelector::StationIterator it;
        log << MSG::DEBUG << " from RDBReaderAtlas --- start " << endmsg;

        bool have_spa_details = (getGeometryVersion().substr(0, 1) != "P");

        for (it = sel.begin(); it != sel.end(); ++it) {
            Station *station = (*it).second;
            for (int ic = 0; ic < station->GetNrOfComponents(); ic++) {
                Component *c = station->GetComponent(ic);
                if (c == nullptr)
                    continue;
                const std::string &cname = c->name;

                if (cname.compare(0, 3, "CSC") == 0)
                    MuonGM::ProcessCSC(mysql, m_dhwcsc, m_wcsc, cname);
                else if (cname.compare(0, 3, "MDT") == 0)
                    MuonGM::ProcessMDT(mysql, m_dhwmdt, m_wmdt, cname);
                else if (cname.compare(0, 3, "RPC") == 0)
                    MuonGM::ProcessRPC(mysql, m_dhwrpc, m_wrpc, m_dhwrpcall, m_wrpcall, cname);
                else if (cname.compare(0, 3, "TGC") == 0)
                    MuonGM::ProcessTGC(mysql, m_dhwtgc, m_wtgc, m_dhwtgcall, m_wtgcall, cname);
                else if (cname.compare(0, 3, "SPA") == 0)
                    MuonGM::ProcessSPA(mysql, m_dhwspa, m_wspa, cname);
                else if (cname.compare(0, 3, "DED") == 0)
                    MuonGM::ProcessDED(mysql, m_dhwded, m_wded, cname);
                else if (cname.compare(0, 3, "SUP") == 0)
                    MuonGM::ProcessSUP(mysql, m_dhwsup, m_wsup, cname);
                else if (cname.compare(0, 3, "CHV") == 0 && have_spa_details)
                    MuonGM::ProcessCHV(mysql, m_dhwchv, m_wchv, cname);
                else if (cname.compare(0, 3, "CRO") == 0 && have_spa_details)
                    MuonGM::ProcessCRO(mysql, m_dhwcro, m_wcro, cname);
                else if (cname.compare(0, 3, "CMI") == 0 && have_spa_details)
                    MuonGM::ProcessCMI(mysql, m_dhwcmi, m_wcmi, cname);
                else if (cname.compare(0, 2, "LB") == 0 && have_spa_details)
                    MuonGM::ProcessLBI(mysql, m_dhwlbi, m_wlbi, cname);
            }
        }

        log << MSG::INFO << "nMDT " << nmdt << " nCSC " << ncsc << " nTGC " << ntgc << " nRPC " << nrpc << endmsg;
        log << MSG::INFO << "nDED " << nded << " nSUP " << nsup << " nSPA " << nspa << endmsg;
        log << MSG::INFO << "nCHV " << nchv << " nCRO " << ncro << " nCMI " << ncmi << " nLBI " << nlbi << endmsg;
    }

    void RDBReaderAtlas::ProcessTGCreadout(MYSQL& mysql) {
        MsgStream log(m_msgSvc, "MuGM:RDBReadAtlas");

        if (getGeometryVersion().substr(0, 1) == "P") {
            IRDBRecordset_ptr ggsd = m_pRDBAccess->getRecordsetPtr("GGSD", m_geoTag, m_geoNode);
            IRDBRecordset_ptr ggcd = m_pRDBAccess->getRecordsetPtr("GGCD", m_geoTag, m_geoNode);
            log << MSG::INFO << "RDBReaderAtlas::ProcessTGCreadout GGSD, GGCD retrieven from Oracle" << endmsg;

            int version = (int)(*ggsd)[0]->getDouble("VERS");
            float wirespacing = (*ggsd)[0]->getDouble("WIRESP") * Gaudi::Units::cm;
            log << MSG::INFO << " ProcessTGCreadout - version " << version << " wirespacing " << wirespacing << endmsg;

            //
            // in case of the layout P03
            //

            // loop over the banks of station components: ALMN
            for (unsigned int ich = 0; ich < ggcd->size(); ++ich) {
                int type = (int)(*ggcd)[ich]->getDouble("ICHTYP");

                if (ich < 19) {
                    std::string name = RDBReaderAtlas::TGCreadoutName(type);

                    int nchrng = (int)(*ggcd)[ich]->getDouble("NCHRNG");
                    std::vector<float> nwgs, roffst, poffst, nsps;
                    std::vector<float> iwgs1(180), iwgs2(180), iwgs3(180);

                    for (int i = 0; i < 3; i++) {
                        std::string A("_");
                        A+= std::to_string(i);
                        nwgs.push_back((*ggcd)[ich]->getDouble("NWGS" + A));
                        roffst.push_back((*ggcd)[ich]->getDouble("ROFFST" + A));
                        poffst.push_back((*ggcd)[ich]->getDouble("POFFST" + A));
                        nsps.push_back((*ggcd)[ich]->getDouble("NSPS" + A));
                    }

                    for (int i = 0; i < nwgs[0]; i++) {
                        std::string A("_");
                        A+= std::to_string(i);
                        // float xxx = (*ggcd)[ich]->getDouble("IWGS1"+A);
                        iwgs1[i] = (float)(*ggcd)[ich]->getDouble("IWGS1" + A);
                    }

                    for (int i = 0; i < nwgs[1]; i++) {
                        std::string A("_");
                        A+= std::to_string(i);
                        iwgs2[i] = (float)(*ggcd)[ich]->getDouble("IWGS2" + A);
                    }

                    for (int i = 0; i < nwgs[2]; i++) {
                        std::string A("_");
                        A+= std::to_string(i);
                        iwgs3[i] = (float)(*ggcd)[ich]->getDouble("IWGS3" + A);
                    }

                    auto rpar = std::make_unique<TgcReadoutParams>(name, type, version, wirespacing, nchrng, &(nwgs[0]), &(iwgs1[0]), &iwgs2[0], &iwgs3[0], &roffst[0], &nsps[0],
                                                                   &poffst[0]);
                    mysql.StoreTgcRPars(rpar.get());
                    m_mgr->storeTgcReadoutParams(std::move(rpar));
                }
            }
        } else {
            //
            // in case of layout Q and following
            //
            AmdcDb *theAmdcDb = dynamic_cast<AmdcDb *>(m_pRDBAccess);
            IRDBRecordset_ptr ggln = theAmdcDb ? theAmdcDb->getRecordsetPtr("GGLN", "Amdc") : m_pRDBAccess->getRecordsetPtr("GGLN", m_geoTag, m_geoNode);

            int version(0);
            float wirespacing(0);
            unsigned int gglnSize(0);
            if (ggln)
                gglnSize = ggln->size();
            else {
                log << MSG::WARNING << " ProcessTGCreadout - IRDBRecordset_ptr GGLN is nullptr" << endmsg;
            }
            if (gglnSize) {
                version = (int)(*ggln)[0]->getInt("VERS");
                wirespacing = (*ggln)[0]->getFloat("WIRESP") * Gaudi::Units::mm;
            }

            log << MSG::INFO << " ProcessTGCreadout - version " << version << " wirespacing " << wirespacing << endmsg;

            // loop over the banks of station components: ALMN
            for (unsigned int ich = 0; ich < gglnSize; ++ich) {
                int type = (int)(*ggln)[ich]->getInt("JSTA");
                std::string name = "TGCReadout" + MuonGM::buildString(type, 2);

                // NCHRNG missing in GGLN, HARD-CODED !!!
                int nchrng;
                if (type == 1 || type == 6 || type == 12 || type >= 18) {
                    nchrng = 24;
                } else {
                    nchrng = 48;
                }
                std::vector<float> nwgs, roffst, poffst, nsps;
                std::vector<float> iwgs1(130), iwgs2(130), iwgs3(130), slarge(33), sshort(33);

                for (int i = 0; i < 3; i++) {
                    std::string A("_");
                    A+= std::to_string(i);
                    nwgs.push_back((*ggln)[ich]->getInt("NWGS" + A));
                    roffst.push_back((*ggln)[ich]->getInt("ROFFST" + A));
                    // poffst.push_back((*ggln)[ich]->getInt("POFFST"+A));
                    poffst.push_back(0);
                    nsps.push_back((*ggln)[ich]->getInt("NSPS" + A));
                }

                for (int i = 0; i < nwgs[0]; i++) {
                    std::string A("_");
                    A+= std::to_string(i);
                    // float xxx = (*ggln)[ich]->getInt("IWGS1"+A);
                    iwgs1[i] = (float)(*ggln)[ich]->getInt("IWGS1" + A);
                }

                for (int i = 0; i < nwgs[1]; i++) {
                    std::string A("_");
                    A+= std::to_string(i);
                    iwgs2[i] = (float)(*ggln)[ich]->getInt("IWGS2" + A);
                }

                for (int i = 0; i < nwgs[2]; i++) {
                    std::string A("_");
                    A+= std::to_string(i);
                    iwgs3[i] = (float)(*ggln)[ich]->getInt("IWGS3" + A);
                }

                // read and store parameters for strips
                float pdist = (*ggln)[ich]->getFloat("PDIST");

                for (int i = 0; i < nsps[0] + 1; i++) {
                    std::string A("_");
                    A+= std::to_string(i);
                    slarge[i] = (float)(*ggln)[ich]->getFloat("SLARGE" + A);
                    sshort[i] = (float)(*ggln)[ich]->getFloat("SHORT" + A);
                }

                auto rpar = std::make_unique<TgcReadoutParams>(name, type, version, wirespacing, nchrng, &(nwgs[0]), &(iwgs1[0]), &iwgs2[0], &iwgs3[0], pdist, &slarge[0],
                                                               &sshort[0], &roffst[0], &nsps[0], &poffst[0]);
                mysql.StoreTgcRPars(rpar.get());
                m_mgr->storeTgcReadoutParams(std::move(rpar));

                // parameters for TGC inactive inner structure

                std::ostringstream Astr;
                if (ich < 9) {
                    Astr << "0" << ich + 1;
                } else {
                    Astr << ich + 1;
                }
                std::string A = Astr.str();
                TGC *tgc = dynamic_cast<TGC*>(mysql.GetTechnology("TGC" + A));
                tgc->widthWireSupport = (*ggln)[ich]->getFloat("S1PP");
                tgc->widthGasChannel = (*ggln)[ich]->getFloat("S2PP");
                tgc->distanceWireSupport = (*ggln)[ich]->getFloat("WSEP");
                tgc->offsetWireSupport[0] = (*ggln)[ich]->getFloat("SP1WI");
                tgc->offsetWireSupport[1] = (*ggln)[ich]->getFloat("SP2WI");
                tgc->offsetWireSupport[2] = (*ggln)[ich]->getFloat("SP3WI");
                tgc->angleTilt = (*ggln)[ich]->getFloat("TILT") * Gaudi::Units::deg;
                tgc->radiusButton = (*ggln)[ich]->getFloat("SP1BU");
                tgc->pitchButton[0] = (*ggln)[ich]->getFloat("SP2BU");
                tgc->pitchButton[1] = (*ggln)[ich]->getFloat("SP3BU");
                tgc->angleButton = (*ggln)[ich]->getFloat("SP4BU") * Gaudi::Units::deg;
            }
        }
    }

    std::string RDBReaderAtlas::TGCreadoutName(int ichtyp) {
        MsgStream log(m_msgSvc, "MuGM:RDBReadAtlas:TGCreadoutName");

        if (getGeometryVersion().substr(0, 1) == "P") {

            if (m_tgcReadoutMapping.size() == 0) {
                // first time fill the vector
                m_tgcReadoutMapping.push_back("T1F1"); // 1

                m_tgcReadoutMapping.push_back("T1E1"); // 2
                m_tgcReadoutMapping.push_back("T1E2");
                m_tgcReadoutMapping.push_back("T1E3");
                m_tgcReadoutMapping.push_back("T1E4");

                m_tgcReadoutMapping.push_back("T2F1"); // 6

                m_tgcReadoutMapping.push_back("T2E1"); // 7
                m_tgcReadoutMapping.push_back("T2E2");
                m_tgcReadoutMapping.push_back("T2E3");
                m_tgcReadoutMapping.push_back("T2E4");
                m_tgcReadoutMapping.push_back("T2E5"); // 11

                m_tgcReadoutMapping.push_back("T3F1"); // 12

                m_tgcReadoutMapping.push_back("T3E1"); // 13
                m_tgcReadoutMapping.push_back("T3E2");
                m_tgcReadoutMapping.push_back("T3E3");
                m_tgcReadoutMapping.push_back("T3E4");
                m_tgcReadoutMapping.push_back("T3E5"); // 17

                m_tgcReadoutMapping.push_back("T4F1"); // 18

                m_tgcReadoutMapping.push_back("T4E1"); // 19
            }

            if (ichtyp < 1 || ichtyp > 19) {
                log << MSG::ERROR << " DBReader::TGCreadoutName  - ichtype " << ichtyp << " out of range 1-19" << endmsg;
                return "XXXY";
            }
        } else { // if (getGeometryVersion().substr(0,1) == "Q")

            // Layout Q and following
            //
            if (m_tgcReadoutMapping.size() == 0) {
                // first time fill the vector

                m_tgcReadoutMapping.push_back("T1F1_1"); // 1

                m_tgcReadoutMapping.push_back("T1E1_1"); // 2
                m_tgcReadoutMapping.push_back("T1E1_2");
                m_tgcReadoutMapping.push_back("T1E1_3");
                m_tgcReadoutMapping.push_back("T1E1_4");

                m_tgcReadoutMapping.push_back("T2F1_1"); // 6

                m_tgcReadoutMapping.push_back("T2E1_1"); // 7
                m_tgcReadoutMapping.push_back("T2E1_2");
                m_tgcReadoutMapping.push_back("T2E1_3");
                m_tgcReadoutMapping.push_back("T2E1_4");
                m_tgcReadoutMapping.push_back("T2E1_5"); // 11

                m_tgcReadoutMapping.push_back("T3F1_1"); // 12

                m_tgcReadoutMapping.push_back("T3E1_1"); // 13
                m_tgcReadoutMapping.push_back("T3E1_2");
                m_tgcReadoutMapping.push_back("T3E1_3");
                m_tgcReadoutMapping.push_back("T3E1_4");
                m_tgcReadoutMapping.push_back("T3E1_5"); // 17

                m_tgcReadoutMapping.push_back("T4F1_1"); // 18
                m_tgcReadoutMapping.push_back("T4F2_1"); // 19

                m_tgcReadoutMapping.push_back("T4E1_1"); // 20
                m_tgcReadoutMapping.push_back("T4E2_1"); // 21
            }

            if (ichtyp < 1 || ichtyp > 21) {
                log << MSG::ERROR << " DBReader::TGCreadoutName  - ichtype " << ichtyp << " out of range 1-21" << endmsg;
                return "XXXY";
            }
        }

        return m_tgcReadoutMapping[ichtyp - 1];
    }
} // namespace MuonGM
