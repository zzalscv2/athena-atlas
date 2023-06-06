/*
 Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

/*
 * This tooltakes the tracks divided into those for a first fit, and additional tracks for a second
 * It initializes a jet candidate, which consists of vertices with tracks fitted to them
 * It runs the full JetFitter fitting algorithm, that is the merge cluster and kalman fit
 * It returns the JetFitter output info
 */

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "TrkTrackLink/ITrackLink.h"
#include "TLorentzVector.h"

#include "TrkJetVxFitter/JetFitterRoutines.h"
#include "TrkJetVxFitter/JetFitterHelper.h"
#include "TrkJetVxFitter/JetFitterInitializationHelper.h"
#include "InDetSecVxFinderTool/InDetJetFitterUtils.h"

namespace Trk {
    class VxJetCandidate;
    class RecVertex;
}

namespace InDet {


static const InterfaceID IID_JetFitterMultiStageFit("JetFitterMultiStageFit", 1, 0);

class JetFitterMultiStageFit : public AthAlgTool {
public:

    static const InterfaceID &interfaceID() {
        return IID_JetFitterMultiStageFit;
    }

    virtual StatusCode initialize() override;

    JetFitterMultiStageFit(const std::string &t, const std::string &n, const IInterface *p);

    ~JetFitterMultiStageFit();

    /*
     * Method for performing the two stage fit
     */
    Trk::VxJetCandidate* doTwoStageFit( const Trk::RecVertex & primaryVertex,
                                        const TLorentzVector & jetMomentum,
                                        const std::vector<const Trk::ITrackLink*> & firstInputTracks,
                                        const std::vector<const Trk::ITrackLink*> & secondInputTracks,
                                        const Amg::Vector3D & vtxSeedDirection) const;


private:
    /*
     * Method which runs the merge procedure, at each cycle using the kalman filter
     */
    void doTheFit(Trk::VxJetCandidate* myJetCandidate,
                  bool performClustering=true) const;

    int getIndexByMass(const double mass) const;

    ToolHandle< Trk::JetFitterInitializationHelper > m_initializationHelper {this,"JetFitterInitializationHelper","Trk::JetFitterInitializationHelper",""};
    ToolHandle< Trk::JetFitterHelper > m_helper {this,"JetFitterHelper","Trk::JetFitterHelper",""};
    ToolHandle< Trk::JetFitterRoutines > m_routines {this,"JetFitterRoutines","Trk::JetFitterRoutines",""};
    ToolHandle< InDet::InDetJetFitterUtils > m_jetFitterUtils {this,"InDetJetFitterUtils","InDet::InDetJetFitterUtils/InDetJetFitterUtils",""};

    Gaudi::Property< int > m_maxNumDeleteIterations {this,"MaxNumDeleteIterations",30,""};
    Gaudi::Property< double > m_vertexProbCut {this,"VertexProbCut",0.001,""};
    Gaudi::Property< int > m_maxClusteringIterations {this,"MaxClusteringIterations",30,""};

    Gaudi::Property< bool > m_useFastClustering {this,"UseFastClustering",false,""};

    // to avoid combinatoric slow down, limit on max tracks for detailed clustering
    Gaudi::Property< int > m_maxTracksForDetailedClustering {this,"maxTracksForDetailedClustering",25,""};

    Gaudi::Property< double > m_vertexClusteringProbabilityCut {this,"VertexClusteringProbabilityCut",0.005,""};

    DoubleArrayProperty m_vertexClusteringProbabilityCutWithMasses
    {this, "VertexClusteringProbabilityCutWithMasses",
	{0.002, 0.002, 0.050, 0.100, 0.200, 0.500, 0.700, 0.900, 0.900}, ""};
    const std::vector<double> m_massBins = {1000., 1500., 2000., 2500.,
					    3000., 4000., 5000., 6000.};

};

}
