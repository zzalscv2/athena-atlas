/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/***************************************************************************
                           SVTag.cxx
***************************************************************************/
#include "JetTagTools/SVTag.h"

#include "JetTagTools/NewLikelihoodTool.h"
#include "GaudiKernel/ITHistSvc.h"
#include "JetTagTools/HistoHelperRoot.h"
#include "JetTagTools/LikelihoodComponents.h"
#include "ParticleJetTools/JetFlavourInfo.h"

#include "xAODTracking/VertexContainer.h"

#include <string>

#include "GeoPrimitives/GeoPrimitivesHelpers.h"

namespace Analysis
{

  SVTag::SVTag(const std::string& t, const std::string& n, const IInterface* p)
    : base_class(t,n,p),
      m_likelihoodTool("Analysis::NewLikelihoodTool", this),
      m_histoHelper(0),
      m_secVxFinderName("SV1"),
      m_isFlipped(false)
  {
    declareProperty("Runmodus",       m_runModus= "reference");
    declareProperty("referenceType",  m_refType = "ALL");
    declareProperty("jetPtMinRef",    m_pTjetmin = 15.*Gaudi::Units::GeV);
    declareProperty("LikelihoodTool", m_likelihoodTool);
    declareProperty("checkOverflows", m_checkOverflows = false);
    declareProperty("purificationDeltaR", m_purificationDeltaR = 0.8);

    declareProperty("jetCollectionList", m_jetCollectionList);
    declareProperty("useForcedCalibration",  m_doForcedCalib   = false);
    declareProperty("ForcedCalibrationName", m_ForcedCalibName = "Cone4H1Tower");

    declareProperty("SecVxFinderName",m_secVxFinderName);
    declareProperty("SVAlgType",      m_SVmode);
    declareProperty("xAODBaseName",      m_xAODBaseName);

    declareProperty("UseDRJetPvSv", m_useDRJPVSV = true);
    declareProperty("UseCHypo", m_useCHypo=true);
    declareProperty("UseSV2Pt", m_usePtSV2=false);
    declareProperty("SaveProbabilities", m_save_probabilities=false);

  }

  SVTag::~SVTag() {
    delete m_histoHelper;
  }

  StatusCode SVTag::initialize() {
    printParameterSettings();

    /** number of hypotheses = 3 : b | u | c **/
    m_hypotheses.push_back("B");
    m_hypotheses.push_back("U");
    if(m_useCHypo){
      m_hypotheses.push_back("C");
    }

    /** retrieving histoHelper: */
    ITHistSvc* myHistoSvc;
    if( service( "THistSvc", myHistoSvc ).isSuccess() ) {
      ATH_MSG_DEBUG("#BTAG#" << name() << ": HistoSvc loaded successfully.");
      m_histoHelper = new HistoHelperRoot(myHistoSvc);
      m_histoHelper->setCheckOverflows(m_checkOverflows);
    } else ATH_MSG_ERROR("#BTAG#" << name() << ": HistoSvc could NOT bo loaded.");

    /** configure likelihood: */
    if( m_runModus == "analysis" && m_SVmode != "SV0" && m_save_probabilities) {
      if ( m_likelihoodTool.retrieve().isFailure() ) {
        ATH_MSG_FATAL("#BTAG# Failed to retrieve tool " << m_likelihoodTool);
        return StatusCode::FAILURE;
      } else {
        ATH_MSG_INFO("#BTAG# Retrieved tool " << m_likelihoodTool);
      }
      m_likelihoodTool->defineHypotheses(m_hypotheses);
      m_likelihoodTool->printStatus();
    }


    /* ----------------------------------------------------------------------------------- */
    /*                         BOOK HISTOS IF IN REFERENCE MODE                            */
    /* ----------------------------------------------------------------------------------- */
    if (m_runModus=="reference" && m_SVmode!="SV0") {
      //
      // Book the histos
      //
      ATH_MSG_VERBOSE("#BTAG# Booking histos...");
      std::vector<double> xb;
      double xbi[10] = {1., 2., 3., 4., 5., 6., 8., 10., 20., 50.};
      for(const auto& jetColl : m_jetCollectionList) {
        for(uint ih=0;ih<m_hypotheses.size();ih++) {
          // SV1
          if (m_SVmode == "SV1") {
            std::string hDir = "/RefFile/SV1/"+jetColl+"/"+m_hypotheses[ih]+"/";
            m_histoHelper->bookHisto(hDir+"N2T", "Number of Good Two Track Vertices",9,xbi);
            m_histoHelper->bookHisto(hDir+"N2TEffSV1", "Number of Good Two Track Vertices",9,xbi);
            m_histoHelper->bookHisto(hDir+"N2TNormSV1", "Number of Good Two Track Vertices",30,0.,30.);
            m_histoHelper->bookHisto(hDir+"BidimME", "(E fraction)**0.7 vs Mass/(Mass+1)" ,50,0.218261406,1.,50,0.,1.);
            m_histoHelper->bookHisto(hDir+"DRJPVSV", "DeltaR between jet axis and (PV,SV) axis",100,0.,0.5);
          } else if (m_SVmode == "SV2") {
            std::string hDir = "/RefFile/SV2/"+jetColl+"/"+m_hypotheses[ih]+"/";
            // SV2
            m_histoHelper->bookHisto(hDir+"N2TEffSV2", "Number of Good Two Track Vertices",9,xbi);
            m_histoHelper->bookHisto(hDir+"N2TNormSV2", "Number of Good Two Track Vertices",30,0.,30.);
            if(m_usePtSV2)m_histoHelper->bookHisto(hDir+"TridimMENPt","ln(Pt) vs (E fraction)**0.7 vs Mass/(Mass+1)",20,0.,1.,20,0.,1.,6,0.,4.8);
            else          m_histoHelper->bookHisto(hDir+"TridimMEN2T"," ln(N) vs (E fraction)**0.7 vs Mass/(Mass+1)",20,0.,1.,20,0.,1.,7,0.,3.8);
            if(ih==0) {
              // Control with SV2
              hDir = "/RefFile/SV2/"+jetColl+"/controlSV/";
              m_histoHelper->bookHisto(hDir+"eta","eta",60,-3.,3.);
              m_histoHelper->bookHisto(hDir+"phi","phi",64,-3.2,3.2);
              m_histoHelper->bookHisto(hDir+"pt","pt",50,0.,300.);
            }
          }
        }
      }
      m_histoHelper->print();
    }

    // Check if this instance of tagger is a flipped one:
    std::string iname( this->name().substr(8) );
    std::string::size_type pos = iname.find("Flip");
    if ( pos!=std::string::npos ) {
      m_isFlipped = true;
      ATH_MSG_INFO("#BTAG# This instance of tagger has a flipped configuration. DRJPVSV computation will be adjusted.");
    }

    //Conversion to GeV
    m_c_mom   = 1000.;
    //
    m_expos = 0.7;

    m_nbjet = 0;
    m_ncjet = 0;
    m_nljet = 0;
    return StatusCode::SUCCESS;
  }


  StatusCode SVTag::finalize()
  {
    if( m_runModus == "reference" ) {
      ATH_MSG_INFO("#BTAG# Number of jets used for calibration for reference "
     << m_refType << " : #b= " << m_nbjet << " #light= " << m_nljet  << " #charm= " << m_ncjet
    );
    }
    return StatusCode::SUCCESS;
  }

  StatusCode SVTag::tagJet(const xAOD::Vertex& priVtx,
                           const xAOD::Jet& jetToTag,
                           xAOD::BTagging& BTag,
                           const std::string &jetName) const
  {

    /** author to know which jet algorithm: */
    std::string author = jetName;
    if (m_doForcedCalib) author = m_ForcedCalibName;
    ATH_MSG_VERBOSE("#BTAG# Using jet type " << author << " for calibrations.");

    /* The jet */
    Amg::Vector3D jetDir(jetToTag.p4().Px(),jetToTag.p4().Py(),jetToTag.p4().Pz());
    double jeteta = jetToTag.eta(), jetphi = jetToTag.phi(), jetpt = jetToTag.pt();
    ATH_MSG_VERBOSE("#BTAG# Jet properties : eta = " << jeteta
                    << " phi = " << jetphi << " pT  = " <<jetpt/m_c_mom);

    // Fill control histograms
    if (m_runModus=="reference" && m_SVmode == "SV2") {
      if (std::abs(jeteta) <= 2.5) {
        m_histoHelper->fillHisto("/RefFile/SV2/"+author+"/controlSV/eta",(double)jeteta);
        m_histoHelper->fillHisto("/RefFile/SV2/"+author+"/controlSV/phi",(double)jetphi);
        m_histoHelper->fillHisto("/RefFile/SV2/"+author+"/controlSV/pt",(double)jetpt/m_c_mom);
      }
    }
    //
    // Get the SV info
    //
    float ambtot = -1., xratio = -1., distnrm = 0., drJPVSV = 0., Lxy = -100., L3d = -100.;
    int NSVPair = -1;
    float distnrmCorr=0.;

    //retrieving the secondary vertices
    bool status = true;
    std::vector< ElementLink< xAOD::VertexContainer > > myVertices;
    // don't check the following status
    BTag.variable<std::vector<ElementLink<xAOD::VertexContainer> > >(m_secVxFinderName, "vertices", myVertices);

    if (!myVertices.empty()) {

      status &= BTag.variable<float>(m_secVxFinderName, "masssvx", ambtot);// mass in MeV
      ambtot/=m_c_mom;
      status &= BTag.variable<float>(m_secVxFinderName, "efracsvx", xratio);
      status &= BTag.variable<int>(m_secVxFinderName, "N2Tpair", NSVPair);

      if (!status) {
        ATH_MSG_WARNING("Error retrieving variables for SV finder name " << m_secVxFinderName << ", result will be incorrect!");
      }

      // DR between Jet axis and PV-SV axis
      // For the time being computed only for Single Vertex...
      if (myVertices[0].isValid()) {
        const xAOD::Vertex* firstVertex = *(myVertices[0]);

        //FIXME ugly hack to get a Amg::Vector3D out of a CLHEP::HepLorentzVector
        const Amg::Vector3D PVposition = priVtx.position();
        const Amg::Vector3D position = firstVertex->position();
        Amg::Vector3D PvSvDir( position.x() - PVposition.x(),
                               position.y() - PVposition.y(),
                               position.z() - PVposition.z() );
        double drJPVSV_1 = Amg::deltaR(jetDir,PvSvDir);
        drJPVSV = drJPVSV_1;
        // for flipped taggers, use -jet direction:
        double drJPVSV_2 = Amg::deltaR(-jetDir, PvSvDir); // for negative tags
        if ( m_isFlipped ) drJPVSV = drJPVSV_2; // for negative tags
        ATH_MSG_VERBOSE("#BTAG# DRJPVSV regular="<<drJPVSV_1<<" flipped="<<drJPVSV_2<<" chosen="<<drJPVSV);

        Lxy = std::hypot(PvSvDir(0,0), PvSvDir(1,0));
        L3d = std::hypot(PvSvDir(0,0), PvSvDir(1,0), PvSvDir(2,0));
      }else{
        ATH_MSG_VERBOSE("#BTAG# No secondary vertex.");
      }

      std::vector<const xAOD::Vertex*> vecVertices;
      for (const auto& link : myVertices) {
	if (!link.isValid()) {
	  ATH_MSG_WARNING("#BTAG# Secondary vertex from InDetVKalVxInJetFinder has zero pointer. Skipping... ");
          continue;
	}
	vecVertices.push_back(*link);
      }

      if (myVertices[0].isValid()) {
        const xAOD::Vertex* myVert  = *myVertices[0];
        distnrm = get3DSignificance(priVtx, vecVertices, jetDir);
        ATH_MSG_VERBOSE("#BTAG# SVX x = " << myVert->position().x() << " y = " << myVert->position().y() << " z = " << myVert->position().z());
        distnrmCorr = get3DSignificanceCorr(priVtx, vecVertices, jetDir);
      } else {
        ATH_MSG_VERBOSE("#BTAG# No vertex. Cannot calculate normalized distance.");
        distnrm=0.;
      }

      ATH_MSG_VERBOSE("#BTAG# The SVX prop. sign3d: " << distnrm);
      ATH_MSG_VERBOSE("#BTAG# Svx Mass = "<< ambtot << " Svx E frac = " << xratio << " NGood2TrackVertices = " << NSVPair);
    } else {
      //keepInfoPlus = false;
      ATH_MSG_VERBOSE("#BTAG# No SVx !");
    }


    /* Give information to the info class. */
    // define Info name based on tagger instance name
    // (remove ToolSvc. in front and Tag inside name):
    std::string iname(name().substr(8));
    std::string instanceName(iname);
    std::string::size_type pos = iname.find("Tag");
    if (pos != std::string::npos) {
      std::string prefix = iname.substr(0,pos);
      std::string posfix = iname.substr(pos+3);
      instanceName = prefix;
      instanceName += posfix;
    }

    //AA: Move to filling xAOD::BTagging
    // note that this block is filling things other than likelihood,
    // it should not be conditional on m_save_probabilities
    if (m_runModus=="analysis") {
      // -- fill extended info class ...

      if (m_xAODBaseName == "SV0") // just to be clear, specify enum explicitely
      {
        BTag.setTaggerInfo(distnrm, xAOD::BTagInfo::SV0_normdist);
        BTag.setSV0_significance3D(distnrm);
      }
      else if (m_xAODBaseName == "SV1")
      {
        BTag.setTaggerInfo(distnrmCorr, xAOD::BTagInfo::SV1_normdist);
        BTag.setVariable<float>(m_xAODBaseName, "significance3d", distnrm);
        BTag.setVariable<float>(m_xAODBaseName, "correctSignificance3d", distnrmCorr);
        BTag.setVariable<float>(m_xAODBaseName, "deltaR", drJPVSV);
        BTag.setVariable<float>(m_xAODBaseName, "Lxy", Lxy);
        BTag.setVariable<float>(m_xAODBaseName, "L3d", L3d);
      }
      else{
        BTag.setVariable<float>(m_xAODBaseName, "normdist", distnrm);
        if (m_xAODBaseName.find("SV1")!=std::string::npos) {
          BTag.setVariable<float>(m_xAODBaseName, "significance3d", distnrm);
          BTag.setVariable<float>(m_xAODBaseName, "correctSignificance3d", distnrmCorr);
          BTag.setVariable<float>(m_xAODBaseName, "deltaR", drJPVSV);
          BTag.setVariable<float>(m_xAODBaseName, "Lxy", Lxy);
          BTag.setVariable<float>(m_xAODBaseName, "L3d", L3d);
        }
      }

    } // end "analysis" mode block


    /* For SV1 & SV2, compute the weight (analysis) or fill histograms (reference) */
    ATH_MSG_VERBOSE("#BTAG# SV mode = " << m_SVmode);

    if (m_SVmode != "SV0" ) {
      float ambtotp = ambtot > 0. ? ambtot/(1.+ambtot) : 0.;
      float xratiop = xratio > 0. ? (float)pow(xratio,m_expos) : 0.;
      float trfJetPt = log(jetToTag.pt()/20000.);
      if(trfJetPt<0.) trfJetPt=0.01;
      if(trfJetPt>4.8) trfJetPt=4.79;
      std::string pref = "";
      if (m_runModus=="reference") {
        if (jetpt >= m_pTjetmin && std::abs(jeteta) <= 2.5) {
          int label = xAOD::jetFlavourLabel(&jetToTag);
          double deltaRtoClosestB = 999.;//, deltaRtoClosestC = 999.;
          if (jetToTag.getAttribute("TruthLabelDeltaR_B",deltaRtoClosestB)) {
            ATH_MSG_VERBOSE("#BTAG# label found : " << label);
            // for purification: require no b or c quark closer than dR=m_purificationDeltaR
            double deltaRtoClosestC;
            jetToTag.getAttribute("TruthLabelDeltaR_C", deltaRtoClosestC);//mcTrueInfo->deltaRMinTo("C");
            double deltaRmin = deltaRtoClosestB < deltaRtoClosestC ? deltaRtoClosestB : deltaRtoClosestC;

            if ( (    "B"==m_refType &&   5==label ) ||  // b-jets
                 ( "UDSG"==m_refType &&   0==label ) ||  // light jets
                 (  "ALL"==m_refType && // all jets: b + purified light jets
                    ( 5==label || 4==label || ( 0==label && deltaRmin > m_purificationDeltaR ) ) )
              ) {
              if (5==label) {
                pref = m_hypotheses[0];
                m_nbjet++;
              } else if (0==label) {
                pref = m_hypotheses[1];
                m_nljet++;
              } else if (4==label && m_useCHypo) {
                pref = m_hypotheses[2];
                m_ncjet++;
              }
            }

            if (pref == "B" || pref == "C" || pref == "U") {
              std::string hDir = "/RefFile/"+m_SVmode+"/"+author+"/"+pref+"/";
              if (m_SVmode == "SV1") m_histoHelper->fillHisto(hDir+"N2TNormSV1",(float)NSVPair);
              if (m_SVmode == "SV2") m_histoHelper->fillHisto(hDir+"N2TNormSV2",(float)NSVPair);
              if (NSVPair > 0 && ambtot > 0.) {
                if (xratiop == 1.) xratiop = 0.999999;  //This is not an overflow...
                if (m_SVmode == "SV1") {
                  m_histoHelper->fillHisto(hDir+"N2T",(float)NSVPair);
                  m_histoHelper->fillHisto(hDir+"N2TEffSV1",(float)NSVPair);
                  m_histoHelper->fillHisto(hDir+"BidimME",ambtotp,xratiop);
                  m_histoHelper->fillHisto(hDir+"DRJPVSV",(float)drJPVSV);
                }
                if (m_SVmode == "SV2") {
                  m_histoHelper->fillHisto(hDir+"N2TEffSV2",(float)NSVPair);
                  if(m_usePtSV2) m_histoHelper->fillHisto(hDir+"TridimMENPt",ambtotp,xratiop,trfJetPt);
                  else           m_histoHelper->fillHisto(hDir+"TridimMEN2T",ambtotp,xratiop,log((float)NSVPair));
                }
              }
            }
          } else {
            ATH_MSG_ERROR("#BTAG# No TruthInfo ! Cannot run in reference mode !");
            return StatusCode::FAILURE;
          }
        }
      } else if (m_runModus=="analysis" && m_save_probabilities) {
        std::vector<double> probi;
        // access efficiencies:
        double effb = m_likelihoodTool->getEff(m_hypotheses[0], author+"#N2T", m_SVmode);
        double effu = m_likelihoodTool->getEff(m_hypotheses[1], author+"#N2T", m_SVmode);
        double effc = 1.e9;
        if(m_useCHypo){
          effc = m_likelihoodTool->getEff(m_hypotheses[2], author+"#N2T", m_SVmode);
        }
        ATH_MSG_DEBUG( "#BTAG#  EFF b,u,c= " << effb << " " << effu << " " << effc);
        if (NSVPair>0 && ambtot > 0.) {
          std::vector<Slice> nslices;
          AtomicProperty atom2(ambtotp,"SecVtx Transformed Mass");
          AtomicProperty atom3(xratiop,"SecVtx Transformed Energy Fraction");
          if (m_SVmode == "SV1") {
            AtomicProperty atom1(NSVPair,"Number of Two Track Vertices");
            std::string compoName(author+"#");
            Composite compo1(compoName+"N2T");
            Composite compo2(compoName+"BidimME");
            compo1.atoms.push_back(atom1);
            compo2.atoms.push_back(atom2);
            compo2.atoms.push_back(atom3);
            Slice slice1("SV1");
            slice1.composites.push_back(compo1);
            slice1.composites.push_back(compo2);
            if(m_useDRJPVSV) {
              AtomicProperty atom4(drJPVSV,"DeltaR between jet axis and (PV,SV) axis");
              Composite compo3(compoName+"DRJPVSV");
              compo3.atoms.push_back(atom4);
              slice1.composites.push_back(compo3);
            }
            nslices.push_back(slice1);
          } else if (m_SVmode == "SV2") {
            std::string compoName(author+"#");
            Slice slice1("SV2");
            if(m_usePtSV2){
              AtomicProperty atom1(trfJetPt,"log(JetPt/2e4)");
              Composite compo(compoName+"TridimMENPt");
              compo.atoms.push_back(atom2);
              compo.atoms.push_back(atom3);
              compo.atoms.push_back(atom1);
              Composite compo1(compoName+"N2TEffSV2");
              AtomicProperty atom4(NSVPair,"Number of Two Track Vertices");
              compo1.atoms.push_back(atom4);
              slice1.composites.push_back(compo);
              slice1.composites.push_back(compo1);
            }else{
              AtomicProperty atom1(log((float)NSVPair),"log(Number of Two Track Vertices)");
              Composite compo(compoName+"TridimMEN2T");
              compo.atoms.push_back(atom2);
              compo.atoms.push_back(atom3);
              compo.atoms.push_back(atom1);
              slice1.composites.push_back(compo);
            }
            nslices.push_back(slice1);
          }
          probi = m_likelihoodTool->calculateLikelihood(nslices);
          ATH_MSG_DEBUG( "#BTAG#  WEIGHT: pb, pu, pc= "
                         << probi[0] << " " << probi[1] << " " << probi[2]);
          if (probi.size() >= 2) {
            probi[0] *= effb;
            probi[1] *= effu;
            if(m_useCHypo){
              probi[2] *= effc;
            }
          } else {
            ATH_MSG_ERROR("#BTAG# Missing number in jet probabilities ! "<<probi.size());
          }
        } else {
          // The SV weight is computed even if there is only one or no track !
          // It may seem a little bit weird...
          probi.push_back((1.-effb));
          probi.push_back((1.-effu));
          if(m_useCHypo){
            probi.push_back((1.-effc));
          }
        }
        if (probi.size()>=2){
          BTag.setVariable<float>(m_xAODBaseName, "pb", probi[0]);
          BTag.setVariable<float>(m_xAODBaseName, "pu", probi[1]);
        }
        if (m_useCHypo and (probi.size()>=3)) BTag.setVariable<float>(m_xAODBaseName, "pc", probi[2]);

      } // end analysis mode

      /* For SV0, put the signed 3D Lxy significance: */
    } else {
      ATH_MSG_VERBOSE("#BTAG# SV0 Lxy3D significance = " << distnrm);
      BTag.setVariable<float>(m_xAODBaseName, "pb", exp(distnrm));
      BTag.setVariable<float>(m_xAODBaseName, "pu", 1);
      if (m_useCHypo) BTag.setVariable<float>(m_xAODBaseName, "pc", 1);
    }

    ATH_MSG_VERBOSE("#BTAG# SVTag Finalizing... ");

    return StatusCode::SUCCESS;
  }

  void SVTag::printParameterSettings() {
    ATH_MSG_INFO("#BTAG# " << name() << "Parameter settings ");
    ATH_MSG_INFO("#BTAG# I am in " << m_runModus << " modus.");
    ATH_MSG_INFO("#BTAG# The method is " << m_SVmode);
    if (m_runModus == "reference") ATH_MSG_INFO("#BTAG# Preparing "<< m_refType<< "-jet probability density functions...");
  }

  double SVTag::get3DSignificance(const xAOD::Vertex& priVertex,
          std::vector<const xAOD::Vertex*>& secVertex,
          const Amg::Vector3D jetDirection) const {

    std::vector<Amg::Vector3D> positions;
    std::vector<AmgSymMatrix(3)> weightMatrices;
    // If multiple secondary vertices were reconstructed, then a common (weighted) position will be used
    // in the signed decay length significance calculation
    Amg::Vector3D weightTimesPosition(0.,0.,0.);
    AmgSymMatrix(3) sumWeights;
    sumWeights.setZero();

    for (const auto& vtx : secVertex) {
      positions.push_back(vtx->position());
      weightMatrices.push_back(vtx->covariancePosition().inverse());
      weightTimesPosition += weightMatrices.back()*positions.back();
      sumWeights += weightMatrices.back();
    }

    // now we have the sum of the weights, let's invert this matrix to get the mean covariance matrix
    bool invertible;
    AmgSymMatrix(3) meanCovariance;
    meanCovariance.setZero();
    sumWeights.computeInverseWithCheck(meanCovariance, invertible);
    if (!invertible) {
       ATH_MSG_WARNING("#BTAG# Could not invert sum of sec vtx matrices");
    return 0.;
    }

    // calculate the weighted mean secondary vertex position
    Amg::Vector3D meanPosition = meanCovariance*weightTimesPosition;

    // add the mean covariance matrix of the secondary vertices to that of the primary vertex
    // this is the covariance matrix for the decay length
    AmgSymMatrix(3) covariance = meanCovariance + priVertex.covariancePosition();

    // ********
    // Calculate the signed decay length significance
    // ********

    double Lx = meanPosition[0]-priVertex.position().x();
    double Ly = meanPosition[1]-priVertex.position().y();
    double Lz = meanPosition[2]-priVertex.position().z();

    const double decaylength = sqrt(Lx*Lx + Ly*Ly + Lz*Lz);
    if(decaylength==0.) return 0.;  //Safety
    const double inv_decaylength = 1. / decaylength;

    double dLdLx = Lx * inv_decaylength;
    double dLdLy = Ly * inv_decaylength;
    double dLdLz = Lz * inv_decaylength;
    double decaylength_err2 = (dLdLx*dLdLx*covariance(0,0) +
			       dLdLy*dLdLy*covariance(1,1) +
			       dLdLz*dLdLz*covariance(2,2) +
			       2.*dLdLx*dLdLy*covariance(0,1) +
			       2.*dLdLx*dLdLz*covariance(0,2) +
			       2.*dLdLy*dLdLz*covariance(1,2));
    if(decaylength_err2<=0.) return 0.;  //Something is wrong
    double decaylength_err = sqrt(decaylength_err2);

    double decaylength_significance = 0.;
    if (decaylength_err != 0.) decaylength_significance = decaylength/decaylength_err;

    // get sign from projection on jet axis
    double L_proj_jetDir = jetDirection.x()*Lx + jetDirection.y()*Ly + jetDirection.z()*Lz;
    if (L_proj_jetDir < 0.) decaylength_significance *= -1.;

    return decaylength_significance;
  }

  double SVTag::get3DSignificanceCorr(const xAOD::Vertex& priVertex,
          std::vector<const xAOD::Vertex*>& secVertex,
          const Amg::Vector3D jetDirection) const {

    std::vector<double> Sig3D(0);
    bool success=true;
    AmgSymMatrix(3) Wgt;

    for (const auto & svrt : secVertex)
      {
         Amg::Vector3D SVmPV = svrt->position()-priVertex.position();
         AmgSymMatrix(3) SVmPVCov = svrt->covariancePosition()+priVertex.covariancePosition();
         SVmPVCov.computeInverseWithCheck(Wgt, success);
         if( !success || Wgt(0,0)<=0. || Wgt(1,1)<=0. || Wgt(2,2)<=0. )continue;     //Inversion failure
         double significance = SVmPV.transpose()*Wgt*SVmPV;
         if(significance <= 0.) continue;                          //Something is still wrong!
         significance = std::sqrt(significance);
         if(SVmPV.dot(jetDirection)<0.) significance *= -1.;
         Sig3D.push_back(significance);
      }

    if(Sig3D.size()==0) return 0.;

    return *std::max_element(Sig3D.begin(),Sig3D.end());
  }

}

