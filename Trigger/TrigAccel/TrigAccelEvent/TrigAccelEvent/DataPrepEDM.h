  /*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/
  #ifndef TRIGACCELEVENT_DATAPREPEDM_H
  #define TRIGACCELEVENT_DATAPREPEDM_H
  //GPU-accelerated data preparation
  
  static constexpr unsigned int MAX_NUMBER_BS_WORDS          = 80000;
  static constexpr unsigned int MAX_GANGED_PIXELS            = 4000;
  static constexpr unsigned int MAX_GANGED_PIXEL_PER_MODULE  = 50;
  static constexpr unsigned int MAX_PIX_BS_HEADERS           = 2048;
  static constexpr unsigned int MAX_PIX_HASH                 = 2048;
  static constexpr unsigned int MAX_SCT_BS_HEADERS           = 8500;
  static constexpr unsigned int MAX_SCT_HASH                 = 8176;
  static constexpr unsigned int MAX_BS_ROBF                  = 200;
  static constexpr unsigned int MAX_ADJ_HIT_PAIRS            = 1024;
  static constexpr unsigned int SCT_MAX_SP_PER_MODULE        = 256;
  static constexpr unsigned int MAX_PHI_INDEX                = 100;
  static constexpr unsigned int MAX_RZ_INDEX                 = 300;
  static constexpr unsigned int MAX_PIX_ROD_INDEX            = 200;
  static constexpr unsigned int MAX_PIX_LINK_INDEX           = 8;
  static constexpr unsigned int MAX_SCT_ROD_INDEX            = 90;
  static constexpr unsigned int MAX_SCT_LINK_INDEX           = 96;
  
  typedef struct SctModuleGeoInfo {
    char m_type;
    float m_phiPitch;
    float m_lorentzShift;
    float m_center[3];
    float m_M[3][3];
    //special meaning for endcap 
    float m_stripLength;
    float m_maxRadius;
    float m_minRadius;
    
  } SCT_MODULE_GEO_INFO;
  
  typedef struct PixelModuleGeoInfo {
    char m_type;
    char m_bLayer;
    float m_phiPitch;
    float m_etaPitchLong;
    float m_etaPitchNormal;
    float m_lorentzShift;
    float m_center[3];
    float m_M[3][3];
    float m_halfWidth;
    float m_halfLength;
  } PIXEL_MODULE_GEO_INFO;

  struct HashQuadruplet {
    unsigned short x,y,z,w;
  };

  typedef struct IdCablingInfo {
  public:
    HashQuadruplet m_pixelRodLinkHashTable[MAX_PIX_ROD_INDEX][MAX_PIX_LINK_INDEX];
    unsigned short m_sctRodLinkHashTable[MAX_SCT_ROD_INDEX][MAX_SCT_LINK_INDEX];
    uint16_t m_pixelModuleInfo[MAX_PIX_HASH];
    uint16_t m_sctModuleInfo[MAX_SCT_HASH];
    PIXEL_MODULE_GEO_INFO m_pixelGeoArray[MAX_PIX_HASH];
    SCT_MODULE_GEO_INFO m_sctGeoArray[MAX_SCT_HASH];
  } ID_CABLING_INFO;

  struct myfloat2 {
    float x,y;
  };

  struct myint4 {
    int x,y,z,w;
  };

  struct myushort4 {
    unsigned short x,y,z,w;
  };

  struct myushort2 {
    unsigned short x,y;
  };

  typedef struct InputByteStreamData {  
    int m_nDataWords;
    uint32_t m_rodIds[MAX_NUMBER_BS_WORDS];
    uint32_t m_words[MAX_NUMBER_BS_WORDS];
    float m_xBeamSpot;
    float m_yBeamSpot; 
  } INPUT_BYTESTREAM_DATA;


  typedef struct DecodedPixelModuleInfo {
    int m_headerPositions;
    int m_trailerPositions;
    int m_gangedStart;
    short m_hashIds;
    unsigned int m_nClusters;
    unsigned int m_nGangedPixels;
  } DECODED_PIX_MODULE_INFO;

  typedef struct DecodedPixelHitInfo {
    unsigned int m_clusterIds;
    unsigned short m_etaIndex;
    unsigned short m_phiIndex;
    unsigned int m_tot;
  } DECODED_PIX_HIT_INFO;

  typedef struct PixelSpacePointType {
    float m_position[3];
    unsigned short m_clusterIdx;
  } PIXEL_SPACEPOINT_TYPE;
  

  typedef struct DecodedPixelData {
  public:
    int m_nHeaders, m_nTrailers;
    int m_nSpacePoints;
    
    int m_gangedPixelsStart;//initial value = m_nDataWords
    int m_nPixels[MAX_PIX_BS_HEADERS];//number of decoded pixels (counting ganged) per module
    int m_clusterStarts[MAX_PIX_BS_HEADERS];//for navigation through cluster and SP storage space
    DECODED_PIX_MODULE_INFO m_modulesInfo[MAX_PIX_BS_HEADERS];
    unsigned short m_moduleInfoWord[MAX_PIX_BS_HEADERS];
    myfloat2 m_clusterPosition[MAX_NUMBER_BS_WORDS+MAX_GANGED_PIXELS];
    unsigned short m_clusterId[MAX_NUMBER_BS_WORDS+MAX_GANGED_PIXELS];
    DECODED_PIX_HIT_INFO m_hitInfo[MAX_NUMBER_BS_WORDS+MAX_GANGED_PIXELS];
    myint4 m_decodedData[MAX_NUMBER_BS_WORDS+MAX_GANGED_PIXELS];
  
    PIXEL_SPACEPOINT_TYPE m_spacePoints[MAX_NUMBER_BS_WORDS+MAX_GANGED_PIXELS]; 
    
    int m_hashToIndex[MAX_PIX_HASH];

  } DECODED_PIXEL_DATA;

  typedef struct DecodedSctHeaderInfo {
    int m_headerPositions;
    int m_rdoEnd;
    short m_hashIds;
    unsigned int m_nClusters;
    bool m_condensedMode;
    
    unsigned int m_nSP;
    unsigned int m_spacePointsStart;
  } DECODED_SCT_HEADER_INFO;
  
  typedef struct SctSpacePointStruct {
    float m_position[3];
    unsigned short m_clusterIdx[2];
  } SCT_SPACEPOINT_TYPE;

  typedef struct DecodedSctData {
    int m_nHeaders;
    int m_nTrailers; 
    int m_nPhiModules;
    unsigned int m_nSpacePoints;
    
    DECODED_SCT_HEADER_INFO m_headersInfo[MAX_SCT_BS_HEADERS];
    unsigned short m_moduleInfoWord[MAX_SCT_BS_HEADERS];
    unsigned short m_headerInfoIndices[MAX_SCT_HASH];//starts from 1 !!!
    unsigned short m_phiModuleIndices[MAX_SCT_BS_HEADERS/2];
    unsigned short m_clusterIds[MAX_NUMBER_BS_WORDS*2];
    float   m_clusterPosition[MAX_NUMBER_BS_WORDS*2];
    SCT_SPACEPOINT_TYPE m_spacePoints[SCT_MAX_SP_PER_MODULE*MAX_SCT_BS_HEADERS/2]; 
    myushort4 m_decodedData[MAX_NUMBER_BS_WORDS*2];
    myushort2 m_clusterInfo[MAX_NUMBER_BS_WORDS*2];
    
  } DECODED_SCT_DATA;
  #endif