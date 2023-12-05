/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
#ifndef TRIGACCELEVENT_TRIGTRACKFOLLOWING_H
#define TRIGACCELEVENT_TRIGTRACKFOLLOWING_h
//GPU-accelerated track following
  
  static constexpr unsigned int MAX_NUMBER_PIX_HITS          = 100000;
  static constexpr unsigned int MAX_NUMBER_SCT_HITS          = 100000;
  static constexpr unsigned int MAX_NUMBER_INPUT_SEEDS       = 50000;
  static constexpr unsigned int MAX_ROAD_LENGTH              = 64;
  
  typedef struct TrackFinderSettings {
  public:
    unsigned int m_maxBarrelPix, m_minEndcapPix, m_maxEndcapPix, m_maxSiliconLayer; 
    float m_magFieldZ; 
    float m_tripletD0Max; 
    float m_tripletD0_PPS_Max; 
    float m_tripletPtMin; 
    int  m_tripletDoPSS, m_doubletFilterRZ; 
    int m_nMaxPhiSlice; 
    unsigned int m_maxTripletBufferLength; 
    int m_isFullScan;
  } TRACK_FINDER_SETTINGS;
  
  typedef struct SiliconPlane{
  public:
    double m_Ax[3];
    double m_Ay[3];
    double m_Az[3];
    double m_D[3];
    double m_B[3];//magnetic field in the center
    float m_minWidth;
    float m_maxWidth;
    float m_length;
    int m_shape;
  } SILICON_PLANE;

  typedef struct SctGeometryStorage {
  public:
    int m_nModules;
    int m_dead[MAX_NUMBER_SCT_MODULES];
    int m_type[MAX_NUMBER_SCT_MODULES];
    SILICON_PLANE m_geoInfo[MAX_NUMBER_SCT_MODULES];
  } SCT_GEO_STORAGE;

  typedef struct PixelGeometryStorage {
  public:
    int m_nModules;
    int m_dead[MAX_NUMBER_PIX_MODULES];
    int m_type[MAX_NUMBER_PIX_MODULES];
    SILICON_PLANE m_geoInfo[MAX_NUMBER_PIX_MODULES];
  } PIXEL_GEO_STORAGE;

  typedef struct PixelClusterStorage {
  public:
    int m_nModules;
    int m_start[MAX_NUMBER_PIX_MODULES];
    int   m_end[MAX_NUMBER_PIX_MODULES];
    float m_localX[MAX_NUMBER_PIX_HITS];
    float m_localY[MAX_NUMBER_PIX_HITS];
    float m_covXX[MAX_NUMBER_PIX_HITS];
    float m_covXY[MAX_NUMBER_PIX_HITS];
    float m_covYY[MAX_NUMBER_PIX_HITS];
  } PIXEL_CLUSTER_STORAGE;
  
  typedef struct SctClusterStorage {
  public:
    int m_nModules;
    int m_start[MAX_NUMBER_SCT_MODULES];
    int   m_end[MAX_NUMBER_SCT_MODULES];
    float m_localX[MAX_NUMBER_SCT_HITS];
    float m_covXX[MAX_NUMBER_SCT_HITS];
  } SCT_CLUSTER_STORAGE;
  
  typedef struct ProtoTrack {
    int m_nElements;
    int m_nSeedElements;
    float m_initialParams[5];
    int m_planeType[MAX_ROAD_LENGTH];
    int m_planeIndex[MAX_ROAD_LENGTH];
    int m_hitIndex[MAX_ROAD_LENGTH];
    int m_seedPlanes[6];//max = 3 SP x 2 clusters
  } PROTO_TRACK;

  typedef struct ProtoTrackStorage {
  public:
    int m_nSeeds;
    PROTO_TRACK m_tracks[MAX_NUMBER_INPUT_SEEDS];
  } PROTO_TRACK_STORAGE;


  typedef struct TrackSeedStorage {
  public:
    int m_nSeeds;
    int m_planeType[MAX_NUMBER_INPUT_SEEDS];
    int m_planeIdx[MAX_NUMBER_INPUT_SEEDS];
    int m_sp1stPlaneIndices[MAX_NUMBER_INPUT_SEEDS][3];
    int m_sp2ndPlaneIndices[MAX_NUMBER_INPUT_SEEDS][3];
    int m_spClusterIndices[MAX_NUMBER_INPUT_SEEDS][6];
    float m_sp1x[MAX_NUMBER_INPUT_SEEDS];
    float m_sp1y[MAX_NUMBER_INPUT_SEEDS];
    float m_sp1z[MAX_NUMBER_INPUT_SEEDS];
    float m_sp2x[MAX_NUMBER_INPUT_SEEDS];
    float m_sp2y[MAX_NUMBER_INPUT_SEEDS];
    float m_sp2z[MAX_NUMBER_INPUT_SEEDS];
    float m_sp3x[MAX_NUMBER_INPUT_SEEDS];
    float m_sp3y[MAX_NUMBER_INPUT_SEEDS];
    float m_sp3z[MAX_NUMBER_INPUT_SEEDS];

  } TRACK_SEED_STORAGE;

  typedef struct ExtendedTrackStateStruct2 {
    float m_par[10];
    float m_cov[55];
  } EXTENDED_TRACK_STATE_TYPE_2;
  
  typedef struct LocalEstimate {
  public:
    float m_P[5];
    float m_J[5][5];//jacobian to get to the next plane estimate
    float m_path;
  } LOCAL_ESTIMATE;

  typedef struct TrackData {
  public:
    int m_status;
    float m_chi2;
    int m_ndof;
    int m_firstElement;
    int m_lastElement;
    char m_stat[MAX_ROAD_LENGTH];
    char m_validatedPlanes[MAX_ROAD_LENGTH];

    LOCAL_ESTIMATE m_E[MAX_ROAD_LENGTH];
    
    EXTENDED_TRACK_STATE_TYPE_2 m_ets;
    int m_nValidated;
  } TRACK_DATA;

  typedef struct SeededOutput {
  public:
    TRACK_DATA m_data[MAX_NUMBER_INPUT_SEEDS];
  } SEEDED_OUTPUT;

  #endif