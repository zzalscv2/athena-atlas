/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// csc_cluster_performance.cxx

// David Adams
// June, 2006
//
// Main program to loop over entries in a simpos tree and find the matching
// entries in a cluster tree.
//
// The classes SimposAccessor and ClusterAccessor used to access the trees
// are generated byt root. If changes are made, run this program with option
// -g and copy the headers to the include directory CscClusterPerformance.

#include <cstdlib>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <cmath>

#include "TTree.h"
#include "TTreeIndex.h"
#include "SimposAccessor.h"
#include "ClusterAccessor.h"

using std::string;
using std::map;
using std::cout;
using std::cerr;
using std::endl;
using std::istringstream;

// To build a exe that only generates the ROOTT interface classes.
#undef  GENERATE_ONLY

typedef std::vector<int> EntryList;
typedef std::vector<std::string> NameList;

namespace {

// Class to hold event number.
class Runevt {
  int m_run;
  int m_evt;
public:
  Runevt(int run, int evt) : m_run(run), m_evt(evt) { }
  int run() const { return m_run; }
  int evt() const { return m_evt; }
  int operator<(const Runevt& rhs) const {
    if ( run() == rhs.run() ) return evt() < rhs.evt();
    return run() < rhs.run();
  }
  int operator==(const Runevt& rhs) const {
    return run() == rhs.run() && evt() == rhs.evt();
  }
};

std::ostream& operator<<(std::ostream& str, const Runevt& rhs) {
  str << rhs.run() << ":" << rhs.evt();
  return str;
}

}  // end unnamed namespace

int main(int narg, char* argv[]) {
  bool help = false;
  bool generate = false;
  int verbose = 1;
  int error = 0;
  int ndump = 0;
  string arg1;
  string arg2;
  // Read option flags.
  int iarg = 0;
  while ( ++iarg<narg && argv[iarg][0] == '-' ) {
    string opt = argv[iarg] + 1;
    if ( opt == "h" ) {
      help = true;
    } else if ( opt == "g" ) {
      generate = true;
    } else if ( opt == "d" ) {
      string sdump = argv[++iarg];
      istringstream ssdump(sdump);
      ssdump >> ndump;
    } else {
      cerr << "Uknown option: -" << opt << endl;
      error = 3;
    }
  }
  // Read the file names.
  if ( !error && !help ) {
    if ( iarg < narg ) {
      arg1 = argv[iarg++];
      if ( iarg < narg ) {
        arg2 = argv[iarg++];
      } else {
        cerr << "Second file name not found" << endl;
        error = 2;
      }
    } else {
      cout << "First file name not found" << endl;
      error = 1;
    }
  }

  if ( help ) {
    cout << "Usage: " << argv[0] << " [-g] simpos_file cluster_file" << endl;
    return error;
  }

  if ( error ) return error;

  // Open simpos file.
  TFile* psfile = new TFile(arg1.c_str(), "READ");
  TTree* pstree = dynamic_cast<TTree*>(psfile->Get("csc_simpos"));
  if ( pstree == nullptr ) {
    cout << "Unable to retrieve simpos tree" << endl;
    cerr << "  File: " << arg1 << endl;
    psfile->Print();
    return 3;
  }
  cout << "Simpos tree has " << pstree->GetEntries() << " entries." << endl;
  if ( generate ) {
    cout << "Generating class SimposAccessor" << endl;
    pstree->MakeClass("SimposAccessor");
  } 

  // Open cluster file.
  TFile* pcfile = new TFile(arg2.c_str(), "READ");
  TTree* pctree = dynamic_cast<TTree*>(pcfile->Get("csc_cluster"));
  if ( pctree == nullptr ) {
    cout << "Unable to retrieve cluster tree" << endl;
    cerr << "  File: " << arg2 << endl;
    pcfile->Print();
    return 4;
  }
  cout << "Cluster tree has " << pctree->GetEntries() << " entries." << endl;
  if ( generate ) {
    cout << "Generating class ClusterAccessor" << endl;
    pctree->MakeClass("ClusterAccessor");
    return 0;
  } 

#ifndef GENERATE_ONLY
#define MAXENT 200
#define MAXSTRIP 10
#define MAXPOS 10
#define MAXEXTRA 20

  // Create result tree.
  int run;
  int evt;
  int nentry;                // Number of entries for each event.
  float yh[MAXENT];          // Hit position in local coordinates.
  float zh[MAXENT];
  float th[MAXENT];
  float eta[MAXENT];         // Production eta
  float pt[MAXENT];          // Production pT
  float yc[MAXENT];          // Cluster coordinates. 
  float zc[MAXENT];
  float dyc[MAXENT];         // Cluster coordinate errors.
  float dzc[MAXENT];
  float tyc[MAXENT];         // Cluster time.
  float tzc[MAXENT];
  int syc[MAXENT];           // Cluster status (see CscClusterStatus).
  int szc[MAXENT];
  int wyc[MAXENT];           // Cluster coordinate full widths (strip counts).
  int wzc[MAXENT];
  float yrefit[MAXENT][MAXPOS];      // Cluster refit hit position.
  float zrefit[MAXENT][MAXPOS];
  float dyrefit[MAXENT][MAXPOS];     // Cluster refit hit position error.
  float dzrefit[MAXENT][MAXPOS];
  int syrefit[MAXENT][MAXPOS];       // Cluster refit status (see CscClusterStatus).
  int szrefit[MAXENT][MAXPOS];
  float qyc[MAXENT][MAXSTRIP];   // Strip charge distributions
  float qzc[MAXENT][MAXSTRIP];
  float qpy[MAXENT];         // Charge on the peak channel.
  float qpz[MAXENT];
  float qly[MAXENT];         // Charge on the peak channel.
  float qlz[MAXENT];
  float qry[MAXENT];         // Charge on the channel right of the peak.
  float qrz[MAXENT];
  float nyc[MAXENT];         // Number of strips in each cathode plane.
  float nzc[MAXENT];
  float pyc[MAXENT];         // Strip pitches.
  float pzc[MAXENT];
  int oyc[MAXENT];           // First strip channel numbers.
  int ozc[MAXENT];
  int myc[MAXENT];           // Cluster channel with maximum charge.
  int mzc[MAXENT];
  float ryc[MAXENT];         // RMS of the charge position.
  float rzc[MAXENT];
  int zsec[MAXENT];          // Z-sector (-1, +1)
  int istation[MAXENT];      // station (1=CSS, 2=CSL)
  int phisec[MAXENT];        // phi-sector (1-8)
  int sector[MAXENT];        // zsec*(2*phisec-istation+1);
  int wlay[MAXENT];          // wire layer (1-4)
//  float exdata[MAXEXTRA][MAXENT];
  NameList exnames;
  exnames.push_back("scor");
  exnames.push_back("dscor");
  exnames.push_back("scor1");
  exnames.push_back("dscor1");
  exnames.push_back("scor2");
  exnames.push_back("dscor2");
  exnames.push_back("scordiff");
  exnames.push_back("dscordiff");
  exnames.push_back("junk");
  if ( exnames.size() > MAXEXTRA ) {
    std::cerr << "Too many extra names." << std::endl;
  }
  TFile* pfile = new TFile("csc_perf.root", "RECREATE");
  TTree* ptree = new TTree("csc_perf", "CSC performance");
  ptree->Branch("run",     &run,      "run/I");
  ptree->Branch("evt",     &evt,      "evt/I");
  ptree->Branch("nentry",  &nentry,   "nentry/I");
  ptree->Branch("yh",       yh,       "yh[nentry]");
  ptree->Branch("zh",       zh,       "zh[nentry]");
  ptree->Branch("th",       th,       "th[nentry]");
  ptree->Branch("eta",      eta,      "eta[nentry]");
  ptree->Branch("pt",       pt,       "pt[nentry]");
  ptree->Branch("yc",       yc,       "yc[nentry]");
  ptree->Branch("zc",       zc,       "zc[nentry]");
  ptree->Branch("dyc",      dyc,      "dyc[nentry]");
  ptree->Branch("dzc",      dzc,      "dzc[nentry]");
  ptree->Branch("tyc",      tyc,      "tyc[nentry]");
  ptree->Branch("tzc",      tzc,      "tzc[nentry]");
  ptree->Branch("syc",      syc,      "syc[nentry]/I");
  ptree->Branch("szc",      szc,      "szc[nentry]/I");
  ptree->Branch("wyc",      wyc,      "wyc[nentry]/I");
  ptree->Branch("wzc",      wzc,      "wzc[nentry]/I");
  ptree->Branch("yrefit",   yrefit,   "yrefit[nentry][10]");
  ptree->Branch("zrefit",   zrefit,   "zrefit[nentry][10]");
  ptree->Branch("dyrefit",  dyrefit,  "dyrefit[nentry][10]");
  ptree->Branch("dzrefit",  dzrefit,  "dzrefit[nentry][10]");
  ptree->Branch("syrefit",  syrefit,  "syrefit[nentry][10]/I");
  ptree->Branch("szrefit",  szrefit,  "szrefit[nentry][10]/I");
  ptree->Branch("qyc",      qyc,      "qyc[nentry][10]");
  ptree->Branch("qzc",      qzc,      "qzc[nentry][10]");
  ptree->Branch("qpy",      qpy,      "qpy[nentry]");
  ptree->Branch("qpz",      qpz,      "qpz[nentry]");
  ptree->Branch("qly",      qly,      "qly[nentry]");
  ptree->Branch("qlz",      qlz,      "qlz[nentry]");
  ptree->Branch("qry",      qry,      "qry[nentry]");
  ptree->Branch("qrz",      qrz,      "qrz[nentry]");
  ptree->Branch("nyc",      nyc,      "nyc[nentry]");
  ptree->Branch("nzc",      nzc,      "nzc[nentry]");
  ptree->Branch("pyc",      pyc,      "pyc[nentry]");
  ptree->Branch("pzc",      pzc,      "pzc[nentry]");
  ptree->Branch("oyc",      oyc,      "oyc[nentry]/I");
  ptree->Branch("ozc",      ozc,      "ozc[nentry]/I");
  ptree->Branch("myc",      myc,      "myc[nentry]/I");
  ptree->Branch("mzc",      mzc,      "mzc[nentry]/I");
  ptree->Branch("ryc",      ryc,      "ryc[nentry]");
  ptree->Branch("rzc",      rzc,      "rzc[nentry]");
  ptree->Branch("zsec",     zsec,     "zsec[nentry]/I");
  ptree->Branch("istation", istation, "istation[nentry]/I");
  ptree->Branch("phisec",   phisec,   "phisec[nentry]/I");
  ptree->Branch("wlay",     wlay,     "wlay[nentry]/I");
  ptree->Branch("sector",   sector,   "sector[nentry]/I");
  //  for ( unsigned int iex=0; iex<exnames.size(); ++iex ) {
  //    string name = exnames[iex];
  //    string desc = name + "[nentry]";
  //    ptree->Branch(name.c_str(), exdata[iex], desc.c_str());
  //  }

  SimposAccessor simpos(pstree);
  ClusterAccessor cluster(pctree);

  // The following line causes dlopen libCint.so error.
  // Commented out Aug 8, 2008
  //pctree->BuildIndex("run", "evt");
  //TTreeIndex cluster_index(pctree, "run", "evt");
  
  // Build index for cluster tree.
  typedef map<Runevt, int> EvtIndex;
  EvtIndex cluster_index;
  for ( int isevt=0; isevt<pstree->GetEntries(); ++isevt ) {
    cluster.GetEntry(isevt);
    Runevt re(cluster.run, cluster.evt);
    cluster_index[re] = isevt;
    cout << "Index: " << re << " " << isevt << endl;
  }

  // 1: Using Default cluspos
  // 0: Using Best cluspos
  cout << "====================================" << endl;
  bool standardComparison =1;
  if (standardComparison)
    cout << "Clusters from the standard CscThresholdBuilder "  << endl;
  else
    cout << "Clusters from the standard + CscSplitCluster " << endl;
  cout << "====================================" << endl;
    
  // Overall statistics.
  int ntot_hit = 0;
  int ntot_rclu_matched = 0;
  int ntot_pclu_matched = 0;
  // Loop over events.
  for ( int isevt=0; isevt<pstree->GetEntries(); ++isevt ) {
    simpos.GetEntry(isevt);
    run = simpos.run;
    evt = simpos.evt;
    Runevt re(run, evt);
    cout << "Processing run:event " << re << endl;
    int nhit = simpos.nentry;
    // Fetch the clusters for this event.
    EvtIndex::const_iterator iicevt = cluster_index.find(re);
    if ( iicevt == cluster_index.end() ) {
      cout << "  Event not found in cluster tree!" << endl;
      continue;
    }
    int icevt = iicevt->second;
    cluster.GetEntry(icevt);
    // Initialize event loop variables.
    int ient = 0;
    // Loop over hits.
    for ( int ihit=0; ihit<nhit; ++ihit ) {
      if ( ient >= MAXENT ) {
        cout << "Too many entries!!!" << endl;
        abort();
      }
      ++ntot_hit;
      // Initialize variables for this hit.
      int icl_rmatch = -1;       // Entry of the matching r-cluster
      int icl_pmatch = -1;       // Entry of the matching phi-cluster.
      double pres = 999999.9;    // Phi residual.
      double rres = 999999.9;    // R residual.
      yc[ient] = 999999.9;
      zc[ient] = 999999.9;
      dyc[ient] = 0.0;
      dzc[ient] = 0.0;
      tyc[ient] = 0.0;
      tzc[ient] = 0.0;
      syc[ient] = -1;
      szc[ient] = -1;
      wyc[ient] = 0;
      wzc[ient] = 0;
      nyc[ient] = 0.0;
      nzc[ient] = 0.0;
      pyc[ient] = 0.0;
      pzc[ient] = 0.0;
      oyc[ient] = 0;
      ozc[ient] = 0;
      myc[ient] = 0;
      mzc[ient] = 0;
      ryc[ient] = 0.0;
      rzc[ient] = 0.0;
      for ( int i=0; i<MAXSTRIP; ++i ) {
        qyc[ient][i] = 0.0;
        qzc[ient][i] = 0.0;
      }
      for ( int i=0; i<MAXPOS; ++i ) {
        zrefit[ient][i] = 0.0;
        dzrefit[ient][i] = 0.0;
        szrefit[ient][i] = 0;
        yrefit[ient][i] = 0.0;
        dzrefit[ient][i] = 0.0;
        syrefit[ient][i] = 0;
      }
      qpy[ient] = 0.0;
      qpz[ient] = 0.0;
      qly[ient] = 0.0;
      qlz[ient] = 0.0;
      qry[ient] = 0.0;
      qrz[ient] = 0.0;
      // Extract hit parameters.
      istation[ient] = simpos.istation[ihit];
      zsec[ient] = simpos.zsec[ihit];
      phisec[ient] = simpos.phisec[ihit];
      sector[ient] = simpos.sector[ihit];//zsec[ient]*(2*phisec[ient]-istation[ient]+1);
      wlay[ient] = simpos.wlay[ihit];
      double hity = simpos.y[ihit];
      double hitz = simpos.z[ihit];
      double hitt = simpos.time[ihit];
      if ( verbose > 1 ) {
        cout << "Hit position: " << hity << ", " << hitz << endl;
        cout << "Cluster multiplicity: " << cluster.nentry << endl;
      }
      // Loop over clusters.
      double ref_measphi =100000;
      double ref_measeta =100000;
      for ( int iclu=0; iclu<cluster.nentry; ++iclu ) {
        // Require wire plane match.
        if ( verbose > 2 ) {
          string sep = ":";
          cout << "  Cluster/hit planes: "
               << cluster.zsec[iclu] << sep << cluster.istation[iclu] << sep
               << cluster.phisec[iclu] << sep << cluster.wlay[iclu] << "/"
               << zsec[ient] << sep << istation[ient] << sep << phisec[ient] << sep << wlay[ient] << endl;
        }
        if ( cluster.zsec[iclu] == zsec[ient] &&
             cluster.istation[iclu] == istation[ient] &&
             cluster.phisec[iclu] == phisec[ient] &&
             cluster.wlay[iclu] == wlay[ient] ) {
          // Does cluster overlap hit?
          // We lose hits outside the chamber acceptance.
          // And (someday) on dead strips?
          bool measphi = cluster.measphi[iclu];
          double pitch = cluster.pitch[iclu];
          int nstrip = cluster.nstrip[iclu];
          double hitpos = measphi ? hity : hitz;
          double halfwidth = 0.5*pitch*nstrip;

          double clupos =cluster.pos[iclu];
          if ( verbose > 1 ) {
            cout << "  Cluster/hit position: " << clupos << "/" << hitpos << endl;
          }
          double cludpos =0.;
          int clusfit = 255;

          if (!measphi && !standardComparison) {
            float dist = 100000;
            for (unsigned int ii=0; ii<10; ii++) {
              double meas_pos = cluster.posrefit[iclu][ii];
              if (meas_pos == 0.0) continue;
              
              cout << "dist:meas_pos  " << dist << ":" << meas_pos << " " << endl;
              if (fabs(hitpos-meas_pos)<dist) {
                dist = fabs(hitpos-meas_pos);
                clupos = meas_pos;
                cout << "  --dist:meas_pos  " << dist << ":" << clupos << " " <<endl;
                
                cludpos = cluster.errrefit[iclu][ii];
                clusfit = cluster.srefit[iclu][ii];
              }
            }
          }
          
          // If overlaps...
          if ( hitpos > clupos - halfwidth  &&
             hitpos < clupos + halfwidth ) {
            // Phi measurement: Fill output tree variables.
            if (measphi) {
              pres = clupos - hitpos;
              if (ref_measphi > fabs(pres)) {
                ref_measphi = fabs(pres);

                icl_pmatch = iclu;
                yc[ient] = cluster.pos[iclu];
                dyc[ient] = cluster.error[iclu];
                //                tyc[ient] = cluster.tfit[iclu];
                tyc[ient] = cluster.tpeak[iclu];
                wyc[ient] = cluster.nstrip[iclu];
                syc[ient] = cluster.sfit[iclu];
                for (int ii=0; ii<10; ii++) {
                  syrefit[ient][ii] = cluster.srefit[iclu][ii];
                  yrefit[ient][ii] = cluster.posrefit[iclu][ii];
                  dyrefit[ient][ii] = cluster.errrefit[iclu][ii];
                }
                double sum_q = 0;
                double sum_qs = 0;
                double sum_qss = 0;
                double qmax = 0.0;
                for ( int i=0; i<wyc[ient]; ++i ) {
                  double q = cluster.qstrip[iclu][i];
                  if ( i < MAXSTRIP ) qyc[ient][i] = q;
                  sum_q += q;
                  sum_qs += i*q;
                  sum_qss += i*i*q;
                  if ( q > qmax ) {
                    myc[ient] = i;
                    qmax = q;
                  }
                }
                qpy[ient] = cluster.qpeak[iclu];
                qly[ient] = cluster.qleft[iclu];
                qry[ient] = cluster.qright[iclu];
                ryc[ient] = sqrt(sum_qss/sum_q - sum_qs*sum_qs/sum_q/sum_q);
                nyc[ient] = cluster.maxstrip[iclu];
                pyc[ient] = cluster.pitch[iclu];
                oyc[ient] = int(cluster.strip0[iclu] + 0.5);
                //              if ( icl_rmatch >= 0 ) break;
                // R measurement: Fill output tree variables.
              }
            } else {
              //              if ( icl_rmatch >= 0 ) {
              //                cerr << "Second r match!!!" << endl;
              //                double oldhw = 0.5*pzc[ient]*wzc[ient];
              //                cerr << "  zsec=" << zsec << " station=" << istation
              //                     << " phisec=" << phisec << " wlay=" << wlay << endl;
              //                cerr << "  Old: zc=" << zc[ient] << " wzc=" << wzc[ient]
              //                     << " min=" << zc[ient]-oldhw << " max=" << zc[ient]+oldhw << endl;
              //                cerr << "  New: zc=" << cluster.pos[iclu] << " wzc=" << cluster.nstrip[iclu]
              //                     << " min=" << clupos - halfwidth << " max=" << clupos + halfwidth << endl;
              //                continue;
              //              }
              rres = clupos - hitpos;
              if (ref_measeta > fabs(rres)) {
                ref_measeta = fabs(rres);
                icl_rmatch = iclu;
                if (standardComparison) {
                  zc[ient] = cluster.pos[iclu];
                  dzc[ient] = cluster.error[iclu];
                  szc[ient] = cluster.sfit[iclu];
                } else {
                  zc[ient] = clupos;
                  dzc[ient] = cludpos;
                  szc[ient] = cluster.sfit[iclu];
                  if (clusfit==20)
                    szc[ient] = 0;
                }
                //                tzc[ient] = cluster.tfit[iclu];
                tzc[ient] = cluster.tpeak[iclu];
                wzc[ient] = cluster.nstrip[iclu];
                cout << "run=" << run << " evt=" << evt << endl;
                cout << hitpos << " <> " << cluster.pos[iclu] << " :: " ;
                for (int i=0; i<10; i++) {
                  szrefit[ient][i] = cluster.srefit[iclu][i];
                  zrefit[ient][i] = cluster.posrefit[iclu][i];
                  dzrefit[ient][i] = cluster.errrefit[iclu][i];
                  cout << cluster.posrefit[iclu][i] << "  ";
                }
                cout << zc[ient] << endl;
                double sum_q = 0;
                double sum_qs = 0;
                double sum_qss = 0;
                double qmax = 0.0;
                for ( int i=0; i<wzc[ient]; ++i ) {
                  double q = cluster.qstrip[iclu][i];
                  if ( i< MAXSTRIP ) qzc[ient][i] = q;
                  sum_q += q;
                  sum_qs += i*q;
                  sum_qss += i*i*q;
                  if ( q > qmax ) {
                    mzc[ient] = i;
                    qmax = q;
                  }
                }
                qpz[ient] = cluster.qpeak[iclu];
                qlz[ient] = cluster.qleft[iclu];
                qrz[ient] = cluster.qright[iclu];
                rzc[ient] = sqrt(sum_qss/sum_q - sum_qs*sum_qs/sum_q/sum_q);
                nzc[ient] = cluster.maxstrip[iclu];
                pzc[ient] = cluster.pitch[iclu];
                ozc[ient] = int(cluster.strip0[iclu] + 0.5);
/*                for ( unsigned int iex=0; iex<exnames.size(); ++iex ) {
                  string name = exnames[iex];
                  if ( name == "scor" ) exdata[iex][ient] = cluster.scor[iclu];
                  else if ( name == "dscor" ) exdata[iex][ient] = cluster.dscor[iclu];
                  else if ( name == "scor1" ) exdata[iex][ient] = cluster.scor1[iclu];
                  else if ( name == "dscor1" ) exdata[iex][ient] = cluster.dscor1[iclu];
                  else if ( name == "scor2" ) exdata[iex][ient] = cluster.scor2[iclu];
                  else if ( name == "dscor2" ) exdata[iex][ient] = cluster.dscor2[iclu];
                  else if ( name == "scordiff" ) exdata[iex][ient] = cluster.scordiff[iclu];
                  else if ( name == "dscordiff" ) exdata[iex][ient] = cluster.dscordiff[iclu];
                  else exdata[iex][ient] = -444.0;
                } */
                //              if ( icl_pmatch > 0 ) break;
              }
            }
          }  // end if cluster/hit overlap
        }  // end if wire plane match.
      }  // end loop over clusters
      if ( verbose ) {
        if ( icl_rmatch >= 0 ) {
          ++ntot_rclu_matched;
          cout << "    Eta residual is " << rres << endl;
        } else {
          cout << "    Eta cluster not found!" << endl;
        }
        if ( icl_pmatch >= 0 ) {
          ++ntot_pclu_matched;
          cout << "    Phi residual is " << pres << endl;
        } else {
          cout << "    Phi cluster not found!" << endl;
        }
      }
      yh[ient] = hity;
      zh[ient] = hitz;
      th[ient] = hitt;
      eta[ient] = simpos.eta[ihit];
      pt[ient] = simpos.pt[ihit];
      ++ient;
    }  // end loop over hits
    // Fill tree.
    nentry = ient;
    ptree->Fill();
  }  // end loop over events

  ptree->Print();
  cout << "R-efficiency = " << ntot_rclu_matched << "/" << ntot_hit << " = ";
  if( ntot_hit != 0 ) cout << double(ntot_rclu_matched)/double(ntot_hit) << endl;
  else cout << "nan" << endl;
  cout << "Phi-efficiency = " << ntot_pclu_matched << "/" << ntot_hit << " = ";
  if( ntot_hit != 0 ) cout << double(ntot_pclu_matched)/double(ntot_hit) << endl;
  else cout << "nan" << endl;

  pfile->Write();
#endif
  cout << "Done." << endl;
  return 0;
}

// Build the skeleton functions.
#define SimposAccessor_cxx
#include "SimposAccessor.h"
void SimposAccessor::Loop() { }
#define ClusterAccessor_cxx
#include "ClusterAccessor.h"
void ClusterAccessor::Loop() { }
