/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "./ModAntikTJetMaker.h"

#include <sstream>

std::vector<Gep::Jet>
Gep::ModAntikTJetMaker::makeJets(const std::vector<Gep::Cluster> &clusters) const {

  std::vector<Gep::Cluster> constituents = clusters;

  std::vector<Gep::Jet> v_jets;

  const unsigned int n_constituents = constituents.size();
  std::vector< std::vector<int> > v_constitutents_in_jet_indices(n_constituents, std::vector<int>() );
  // initialize vector of constituents with it's own index
  for(unsigned int i = 0; i < n_constituents; i++){ v_constitutents_in_jet_indices.at(i).push_back(i); }

  int niter = 0;

  //Iterate over clusters
  while(!constituents.empty() && niter < m_nIter){

    const unsigned int n_clusters = constituents.size();

    //create factorized separation list
    std::vector< std::vector< float > > DeltaR2(n_clusters, std::vector< float >(n_clusters, 0.0));
    std::vector< float > InvPt2 (n_clusters, 0.0);
    for (unsigned int i = 0; i < n_clusters; i++){
      InvPt2[i] = 1/constituents.at(i).vec.Perp2();
      DeltaR2[i][i] = 1.0;
      for(unsigned int j = i+1; j < n_clusters; j++){
        float deltaEta = constituents.at(i).vec.Eta() - constituents.at(j).vec.Eta();
        float deltaPhi = constituents.at(i).vec.DeltaPhi(constituents.at(j).vec);
        DeltaR2[i][j] = (deltaEta*deltaEta + deltaPhi*deltaPhi)/(m_jetR*m_jetR);
	DeltaR2[j][i] = (deltaEta*deltaEta + deltaPhi*deltaPhi)/(m_jetR*m_jetR);
      }
    }
    

    // indices of minimum DeltaR in i-th row (aka j in d_ij)
    std::vector< int > minDeltaR2_indices(n_clusters, 0.0);
    for (unsigned int i = 0; i < n_clusters; i++ ) {
      minDeltaR2_indices[i] = std::distance( DeltaR2[i].begin(), std::min_element(std::begin(DeltaR2[i]), std::end(DeltaR2[i])) );
    }

    //d_ij vector of with minDeltaR2_indices_ij
    std::vector< float > InvPt2DeltaR2(n_clusters, 0.0);
    for (unsigned int i = 0; i < n_clusters; i++ ) InvPt2DeltaR2[i] = InvPt2[i]*DeltaR2[i][minDeltaR2_indices[i]];

    // index of minimum d_ij (aka i in d_ij)
    int minInvPt2DeltaR2_index = std::distance(InvPt2DeltaR2.begin() , std::min_element(std::begin(InvPt2DeltaR2), std::end(InvPt2DeltaR2)) );

    // if d_ij < diB, merge j into i, otherwise i is jet
    if (InvPt2DeltaR2[minInvPt2DeltaR2_index] < InvPt2[minInvPt2DeltaR2_index]){
      //This is the WTA pt merging scheme
      // only momentum components are modified
      TVector3 momentum_vector(constituents.at(minInvPt2DeltaR2_index).vec.Px() + constituents.at(minDeltaR2_indices[minInvPt2DeltaR2_index]).vec.Px(),
			       constituents.at(minInvPt2DeltaR2_index).vec.Py() + constituents.at(minDeltaR2_indices[minInvPt2DeltaR2_index]).vec.Py(),
			       constituents.at(minInvPt2DeltaR2_index).vec.Pz() + constituents.at(minDeltaR2_indices[minInvPt2DeltaR2_index]).vec.Pz());

      //px, py and pz are recalculated from pt, eta and phi, leaving eta and phi unchanged 
      constituents.at(minInvPt2DeltaR2_index).vec.SetPtEtaPhiE(momentum_vector.Pt(), 
							 constituents.at(minInvPt2DeltaR2_index).vec.Eta(), 
							 constituents.at(minInvPt2DeltaR2_index).vec.Phi(), 
							 momentum_vector.Mag() );  

      // add indices in j to lilst of constituents of subjet i
      v_constitutents_in_jet_indices.at(minInvPt2DeltaR2_index).insert( v_constitutents_in_jet_indices.at(minInvPt2DeltaR2_index).end(),
									v_constitutents_in_jet_indices.at(minDeltaR2_indices[minInvPt2DeltaR2_index]).begin(),
									v_constitutents_in_jet_indices.at(minDeltaR2_indices[minInvPt2DeltaR2_index]).end() );

      constituents.erase(constituents.begin() + minDeltaR2_indices[minInvPt2DeltaR2_index]);
      v_constitutents_in_jet_indices.erase(v_constitutents_in_jet_indices.begin() + minDeltaR2_indices[minInvPt2DeltaR2_index]);
    } else {

      //create custom jet based on constituents.at(minInvPt2DeltaR2_index)
      Gep::Jet jet;
      jet.vec.SetXYZT(constituents.at(minInvPt2DeltaR2_index).vec.Px(), constituents.at(minInvPt2DeltaR2_index).vec.Py(),
		    constituents.at(minInvPt2DeltaR2_index).vec.Pz(), constituents.at(minInvPt2DeltaR2_index).vec.E());
      jet.constituentsIndices.insert( jet.constituentsIndices.end(), 
				       v_constitutents_in_jet_indices.at( minInvPt2DeltaR2_index).begin(), 
				       v_constitutents_in_jet_indices.at( minInvPt2DeltaR2_index).end());

      v_jets.push_back(jet);

      constituents.erase(constituents.begin() + minInvPt2DeltaR2_index);
      v_constitutents_in_jet_indices.erase(v_constitutents_in_jet_indices.begin() + minDeltaR2_indices[minInvPt2DeltaR2_index]);

    }

    niter++;
  }

  return   v_jets;
}


std::string Gep::ModAntikTJetMaker::toString() const {
  
  std::stringstream ss;
  ss << "ModAntikTJetMaker. iterations: " << m_nIter
     << " rad: " << m_jetR;
  return ss.str();
}
