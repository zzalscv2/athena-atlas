//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#include "BasicConstantGPUDataExporter.h"

#include "CaloRecGPU/CUDAFriendlyClasses.h"

#include "CxxUtils/checker_macros.h"

#include "AthenaKernel/errorcheck.h"
#include "CaloIdentifier/CaloCell_ID.h"
#include "StoreGate/DataHandle.h"
#include "CaloConditions/CaloNoise.h"

#include "boost/chrono/chrono.hpp"
#include "boost/chrono/thread_clock.hpp"

#include <algorithm>

using namespace CaloRecGPU;

BasicConstantGPUDataExporter::BasicConstantGPUDataExporter(const std::string & type, const std::string & name, const IInterface * parent):
  AthAlgTool(type, name, parent),
  CaloGPUTimed(this),
  m_hasBeenInitialized(false)
{
  declareInterface<ICaloClusterGPUConstantTransformer> (this);
}

StatusCode BasicConstantGPUDataExporter::initialize()
{
  if (m_hasBeenInitialized)
    {
      ATH_MSG_INFO("Initializing data tool again...");
      return StatusCode::SUCCESS;
    }

  ATH_CHECK(m_noiseCDOKey.initialize());

  ATH_CHECK(m_caloMgrKey.initialize());

  m_hasBeenInitialized = true;

  return StatusCode::SUCCESS;
}

StatusCode BasicConstantGPUDataExporter::convert(ConstantDataHolder &, const bool) const
{
  ATH_MSG_ERROR("BasicConstantGPUDataExporter (" << this->name() << ") must be used with the "
                "GPU data preparation happening on the first event.");

  return StatusCode::FAILURE;
}

StatusCode BasicConstantGPUDataExporter::convert(const EventContext & ctx, ConstantDataHolder & cd, const bool keep_CPU_info) const
{
  using clock_type = boost::chrono::thread_clock;
  auto time_cast = [](const auto & before, const auto & after)
  {
    return boost::chrono::duration_cast<boost::chrono::microseconds>(after - before).count();
  };

  auto start = clock_type::now();

  SG::ReadCondHandle<CaloDetDescrManager> caloMgrHandle{m_caloMgrKey, ctx};
  const CaloDetDescrManager * calo_dd_man = *caloMgrHandle;

  cd.m_geometry.allocate();

  const CaloCell_ID * calo_id = calo_dd_man->getCaloCell_ID();

  auto neigh_option_exclude = [&](const CaloCell_ID::SUBCALO sub_detector, const Identifier cell_id,
                                  const bool restrict_HECIWandFCal = true, const bool restrict_PS = true) -> bool
  {
    const bool doRestrictHECIWandFCal = restrict_HECIWandFCal;
    const bool doRestrictPS = restrict_PS;
    if (doRestrictHECIWandFCal && sub_detector != CaloCell_ID::LAREM)
      {
        if (sub_detector == CaloCell_ID::LARHEC && calo_id->region(cell_id) == 1)
          {
            return true;
          }
        if (sub_detector == CaloCell_ID::LARFCAL && calo_id->sampling(cell_id) > 1)
          {
            return true;
          }
      }
    if (doRestrictPS && sub_detector == CaloCell_ID::LAREM && calo_id->sampling(cell_id) == 0)
      {
        return true;
      }
    return false;
  };

  float min_eta_pos[NumSamplings], max_eta_pos[NumSamplings],
        min_eta_neg[NumSamplings], max_eta_neg[NumSamplings],
        min_phi_pos[NumSamplings], max_phi_pos[NumSamplings],
        min_phi_neg[NumSamplings], max_phi_neg[NumSamplings],
        min_deta   [NumSamplings], min_dphi   [NumSamplings];

  for (int i = 0; i < NumSamplings; ++i)
    {
      min_eta_pos[i] = std::numeric_limits<float>::max();
      max_eta_pos[i] = std::numeric_limits<float>::lowest();
      min_eta_neg[i] = std::numeric_limits<float>::max();
      max_eta_neg[i] = std::numeric_limits<float>::lowest();
      min_phi_pos[i] = std::numeric_limits<float>::max();
      max_phi_pos[i] = std::numeric_limits<float>::lowest();
      min_phi_neg[i] = std::numeric_limits<float>::max();
      max_phi_neg[i] = std::numeric_limits<float>::lowest();
      min_deta   [i] = std::numeric_limits<float>::max();
      min_dphi   [i] = std::numeric_limits<float>::max();
    }

  for (int cell = 0; cell < NCaloCells; ++cell)
    {
      const CaloDetDescrElement * caloElement = calo_dd_man->get_element((IdentifierHash) cell);

      const int sampling = calo_id->calo_sample(calo_id->cell_id((IdentifierHash) cell));

      cd.m_geometry->caloSample[cell] = sampling;
      cd.m_geometry->x[cell] = caloElement->x();
      cd.m_geometry->y[cell] = caloElement->y();
      cd.m_geometry->z[cell] = caloElement->z();
      cd.m_geometry->r[cell] = caloElement->r();
      cd.m_geometry->eta[cell] = caloElement->eta();
      cd.m_geometry->phi[cell] = caloElement->phi();

      cd.m_geometry->dx[cell] = caloElement->dx();
      cd.m_geometry->dy[cell] = caloElement->dy();
      cd.m_geometry->dz[cell] = caloElement->dz();
      cd.m_geometry->dr[cell] = caloElement->dr();
      cd.m_geometry->deta[cell] = caloElement->deta();
      cd.m_geometry->dphi[cell] = caloElement->dphi();

      cd.m_geometry->volume[cell] = caloElement->volume();
      cd.m_geometry->neighbours.total_number[cell] = 0;
      cd.m_geometry->neighbours.offsets[cell] = 0;

      if (caloElement->eta() >= 0)
        {
          min_eta_pos[sampling] = std::min(min_eta_pos[sampling], caloElement->eta() - caloElement->deta() / 2);
          min_phi_pos[sampling] = std::min(min_phi_pos[sampling], caloElement->phi() - caloElement->dphi() / 2);
          max_eta_pos[sampling] = std::max(max_eta_pos[sampling], caloElement->eta() + caloElement->deta() / 2);
          max_phi_pos[sampling] = std::max(max_phi_pos[sampling], caloElement->phi() + caloElement->dphi() / 2);
        }
      else
        {
          min_eta_neg[sampling] = std::min(min_eta_neg[sampling], caloElement->eta() - caloElement->deta() / 2);
          min_phi_neg[sampling] = std::min(min_phi_neg[sampling], caloElement->phi() - caloElement->dphi() / 2);
          max_eta_neg[sampling] = std::max(max_eta_neg[sampling], caloElement->eta() + caloElement->deta() / 2);
          max_phi_neg[sampling] = std::max(max_phi_neg[sampling], caloElement->phi() + caloElement->dphi() / 2);
        }
          min_deta[sampling] = std::min(min_deta[sampling], caloElement->deta());
          min_dphi[sampling] = std::min(min_dphi[sampling], caloElement->dphi());

    }

  for (int i = 0; i < NumSamplings; ++i)
    {
      constexpr float corrective_factor = 0.99f;
      cd.m_geometry->etaPhiToCell.initialize(i, min_eta_neg[i], min_phi_neg[i], max_eta_neg[i], max_phi_neg[i],
                                             min_eta_pos[i], min_phi_pos[i], max_eta_pos[i], max_phi_pos[i],
                                             min_deta[i]*corrective_factor, min_dphi[i]*corrective_factor);
    }

  cd.m_geometry->fill_eta_phi_map();

  std::vector<IdentifierHash> neighbour_vector, full_neighs, prev_neighs;

  for (int neigh_bit_set = 0; neigh_bit_set < NumNeighOptions; ++neigh_bit_set)
    {
      const unsigned int curr_neigh_opt = (1U << neigh_bit_set);

      for (int cell = 0; cell < NCaloCells; ++cell)
        {
          const CaloDetDescrElement * caloElement = calo_dd_man->get_element((IdentifierHash) cell);
          CaloCell_ID::SUBCALO sub_detector = caloElement->getSubCalo();

          if (neigh_option_exclude(sub_detector, calo_id->cell_id((IdentifierHash) cell), true, false))
            //Restrict HECIW and FCal.
            {
              cd.m_geometry->neighbours.offsets[cell] |= NeighOffsets::limited_HECIWandFCal_bitmask();
            }
          if (neigh_option_exclude(sub_detector, calo_id->cell_id((IdentifierHash) cell), false, true))
            //Restrict PS
            {
              cd.m_geometry->neighbours.offsets[cell] |= NeighOffsets::limited_PS_bitmask();
            }

          if (curr_neigh_opt == LArNeighbours::corners2D || curr_neigh_opt == LArNeighbours::corners3D)
            //Scanning the ATLAS codebase, neighbour handling has special cases
            //for all2D (LarFCAL_Base_ID.cxx, LArMiniFCAL_ID.cxx, Tile_Base_ID.cxx)
            //and all3DwithCorners (Tile_Base_ID.cxx), which include
            //neighbours not returned when just the constituent bits are set separately.
            //As an imperfect workaround, we stuff them in the immediately previous option
            //(corners, for both 2D and 3D).
            {
              if (curr_neigh_opt == LArNeighbours::corners2D)
                {
                  calo_id->get_neighbours((IdentifierHash) cell,
                                          (LArNeighbours::neighbourOption) LArNeighbours::all2D,
                                          full_neighs);
                }
              else /*if (curr_neigh_opt == LArNeighbours::corners3D)*/
                {
                  calo_id->get_neighbours((IdentifierHash) cell,
                                          (LArNeighbours::neighbourOption) LArNeighbours::all3DwithCorners,
                                          prev_neighs);
                  calo_id->get_neighbours((IdentifierHash) cell,
                                          (LArNeighbours::neighbourOption) LArNeighbours::super3D,
                                          neighbour_vector);
                  //We will exclude the cells that could come from later options
                  //(it seems,  from Tile_Base_ID.cxx, all3DwithCorners will give more than super3D),
                  //which... might make some sense, but seems unexpected.


                  std::sort(neighbour_vector.begin(), neighbour_vector.end());
                  std::sort(prev_neighs.begin(), prev_neighs.end());

                  full_neighs.clear();

                  std::set_difference( prev_neighs.begin(), prev_neighs.end(),
                                       neighbour_vector.begin(), neighbour_vector.end(),
                                       std::back_inserter(full_neighs)    );

                }

              prev_neighs.resize(cd.m_geometry->neighbours.total_number[cell]);
              //We want to add just the neighbours that are not part of this.

              for (size_t neigh = 0; neigh < prev_neighs.size(); ++neigh)
                {
                  prev_neighs[neigh] = cd.m_geometry->neighbours.get_neighbour(cell, neigh, false, false);
                }

              std::sort(full_neighs.begin(), full_neighs.end());
              std::sort(prev_neighs.begin(), prev_neighs.end());

              neighbour_vector.clear();

              std::set_difference( full_neighs.begin(), full_neighs.end(),
                                   prev_neighs.begin(), prev_neighs.end(),
                                   std::back_inserter(neighbour_vector)    );
            }
          else
            {
              calo_id->get_neighbours((IdentifierHash) cell, (LArNeighbours::neighbourOption) curr_neigh_opt, neighbour_vector);
            }

          std::sort(neighbour_vector.begin(), neighbour_vector.end());

          const int neighs_start = cd.m_geometry->neighbours.total_number[cell];

          for (size_t neigh_num = 0; neigh_num < neighbour_vector.size() && neighs_start + neigh_num < NMaxNeighbours; ++neigh_num)
            {
              cd.m_geometry->neighbours.set_neighbour(cell, neighs_start + neigh_num, neighbour_vector[neigh_num]);
            }
          cd.m_geometry->neighbours.total_number[cell] += neighbour_vector.size();

          cd.m_geometry->neighbours.offsets[cell] += NeighOffsets::offset_delta(neigh_bit_set) * neighbour_vector.size();

        }
    }

  auto after_geo = clock_type::now();

  SG::ReadCondHandle<CaloNoise> noise_handle(m_noiseCDOKey, ctx);
  CaloNoise * noise_tool ATLAS_THREAD_SAFE = const_cast<CaloNoise *>(*noise_handle);
  //We are committing the sin of const-casting to access larStorage and tileStorage.
  //I think const views into these could reasonably be added to the CaloNoise itself,
  //but I don't want to change unrelated portions of the code and I promise I'll behave.
  //Also suppress non-thread-safe complaints as we'll only be reading...
  //(Furthermore, constant data preparation should only happen in one thread anyway.)

  IdentifierHash t_start, t_end;
  calo_id->calo_cell_hash_range(CaloCell_ID::TILE, t_start, t_end);

  if (t_start != cd.m_geometry->s_tileStart)
    {
      ATH_MSG_WARNING("Tile start (" << t_start << ") differs from assumed constant value (" << cd.m_geometry->s_tileStart << ")!");
    }
  if (t_end != cd.m_geometry->s_tileEnd)
    {
      ATH_MSG_WARNING("Tile end (" << t_end << ") differs from assumed constant value (" << cd.m_geometry->s_tileEnd << ")!");
    }

  cd.m_cell_noise.allocate();

  //Were it not for the fact that the LAr noise array inside
  //the CaloNoise has three gains while the Tile has all four,
  //we might try to do something more memcpy-y...
  //(Still, with hope, it's highly optimized anyway.)

  for (int gain_state = 0; gain_state < CaloRecGPU::NumGainStates; ++gain_state)
    {
      for (int cell = 0; cell < int(t_start); ++cell)
        {
          cd.m_cell_noise->noise[gain_state][cell] = noise_tool->larStorage()[(gain_state > 2 ? 0 : gain_state)][cell];
        }
      for (int cell = t_start; cell < int(t_end); ++cell)
        {
          cd.m_cell_noise->noise[gain_state][cell] = noise_tool->tileStorage()[gain_state][cell - t_start];
        }
      for (int cell = t_end; cell < NCaloCells; ++cell)
        {
          cd.m_cell_noise->noise[gain_state][cell] = 0;
        }
    }

  auto after_noise = clock_type::now();

  cd.sendToGPU(!(m_keepCPUData || keep_CPU_info));


  auto after_send = clock_type::now();

  if (m_measureTimes)
    {
      record_times(ctx.evt(), time_cast(start, after_geo),
                   time_cast(after_geo, after_noise),
                   time_cast(after_noise, after_send)
                  );
    }

  return StatusCode::SUCCESS;

}

StatusCode BasicConstantGPUDataExporter::finalize()
{

  if (m_measureTimes)
    {
      print_times("Geometry Geometry_Correction Noise Transfer_to_GPU", 3 /*4*/);
    }
  return StatusCode::SUCCESS;
}


BasicConstantGPUDataExporter::~BasicConstantGPUDataExporter()
{
  //Nothing!
}