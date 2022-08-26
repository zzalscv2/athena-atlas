// Dear emacs, this is -*- c++ -*-
//
// Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
//

#ifndef CALORECGPU_STANDALONEDATAIO_H
#define CALORECGPU_STANDALONEDATAIO_H

#include "Helpers.h"

#include "CUDAFriendlyClasses.h"


#include <fstream>
#include <string>
#include <map>
#include <set>
#include <iomanip>

#include <boost/filesystem.hpp>

struct StandaloneDataIO
{
  enum class ErrorState
  {
    OK = 0, ReadError, WriteError, VersionError
  };

 protected:


  constexpr static int current_version = 4;

  template <class T>
  inline static ErrorState read_from_file(const boost::filesystem::path & file, T * obj_ptr, const size_t offset = 0, const bool can_have_bool_at_start = true)
  {
    std::ifstream in(file.native(), std::ios_base::binary);
    if (boost::filesystem::file_size(file) && can_have_bool_at_start)
      //Some of our output versions had an extra bool at the start.
      {
        in.ignore(sizeof(bool) + offset);
        if (in.fail())
          {
            return ErrorState::ReadError;
          }
      }
    in.read((char *) obj_ptr, sizeof(T));
    if (in.fail())
      {
        return ErrorState::ReadError;
      }
    in.close();

    return ErrorState::OK;
  }

  inline static int guess_version(const boost::filesystem::path & filepath)
  {
    if (filepath.extension() == ".dat")
      {
        return 1;
      }
    else if (filepath.extension() == ".diag")
      {
        return 2;
      }
    else if (filepath.extension() == ".abrv")
      {
        return 3;
      }
    else if (filepath.extension() == ".clusters" || filepath.extension() == ".geo" || filepath.extension() == ".noise" || filepath.extension() == ".cells")
      {
        return 4;
      }
    else
      {
        return -1;
      }
  }

  inline static void report_error(const boost::filesystem::path & file, const std::string & kind, const bool report = false)
  {
    if (report)
      {
        std::cerr << "ERROR: when " << kind << " from '" << file << "'." << std::endl;
      }
  }

 public:

  struct ConstantInformation
  {
   protected:
    constexpr static int NMaxNeighbours = 26;
    constexpr static int NCaloCells = 187652;

    struct GeometryArr_v1
    {
      int    caloSample[NCaloCells];
      float  x[NCaloCells];
      float  y[NCaloCells];
      float  z[NCaloCells];
      float  eta[NCaloCells];
      float  phi[NCaloCells];
      float  noise[NCaloCells];
      unsigned int nNeighbours[NCaloCells];
      unsigned int neighbours[NCaloCells][NMaxNeighbours];
    };

   public:

    inline static ErrorState read_geometry(const boost::filesystem::path & file, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> & geo, const int version)
    {
      if (version <= 0 || version > current_version)
        {
          return ErrorState::VersionError;
        }
      else if (version < 4)
        {
          CaloRecGPU::Helpers::CPU_object<GeometryArr_v1> temp(true);
          if (read_from_file<GeometryArr_v1>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading geometry");
              return ErrorState::ReadError;
            }
          geo.allocate();
          for (int i = 0; i < NCaloCells; ++i)
            {
              geo->caloSample[i] = temp->caloSample[i];
              geo->x[i] = temp->x[i];
              geo->y[i] = temp->y[i];
              geo->z[i] = temp->z[i];
              geo->eta[i] = temp->eta[i];
              geo->phi[i] = temp->phi[i];
              geo->nNeighbours[i] = temp->nNeighbours[i];
              for (int j = 0; j < NMaxNeighbours; ++j)
                {
                  geo->neighbours[i][j] = temp->neighbours[i][j];
                }
            }
        }
      else if (version == 4)
        {
          std::ifstream in(file.native(), std::ios_base::binary);
          geo.binary_input(in);
          if (in.fail())
            {
              report_error(file, "reading geometry");
              return ErrorState::ReadError;
            }
          in.close();
        }
      else
        {
          return ErrorState::VersionError;
        }
      return ErrorState::OK;
    }

    inline static ErrorState read_noise(const boost::filesystem::path & file, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> & noise, const int version)
    {
      if (version <= 0 || version > current_version)
        {
          return ErrorState::VersionError;
        }
      if (version < 4)
        {
          CaloRecGPU::Helpers::CPU_object<GeometryArr_v1> temp(true);
          if (read_from_file<GeometryArr_v1>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading noise");
              return ErrorState::ReadError;
            }
          noise.allocate();
          for (int i = 0; i < NCaloCells; ++i)
            {
              for (int j = 0; j < CaloRecGPU::NumGainStates; ++j)
                {
                  noise->noise[j][i] = temp->noise[i];
                  //Wrongfully assuming the same noise for all gain states,
                  //but that's what was being done...
                }
            }
        }
      else if (version == 4)
        {
          std::ifstream in(file.native(), std::ios_base::binary);
          noise.binary_input(in);
          if (in.fail())
            {
              report_error(file, "reading noise");
              return ErrorState::ReadError;
            }
          in.close();
        }
      else
        {
          return ErrorState::VersionError;
        }
      return ErrorState::OK;
    }

    inline static ErrorState read_geometry_and_noise(const boost::filesystem::path & file,
                                                     CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> & geo,
                                                     CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> & noise, const int version)
    {
      if (version <= 0 || version > current_version)
        {
          return ErrorState::VersionError;
        }
      else if (version < 4)
        {
          CaloRecGPU::Helpers::CPU_object<GeometryArr_v1> temp(true);
          if (read_from_file<GeometryArr_v1>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading geometry and noise");
              return ErrorState::ReadError;
            }
          geo.allocate();
          noise.allocate();
          for (int i = 0; i < NCaloCells; ++i)
            {
              geo->caloSample[i] = temp->caloSample[i];
              geo->x[i] = temp->x[i];
              geo->y[i] = temp->y[i];
              geo->z[i] = temp->z[i];
              geo->eta[i] = temp->eta[i];
              geo->phi[i] = temp->phi[i];
              for (int j = 0; j < CaloRecGPU::NumGainStates; ++j)
                {
                  noise->noise[j][i] = temp->noise[i];
                  //Wrongfully assuming the same noise for all gain states,
                  //but that's what was being done...
                }
              geo->nNeighbours[i] = temp->nNeighbours[i];
              for (int j = 0; j < NMaxNeighbours; ++j)
                {
                  geo->neighbours[i][j] = temp->neighbours[i][j];
                }
            }
        }
      else
        {
          //Current (and future?) versions
          //save the noise and geometry
          //in separate files, hence the error.
          return ErrorState::VersionError;
        }
      return ErrorState::OK;
    }

    inline static ErrorState read_geometry(const boost::filesystem::path & file, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> & geo)
    {
      return read_geometry(file, geo, guess_version(file));
    }

    inline static ErrorState read_noise(const boost::filesystem::path & file, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> & noise)
    {
      return read_noise(file, noise, guess_version(file));
    }

    inline static ErrorState read_geometry_and_noise(const boost::filesystem::path & file,
                                                     CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> & geo,
                                                     CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> & noise)
    {
      return read_geometry_and_noise(file, geo, noise, guess_version(file));
    }

    inline static ErrorState write_geometry(boost::filesystem::path file, const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> & geo)
    {
      file.replace_extension(".geo");
      std::ofstream out(file, std::ios_base::binary);
      geo.binary_output(out);
      if (out.fail())
        {
          report_error(file, "writing geometry");
          return ErrorState::WriteError;
        }
      out.close();
      return ErrorState::OK;
    }

    inline static ErrorState write_noise(boost::filesystem::path file, const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> & noise)
    {
      file.replace_extension(".noise");
      std::ofstream out(file, std::ios_base::binary);
      noise.binary_output(out);
      if (out.fail())
        {
          report_error(file, "writing noise");
          return ErrorState::WriteError;
        }
      out.close();
      return ErrorState::OK;
    }
  };

  struct EventInformation
  {
   protected:
    constexpr static int NMaxNeighbours = 26;
    constexpr static int NCaloCells = 187652;

    //Data files, Ademar-style .dat
    struct ClusterInfoArr_v1
    {
      constexpr static int NMaxClusters = 100000;
      constexpr static int NClustTimers = 11;
      int nClusters;
      int seedTag[NCaloCells];
      int clusterSize[NMaxClusters];
      float cellSNR[NCaloCells];
      float clusterEnergy[NMaxClusters];
      float clusterEt[NMaxClusters];
      float clusterEta[NMaxClusters];
      float clusterPhi[NMaxClusters];
      float timers[NClustTimers];
    };

    struct CellEnergyArr_v1
    {
      float energy[NCaloCells];
    };

    struct CellTagArr_v1
    {
      int clusterTag[NCaloCells];
    };

    //Data files, Ademar-style .diag
    struct ClusterInfoArr_v2
    {
      constexpr static int NMaxClusters = 50000;
      int   seedTag[NCaloCells];
      float cellE[NCaloCells];
      int nClusters;
      int clusterNCells[NMaxClusters];
      float clusterEnergy[NMaxClusters];
      float clusterEt[NMaxClusters];
      float clusterEta[NMaxClusters];
      float clusterPhi[NMaxClusters];
    };

    inline static void cluster_tag_adjust(CaloRecGPU::tag_type & tag, const int old_tag, const int index)
    {
      if (old_tag < 0)
        //Old way of storing had cells not part of clusters having tag -1.
        {
          tag = CaloRecGPU::Tags::InvalidTag;
        }
      else
        {
          tag = CaloRecGPU::Tags::make_seed_tag(0x7f7fffff, index, old_tag);
          //0x7f7fffff is the largest possible valid (finite non-NaN) floating point value.
        }
    }

   public:
    inline static ErrorState read_cluster_info(const boost::filesystem::path & file, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters, const int version)
    {
      if (version <= 0 || version > current_version)
        {
          return ErrorState::VersionError;
        }
      else if (version == 1)
        {
          CaloRecGPU::Helpers::CPU_object<ClusterInfoArr_v1> temp(true);
          if (read_from_file<ClusterInfoArr_v1>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading clusters");
              return ErrorState::ReadError;
            }
          clusters.allocate();
          clusters->number = temp->nClusters;
          for (int i = 0; i < temp->nClusters; ++i)
            {
              clusters->clusterEnergy[i] = temp->clusterEnergy[i];
              clusters->clusterEt[i] = temp->clusterEt[i];
              clusters->clusterEta[i] = temp->clusterEta[i];
              clusters->clusterPhi[i] = temp->clusterPhi[i];
              clusters->seedCellID[i] = -1;
              //One would need to recalculate if using this for old versions...
              //(In this case we could actually recalculate since we have the SNR,
              // but for version 2 that's not possible, so it's best to be consistent
              // and ask the user to handle it if necessary.)
            }
        }
      else if (version == 2)
        {
          CaloRecGPU::Helpers::CPU_object<ClusterInfoArr_v2> temp(true);
          if (read_from_file<ClusterInfoArr_v2>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading clusters");
              return ErrorState::ReadError;
            }
          clusters.allocate();
          clusters->number = temp->nClusters;
          for (int i = 0; i < temp->nClusters; ++i)
            {
              clusters->clusterEnergy[i] = temp->clusterEnergy[i];
              clusters->clusterEt[i] = temp->clusterEt[i];
              clusters->clusterEta[i] = temp->clusterEta[i];
              clusters->clusterPhi[i] = temp->clusterPhi[i];
              clusters->seedCellID[i] = -1;
              //One would need to recalculate if using this for old versions...
            }
        }
      else if (version == 3)
        {
          clusters.allocate();

          std::ifstream in(file.native(), std::ios_base::binary);

          in.ignore(sizeof(int) * NCaloCells);
          //seedTag
          in.ignore(sizeof(float) * NCaloCells);
          //cellE

          if (in.fail())
            {
              report_error(file, "reading clusters");
              return ErrorState::ReadError;
            }

          in.read((char *) & (clusters->number), sizeof(int));

          if (in.fail() || clusters->number < 0 || clusters->number > ClusterInfoArr_v2::NMaxClusters)
            {
              report_error(file, "reading clusters");
              return ErrorState::ReadError;
            }

          in.ignore(sizeof(int) * clusters->number);
          //clusterNCells

          in.read((char *) clusters->clusterEnergy, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterEt, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterEta, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterPhi, sizeof(float) * clusters->number);

          if (in.fail())
            {
              report_error(file, "reading clusters");
              return ErrorState::ReadError;
            }

          in.close();

          for (int i = 0; i < clusters->number; ++i)
            {
              clusters->seedCellID[i] = -1;
            }
          //One would need to recalculate if using this for old versions...

        }
      else if (version == 4)
        {
          clusters.allocate();

          std::ifstream in(file.native(), std::ios_base::binary);

          in.read((char *) & (clusters->number), sizeof(int));

          if (in.fail() || clusters->number < 0)
            {
              report_error(file, "reading clusters");
              return ErrorState::ReadError;
            }

          in.read((char *) clusters->clusterEnergy, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterEt, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterEta, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterPhi, sizeof(float) * clusters->number);
          in.read((char *) clusters->seedCellID, sizeof(int) * clusters->number);

          if (in.fail())
            {
              report_error(file, "reading clusters");
              return ErrorState::ReadError;
            }

          in.close();
        }
      else
        {
          return ErrorState::VersionError;
        }
      return ErrorState::OK;

    }

    inline static ErrorState read_cell_info(const boost::filesystem::path & file,
                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                            const int version)
    {
      if (version <= 0 || version > current_version)
        {
          return ErrorState::VersionError;
        }
      else if (version == 1)
        //version 1 does not have a file where one can read all the necessary info,
        //only the energies, but we keep this here for easier folder reading...
        {
          cell_info.allocate();
          cell_state.allocate();
          CaloRecGPU::Helpers::CPU_object<CellEnergyArr_v1> temp(true);
          if (read_from_file<CellEnergyArr_v1>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading cells");
              return ErrorState::ReadError;
            }
          for (int i = 0; i < NCaloCells; ++i)
            {
              cell_info->energy[i] = temp->energy[i];
              cell_info->gain[i] = 0;
              //We don't have varying gains in this case,
              //so we just assume it's constant and equal to the first type.
              //(A guess as good as any...)
            }
        }
      else if (version == 2)
        {
          CaloRecGPU::Helpers::CPU_object<ClusterInfoArr_v2> temp(true);
          if (read_from_file<ClusterInfoArr_v2>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading cells");
              return ErrorState::ReadError;
            }
          cell_info.allocate();
          cell_state.allocate();
          for (int i = 0; i < NCaloCells; ++i)
            {
              cell_info->energy[i] = temp->cellE[i];
              cell_info->gain[i] = 0;
              //We don't have varying gains in this case,
              //so we just assume it's constant and equal to the first type.
              //(A guess as good as any...)

              cluster_tag_adjust(cell_state->clusterTag[i], temp->seedTag[i], i);
            }
        }
      else if (version == 3)
        {
          cell_info.allocate();
          cell_state.allocate();

          CaloRecGPU::Helpers::CPU_object<CellTagArr_v1> temp(true);

          std::ifstream in(file.native(), std::ios_base::binary);

          in.read((char *) temp->clusterTag, sizeof(int) * NCaloCells);
          in.read((char *) cell_info->energy, sizeof(float) * NCaloCells);

          if (in.fail())
            {
              report_error(file, "reading clusters");
              return ErrorState::ReadError;
            }
          for (int i = 0; i < NCaloCells; ++i)
            {
              cell_info->gain[i] = 0;
              //We don't have varying gains in this case,
              //so we just assume it's constant and equal to the first type.
              //(A guess as good as any...)

              cluster_tag_adjust(cell_state->clusterTag[i], temp->clusterTag[i], i);
            }
        }
      else if (version == 4)
        {
          std::ifstream in(file.native(), std::ios_base::binary);
          cell_info.binary_input(in);
          cell_state.binary_input(in);
          if (in.fail())
            {
              report_error(file, "reading cells");
              return ErrorState::ReadError;
            }
          in.close();
        }
      else
        {
          return ErrorState::VersionError;
        }
      return ErrorState::OK;

    }


    inline static ErrorState read_cell_and_cluster_info(const boost::filesystem::path & file,
                                                        CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                                        CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                                        CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters,
                                                        const int version)
    {
      if (version <= 0 || version > current_version)
        {
          return ErrorState::VersionError;
        }
      else if (version == 1)
        //version 1 does not have a file where one can read all the necessary info,
        //since the energies are stored separately, but we keep this here for easier folder reading...
        {
          CaloRecGPU::Helpers::CPU_object<ClusterInfoArr_v1> temp(true);
          if (read_from_file<ClusterInfoArr_v1>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading cells and clusters");
              return ErrorState::ReadError;
            }
          clusters.allocate();
          cell_state.allocate();
          cell_info.allocate();
          clusters->number = temp->nClusters;
          for (int i = 0; i < temp->nClusters; ++i)
            {
              clusters->clusterEnergy[i] = temp->clusterEnergy[i];
              clusters->clusterEt[i] = temp->clusterEt[i];
              clusters->clusterEta[i] = temp->clusterEta[i];
              clusters->clusterPhi[i] = temp->clusterPhi[i];
              clusters->seedCellID[i] = -1;
            }
          for (int i = 0; i < NCaloCells; ++i)
            {
              cluster_tag_adjust(cell_state->clusterTag[i], temp->seedTag[i], i);
            }
        }
      else if (version == 2)
        {
          CaloRecGPU::Helpers::CPU_object<ClusterInfoArr_v2> temp(true);
          if (read_from_file<ClusterInfoArr_v2>(file, temp) != ErrorState::OK)
            {
              report_error(file, "reading cells and clusters");
              return ErrorState::ReadError;
            }
          clusters.allocate();
          cell_state.allocate();
          cell_info.allocate();

          clusters->number = temp->nClusters;
          for (int i = 0; i < temp->nClusters; ++i)
            {
              clusters->clusterEnergy[i] = temp->clusterEnergy[i];
              clusters->clusterEt[i] = temp->clusterEt[i];
              clusters->clusterEta[i] = temp->clusterEta[i];
              clusters->clusterPhi[i] = temp->clusterPhi[i];
              clusters->seedCellID[i] = -1;
            }
          for (int i = 0; i < NCaloCells; ++i)
            {
              cell_info->energy[i] = temp->cellE[i];
              cell_info->gain[i] = 0;
              cluster_tag_adjust(cell_state->clusterTag[i], temp->seedTag[i], i);
            }
        }
      else if (version == 3)
        {
          clusters.allocate();
          cell_info.allocate();
          cell_state.allocate();
          CaloRecGPU::Helpers::CPU_object<CellTagArr_v1> temp(true);

          std::ifstream in(file.native(), std::ios_base::binary);

          in.read((char *) temp->clusterTag, sizeof(int) * NCaloCells);
          in.read((char *) cell_info->energy, sizeof(float) * NCaloCells);

          if (in.fail())
            {
              report_error(file, "reading cells and clusters");
              return ErrorState::ReadError;
            }

          in.read((char *) & (clusters->number), sizeof(int));

          if (in.fail() || clusters->number < 0 || clusters->number > ClusterInfoArr_v2::NMaxClusters)
            {
              report_error(file, "reading cells and clusters");
              return ErrorState::ReadError;
            }

          in.ignore(sizeof(int) * clusters->number);
          //clusterNCells

          in.read((char *) clusters->clusterEnergy, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterEt, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterEta, sizeof(float) * clusters->number);
          in.read((char *) clusters->clusterPhi, sizeof(float) * clusters->number);

          if (in.fail())
            {
              report_error(file, "reading cells and clusters");
              return ErrorState::ReadError;
            }

          in.close();

          for (int i = 0; i < clusters->number; ++i)
            {
              clusters->seedCellID[i] = -1;
            }

          for (int i = 0; i < NCaloCells; ++i)
            {
              cell_info->gain[i] = 0;
              //We don't have varying gains in this case,
              //so we just assume it's constant and equal to the first type.
              //(A guess as good as any...)

              cluster_tag_adjust(cell_state->clusterTag[i], temp->clusterTag[i], i);
            }
        }
      else
        {
          return ErrorState::VersionError;
        }
      return ErrorState::OK;
    }

    inline static ErrorState read_cluster_info(const boost::filesystem::path & file, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters)
    {
      return read_cluster_info(file, clusters, guess_version(file));
    }

    inline static ErrorState read_cell_info(const boost::filesystem::path & file,
                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state)
    {
      return read_cell_info(file, cell_info, cell_state, guess_version(file));
    }

    inline static ErrorState read_cell_and_cluster_info(const boost::filesystem::path & file,
                                                        CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                                        CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                                        CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters)
    {
      return read_cell_and_cluster_info(file, cell_info, cell_state, clusters, guess_version(file));
    }

    inline static ErrorState write_cluster_info(boost::filesystem::path file, const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters)
    {
      file.replace_extension(".clusters");
      std::ofstream out(file, std::ios_base::binary);

      out.write((char *) & (clusters->number), sizeof(int));
      out.write((char *) clusters->clusterEnergy, sizeof(float) * clusters->number);
      out.write((char *) clusters->clusterEt, sizeof(float) * clusters->number);
      out.write((char *) clusters->clusterEta, sizeof(float) * clusters->number);
      out.write((char *) clusters->clusterPhi, sizeof(float) * clusters->number);
      out.write((char *) clusters->seedCellID, sizeof(int) * clusters->number);

      if (out.fail())
        {
          report_error(file, "writing clusters");
          return ErrorState::WriteError;
        }
      out.close();
      return ErrorState::OK;
    }

    inline static ErrorState write_cell_info(boost::filesystem::path file,
                                             const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                             const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state)
    {
      file.replace_extension(".cells");
      std::ofstream out(file, std::ios_base::binary);
      cell_info.binary_output(out);
      cell_state.binary_output(out);
      if (out.fail())
        {
          report_error(file, "writing cells");
          return ErrorState::WriteError;
        }
      out.close();
      return ErrorState::OK;
    }

  };

 protected:

  inline static bool create_or_check_folder(const boost::filesystem::path & folder, const bool output_errors = true)
  {
    if (!boost::filesystem::exists(folder))
      {
        if (!boost::filesystem::create_directory(folder))
          {
            if (output_errors)
              {
                std::cout << "ERROR: folder '" << folder << "' could not be created." << std::endl;
              }
            return false;
          }
      }
    else if (!boost::filesystem::is_directory(folder))
      {
        if (output_errors)
          {
            std::cout << "ERROR: folder '" << folder << "' is not a valid folder." << std::endl;
          }
        return false;
      }
    return true;
  }

 public:

  inline static ErrorState prepare_folder_for_output(const boost::filesystem::path & folder, const bool output_errors = true)
  {
    if (!create_or_check_folder(folder, output_errors))
      {
        return ErrorState::WriteError;
      }
    return ErrorState::OK;
  }

  inline static std::string build_filename( const std::string & prefix,
                                            const std::string & text,
                                            const std::string & suffix,
                                            const std::string & ext       )
  {
    return prefix + (prefix.size() > 0 ? "_" : "") + text +
           (suffix.size() > 0 ? "_" : "") + suffix + "." + ext;
  }

  inline static std::string build_filename( const std::string & prefix,
                                            const size_t event_number,
                                            const std::string & suffix,
                                            const std::string & ext,
                                            const unsigned int num_width = 9)
  {
    std::ostringstream event_ID_format;
    event_ID_format << std::setfill('0') << std::setw(num_width) << event_number;
    const std::string event_ID = event_ID_format.str();
    return build_filename(prefix, event_ID, suffix, ext);
  }


  inline static ErrorState save_constants_to_folder(const boost::filesystem::path & folder,
                                                    const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> & geo,
                                                    const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> & noise,
                                                    const std::string & prefix = "",
                                                    const std::string & suffix = "",
                                                    const bool output_errors = true)
  {
    if (!create_or_check_folder(folder, output_errors))
      {
        return ErrorState::WriteError;
      }

    auto filename = [&] (const std::string & text, const std::string & ext)
    {
      return folder / build_filename(prefix, text, suffix, ext);
    };

    if (ConstantInformation::write_geometry(filename("geometry", "geo"), geo) != ErrorState::OK)
      {
        return ErrorState::WriteError;
      }
    if (ConstantInformation::write_noise(filename("noise", "noise"), noise) != ErrorState::OK)
      {
        return ErrorState::WriteError;
      }
    return ErrorState::OK;
  }

  inline static ErrorState save_event_to_folder(const size_t event_number,
                                                const boost::filesystem::path & folder,
                                                const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                                const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                                const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters,
                                                const std::string & prefix = "",
                                                const std::string & suffix = "",
                                                const unsigned int num_width = 9,
                                                const bool output_errors = true)
  {
    if (!create_or_check_folder(folder, output_errors))
      {
        return ErrorState::WriteError;
      }

    std::ostringstream event_ID_format;
    event_ID_format << std::setfill('0') << std::setw(num_width) << event_number;
    const std::string event_ID = event_ID_format.str();

    auto filename = [&] (const std::string & ext)
    {
      return folder / build_filename(prefix, event_ID, suffix, ext);
    };

    if (EventInformation::write_cell_info(filename("cells"), cell_info, cell_state) != ErrorState::OK)
      {
        return ErrorState::WriteError;
      }
    if (EventInformation::write_cluster_info(filename("clusters"), clusters) != ErrorState::OK)
      {
        return ErrorState::WriteError;
      }
    return ErrorState::OK;
  }

  inline static ErrorState save_cells_to_folder(const size_t event_number,
                                                const boost::filesystem::path & folder,
                                                const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> & cell_info,
                                                const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> & cell_state,
                                                const std::string & prefix = "",
                                                const std::string & suffix = "",
                                                const unsigned int num_width = 9,
                                                const bool output_errors = true)
  {
    if (!create_or_check_folder(folder, output_errors))
      {
        return ErrorState::WriteError;
      }

    std::ostringstream event_ID_format;
    event_ID_format << std::setfill('0') << std::setw(num_width) << event_number;
    const std::string event_ID = event_ID_format.str();

    auto filename = [&] (const std::string & ext)
    {
      return folder / build_filename(prefix, event_ID, suffix, ext);
    };

    if (EventInformation::write_cell_info(filename("cells"), cell_info, cell_state) != ErrorState::OK)
      {
        return ErrorState::WriteError;
      }
    return ErrorState::OK;
  }

  inline static ErrorState save_clusters_to_folder(const size_t event_number,
                                                   const boost::filesystem::path & folder,
                                                   const CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> & clusters,
                                                   const std::string & prefix = "",
                                                   const std::string & suffix = "",
                                                   const unsigned int num_width = 9,
                                                   const bool output_errors = true)
  {
    if (!create_or_check_folder(folder, output_errors))
      {
        return ErrorState::WriteError;
      }

    std::ostringstream event_ID_format;
    event_ID_format << std::setfill('0') << std::setw(num_width) << event_number;
    const std::string event_ID = event_ID_format.str();

    auto filename = [&] (const std::string & ext)
    {
      return folder / build_filename(prefix, event_ID, suffix, ext);
    };

    if (EventInformation::write_cluster_info(filename("clusters"), clusters) != ErrorState::OK)
      {
        return ErrorState::WriteError;
      }
    return ErrorState::OK;
  }

  struct FolderLoad
  {
    std::map<std::string, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr>> geometry;
    std::map<std::string, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr>> noise;
    std::map<std::string, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr>> clusters;
    std::map<std::string, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr>> cell_info;
    std::map<std::string, CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr>> cell_state;
  };


  template <class F>
  inline static FolderLoad load_folder_filter(F && filter_function,
                                              //Receives a std::string (the filename),
                                              //returns `true` if the file should be filtered out.
                                              const boost::filesystem::path & folder,
                                              int max_events = -1,
                                              const bool load_clusters = true,
                                              const bool load_cells = true,
                                              const bool load_geometry = true,
                                              const bool load_noise = true,
                                              const bool output_messages = true)
  {
    FolderLoad ret;
    if (!boost::filesystem::is_directory(folder))
      {
        if (output_messages)
          {
            std::cout << "ERROR: '" << folder << "' is not a valid folder." << std::endl;
          }
        return ret;
      }
    std::set<std::string> read_one_part_of_v1_cells;
    int event_count = 0;
    for (const boost::filesystem::path & file : boost::filesystem::directory_iterator(folder))
      {
        if (max_events > 0 && event_count >= max_events && ret.geometry.size() > 0 && ret.noise.size() > 0)
          {
            break;
          }
        const bool can_load_events = (max_events < 0) || (event_count < max_events);
        const std::string filename = file.stem().native();
        if (filter_function(filename))
          {
            continue;
          }
        auto check_error = [&](const ErrorState & es, const std::string & str)
        {
          if (es == ErrorState::OK)
            {
              return false;
            }
          if (output_messages)
            {
              std::cout << "ERROR: '" << file << "' is not a valid " << str << " file (" << (int) es << ")." << std::endl;
            }
          return true;
        };
        auto output_loading_message = [&](const std::string & str)
        {
          if (output_messages)
            {
              std::cout << "Loaded " << str << " from '" << file << "'." << std::endl;
            }
        };
        if (file.extension() == ".geo")
          {
            if (!load_geometry)
              {
                continue;
              }
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> tempgeo(true);
            if (check_error(ConstantInformation::read_geometry(file, tempgeo), "geometry"))
              {
                continue;
              }
            ret.geometry[filename] = std::move(tempgeo);
            output_loading_message("geometry");
          }
        else if (filename.find("geometry") != std::string::npos)
          {
            if (!load_geometry && !load_noise)
              {
                continue;
              }
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::GeometryArr> tempgeo(true);
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> tempnois(true);
            if (check_error(ConstantInformation::read_geometry_and_noise(file, tempgeo, tempnois), "geometry and noise"))
              {
                continue;
              }
            if (load_geometry)
              {
                ret.geometry[filename] = std::move(tempgeo);
              }
            if (load_noise)
              {
                ret.noise[filename] = std::move(tempnois);
              }
            output_loading_message("geometry and noise");
          }
        else if (file.extension() == ".noise")
          {
            if (!load_noise)
              {
                continue;
              }
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellNoiseArr> tempnois(true);
            if (check_error(ConstantInformation::read_noise(file, tempnois), "noise"))
              {
                continue;
              }
            ret.noise[filename] = std::move(tempnois);
            output_loading_message("noise");
          }
        else if (file.extension() == ".clusters")
          {
            if (!load_clusters || !can_load_events)
              {
                continue;
              }
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> tempclu(true);
            if (check_error(EventInformation::read_cluster_info(file, tempclu), "clusters"))
              {
                continue;
              }
            ret.clusters[filename] = std::move(tempclu);
            output_loading_message("clusters");
            if (ret.cell_info.count(filename) > 0 || !load_cells)
              {
                ++event_count;
              }
          }
        else if (file.extension() == ".cells")
          {
            if (!load_cells || !can_load_events)
              {
                continue;
              }
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> tempcellinfo(true);
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> tempcellstate(true);
            if (check_error(EventInformation::read_cell_info(file, tempcellinfo, tempcellstate), "cells"))
              {
                continue;
              }
            ret.cell_info[filename] = std::move(tempcellinfo);
            ret.cell_state[filename] = std::move(tempcellstate);
            output_loading_message("cells");
            if (ret.clusters.count(filename) > 0 || !load_clusters)
              {
                ++event_count;
              }
          }
        else if ((file.extension() == ".diag" || file.extension() == ".abrv"))
          {
            if (!(load_cells || load_clusters) || !can_load_events)
              {
                continue;
              }
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> tempcellinfo(true);
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> tempcellstate(true);
            CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> tempclu(true);
            if (check_error(EventInformation::read_cell_and_cluster_info(file, tempcellinfo, tempcellstate, tempclu), "cells and clusters"))
              {
                continue;
              }
            if (load_cells)
              {
                ret.cell_info[filename] = std::move(tempcellinfo);
                ret.cell_state[filename] = std::move(tempcellstate);
              }
            if (load_clusters)
              {
                ret.clusters[filename] = std::move(tempclu);
              }
            if (load_cells && load_clusters)
              {
                output_loading_message("cells and clusters");
              }
            else if (load_cells)
              {
                output_loading_message("cells");
              }
            else if (load_clusters)
              {
                output_loading_message("clusters");
              }
            ++event_count;
          }
        else if (file.extension() == ".dat")
          {
            if (!(load_cells || load_clusters) || !can_load_events)
              {
                continue;
              }
            const auto str_starts_with = [ ](const std::string & str, const std::string & start)
            {
              for (size_t i = 0; i < start.size(); ++i)
                {
                  if (str[i] != start[i])
                    {
                      return false;
                    }
                }
              return true;
            };

            if (str_starts_with(filename, "cellData_"))
              {
                if (!load_cells)
                  {
                    continue;
                  }
                const std::string real_name = filename.substr(9);
                if (read_one_part_of_v1_cells.count(real_name) > 0 && ret.cell_info.count(real_name) == 0)
                  {
                    //The other part of the event information was not properly loaded.
                    continue;
                  }
                else if (read_one_part_of_v1_cells.count(real_name) > 0)
                  {
                    if (check_error(EventInformation::read_cell_info(file, ret.cell_info[real_name], ret.cell_state[real_name]), "cells"))
                      {
                        ret.cell_info.erase(real_name);
                        ret.cell_state.erase(real_name);
                        ret.clusters.erase(real_name);
                        continue;
                      }
                    ++event_count;
                  }
                else
                  {
                    read_one_part_of_v1_cells.insert(real_name);
                    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> tempcellinfo(true);
                    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> tempcellstate(true);
                    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> tempclu(true);
                    if (check_error(EventInformation::read_cell_info(file, tempcellinfo, tempcellstate), "cells"))
                      {
                        continue;
                      }
                    if (load_cells)
                      {
                        ret.cell_info[filename] = std::move(tempcellinfo);
                        ret.cell_state[filename] = std::move(tempcellstate);
                      }
                    if (load_clusters)
                      {
                        ret.clusters[filename] = std::move(tempclu);
                      }
                  }
                output_loading_message("partial cell data");
              }
            else if (str_starts_with(filename, "clusterData_"))
              {
                if (!load_cells && !load_clusters)
                  {
                    continue;
                  }
                const std::string real_name = filename.substr(12);
                if (read_one_part_of_v1_cells.count(real_name) > 0 && ret.cell_info.count(real_name) == 0 && load_cells)
                  {
                    //The other part of the event information was not properly loaded.
                    continue;
                  }
                else if (read_one_part_of_v1_cells.count(real_name) > 0)
                  {
                    if (check_error(EventInformation::read_cell_and_cluster_info(file, ret.cell_info[real_name], ret.cell_state[real_name], ret.clusters[real_name]), "cells and clusters"))
                      {
                        ret.cell_info.erase(real_name);
                        ret.cell_state.erase(real_name);
                        ret.clusters.erase(real_name);
                        continue;
                      }
                    ++event_count;
                  }
                else
                  {
                    read_one_part_of_v1_cells.insert(real_name);
                    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellInfoArr> tempcellinfo(true);
                    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::CellStateArr> tempcellstate(true);
                    CaloRecGPU::Helpers::CPU_object<CaloRecGPU::ClusterInfoArr> tempclu(true);
                    if (check_error(EventInformation::read_cell_and_cluster_info(file, tempcellinfo, tempcellstate, tempclu), "cells and clusters"))
                      {
                        continue;
                      }
                    if (load_cells)
                      {
                        ret.cell_info[filename] = std::move(tempcellinfo);
                        ret.cell_state[filename] = std::move(tempcellstate);
                      }
                    if (load_clusters)
                      {
                        ret.clusters[filename] = std::move(tempclu);
                      }
                  }
                output_loading_message("clusters and partial cell data");
              }
            else
              {
                if (output_messages)
                  {
                    std::cout << "ERROR: '" << file << "' does not seem to be a valid file for data input." << std::endl;
                  }
              }
          }
        else if (can_load_events)
          {
            if (output_messages)
              {
                std::cout << "ERROR: '" << file << "' does not seem to be a valid file for data input." << std::endl;
              }
          }
      }
    return ret;
  }

  inline static FolderLoad load_folder(const boost::filesystem::path & folder,
                                       int max_events = -1,
                                       const bool load_clusters = true,
                                       const bool load_cells = true,
                                       const bool load_geometry = true,
                                       const bool load_noise = true,
                                       const bool output_messages = true)
  {
    return load_folder_filter([&](const std::string &)
    {
      return false;
    },
    folder, max_events, load_clusters, load_cells, load_geometry, load_noise, output_messages);
  }

};


#endif //CALORECGPU_STANDALONEDATAIO_H