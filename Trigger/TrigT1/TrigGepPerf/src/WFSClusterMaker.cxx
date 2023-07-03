/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "./WFSClusterMaker.h"

std::vector<Gep::Cluster>
Gep::WFSClusterMaker::makeClusters(const pGepCellMap& caloCellsMap) const {

  std::vector<Gep::Cluster> clusters;

  // Loop over all cells
  for (auto const& cell_itr : *caloCellsMap) {

	// Select only seed cells
	if (!isSeedCell(cell_itr.second)) continue;

	// Clustering
        std::vector<Gep::CustomCaloCell> cluster_cells = clusterFromCells(cell_itr.second, caloCellsMap);
	
        Gep::Cluster cluster = getClusterFromListOfCells(cluster_cells);
        clusters.push_back(cluster);

  }

  // Order topo clusters according to their et
  orderClustersInEt(clusters);

  return clusters;
}


bool Gep::WFSClusterMaker::isSeedCell (const Gep::CustomCaloCell& cell) const {

  if (cell.isBadCell()) return false;
  if (fabs(cell.sigma) < m_seed_threshold) return false;
  if (!isInAllowedSampling(cell.sampling, m_allowed_seed_samplings)) return false;

  return true;
}


bool Gep::WFSClusterMaker::isInAllowedSampling(int sampling, std::vector<int> list_of_samplings) const {

  for (unsigned int i = 0; i < list_of_samplings.size(); ++i) {
	if (list_of_samplings[i] == sampling) return true;
  }
  return false;
}


bool Gep::WFSClusterMaker::isNewCell(unsigned int id, std::vector<unsigned int> seenCells) const {

  for (unsigned int i = 0; i < seenCells.size(); ++i) {
        if (id == seenCells[i]) return false;
  }

  return true;
}


std::vector<Gep::CustomCaloCell>
Gep::WFSClusterMaker::clusterFromCells(const Gep::CustomCaloCell& seed,
					  const pGepCellMap& caloCellsMap) const {

  std::vector<Gep::CustomCaloCell> v_clusterCells;

  std::vector<Gep::CustomCaloCell> cellsNextLayer, cellsThisLayer;
  std::vector<unsigned int> seenCells;

  // Fill seed into supporting vectors
  v_clusterCells.push_back(seed);
  cellsNextLayer.push_back(seed);
  seenCells.push_back(seed.id);

  int i_shell = 1;

  while (cellsNextLayer.size() > 0 && i_shell <= m_max_shells) {

        cellsThisLayer = cellsNextLayer;
        cellsNextLayer.clear();
        ++i_shell;

        // Loop over all cells in this shell
        for (unsigned int i_cell = 0; i_cell < cellsThisLayer.size(); ++i_cell) {

                // Go through list of neighbouring cells and check whether they are part of the cluster
                for (unsigned int i_neighbour = 0; i_neighbour < (cellsThisLayer[i_cell]).neighbours.size(); ++i_neighbour) {
			Gep::CustomCaloCell neighbour = caloCellsMap->at((cellsThisLayer[i_cell]).neighbours[i_neighbour]);

                        // reject if bad cell
                        if (neighbour.isBadCell()) continue;

                        // Reject if cell is not above clustering threshold
                        if (fabs(neighbour.sigma) < m_clustering_threshold) continue;

                        // Reject if cell was already considered
                        if (!isNewCell(neighbour.id, seenCells)) continue;

                        // Ignore cells in disallowed samplings
                        if (!isInAllowedSampling(neighbour.sampling, m_allowed_clustering_samplings)) continue;

                        seenCells.push_back(neighbour.id);
                        cellsNextLayer.push_back(neighbour);
                        v_clusterCells.push_back(neighbour);
                }
        }
        cellsThisLayer.clear();
  }

  return v_clusterCells;
}


Gep::Cluster Gep::WFSClusterMaker::getClusterFromListOfCells(std::vector<Gep::CustomCaloCell> cells) const {

  Gep::Cluster cluster;

  std::vector<unsigned int> v_cellIDs;
  double cluster_e = 0.0;
  double etaSum = 0.0;
  double phiSum = 0.0;
  double abs_e = 0.0;
  float weight = 0.0;

  double seed_phi = cells[0].phi;
  for (unsigned int i_cell = 0; i_cell < cells.size(); ++i_cell) {
        cluster_e += cells[i_cell].e;
        abs_e += fabs(cells[i_cell].e);
        v_cellIDs.push_back(cells[i_cell].id);
        etaSum += fabs(cells[i_cell].e) * cells[i_cell].eta;
        phiSum += fabs(cells[i_cell].e) * getDeltaPhi(cells[i_cell].phi, seed_phi);
        if (fabs(cells[i_cell].sigma) > m_seed_threshold) weight += 1.0;
  }

  cluster.ncells = cells.size();
  cluster.time = cells[0].time; // Take time of seed cell
  cluster.cell_id = v_cellIDs;

  double cluster_eta = etaSum / abs_e;
  double cluster_phi = calculateClusterPhi(seed_phi, phiSum / abs_e);
  double cluster_et = (cluster_e * (1.0 / std::cosh(cluster_eta))) / weight;
  cluster.setEtEtaPhi(cluster_et, cluster_eta, cluster_phi);

  return cluster;
}


double Gep::WFSClusterMaker::getDeltaPhi(double phi, double seed_phi) const {
  double delta_phi = fabs(fabs( fabs( phi - seed_phi ) - TMath::Pi() ) - TMath::Pi());
  if (phi < seed_phi) delta_phi *= -1.00;
  // Taking care of the -pi/pi split
  if ((fabs(phi + seed_phi) < TMath::Pi()) && (fabs(phi) + fabs(seed_phi) > 5.0)) delta_phi *= -1.00;
  return delta_phi;
}

double Gep::WFSClusterMaker::calculateClusterPhi(double seed_phi, double delta_phi) const {
  double phi = seed_phi + delta_phi;
  if (phi > TMath::Pi()) phi = -1.0*TMath::Pi() + (phi - TMath::Pi());
  if (phi < (-1.0)*TMath::Pi()) phi = TMath::Pi() + (phi + TMath::Pi());
  return phi;
}


void Gep::WFSClusterMaker::orderClustersInEt(std::vector<Gep::Cluster> &v_clusters) const {

  std::vector<Gep::Cluster> v_ordered;
  for (unsigned int i_cluster = 0; i_cluster < v_clusters.size(); ++i_cluster) {
        float et = v_clusters[i_cluster].et();

        // Fill first cluster
        if (v_ordered.size() == 0) {
                v_ordered.push_back(v_clusters[i_cluster]);
                continue;
        }

        // Find correct position for filling
        for (unsigned int i = 0; i < v_ordered.size(); ++i) {
                if (v_ordered[i].et() < et) {
                        v_ordered.insert(v_ordered.begin()+i, v_clusters[i_cluster]);
                        break;
                }
        }

        // If cluster has smallest et so far, it wasn't filled at all so we need to take care of it
        if (v_ordered.size() != i_cluster+1) v_ordered.push_back(v_clusters[i_cluster]);
  }

  v_clusters = v_ordered;

  return;

}


std::string Gep::WFSClusterMaker::getName() const {
  return "CaloWFS";
}
