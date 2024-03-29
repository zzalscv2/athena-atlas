# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

def SingleToolToPlot(tool_name, prefix):
    return (tool_name, prefix)

def ComparedToolsToPlot(tool_ref, tool_test, prefix, match_in_energy = False, match_without_shared = False, match_perfectly = False):
    return (tool_ref, tool_test, prefix, match_in_energy, match_without_shared, match_perfectly)

class PlotterConfigurator:
    #DoCells currently changes nothing
    #(originally was intended to show
    #that we had cells with the same energy),
    #keeping it here to remain part of the interface
    #if/when we port the cell maker as well.
    def __init__ (self, StepsToPlot = [], PairsToPlot = [], DoStandard = True, DoMoments = False, DoCells = False):
        self.PlotsToDo = []
        if DoStandard:
            for step in StepsToPlot:
                self.PlotsToDo += [
                                    ( (step + "_cluster_E",),
                                      {'type': 'TH1F',
                                       'title': "Cluster Energy; E [MeV]; Number of Events",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_Et",),
                                      {'type': 'TH1F',
                                       'title': "Cluster Transverse Energy; E_T [MeV]; Number of Events",
                                       'xbins':  63,
                                       'xmin':  -5020,
                                       'xmax':   5020,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_eta",),
                                      {'type': 'TH1F',
                                       'title': "Cluster #eta; #eta; Number of Events",
                                       'xbins':  84,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_phi",),
                                      {'type': 'TH1F',
                                       'title': "Cluster #phi; #phi; Number of Events",
                                       'xbins':  61,
                                       'xmin':  -3.25,
                                       'xmax':   3.25,
                                       'path': "EXPERT"}
                                    ),
                                    ( (step + "_cluster_eta," + step + "_cluster_phi",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #eta versus #phi; #eta; #phi",
                                       'xbins':  84,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  61,
                                       'ymin':  -3.25,
                                       'ymax':   3.25,
                                       'path': "EXPERT"}
                                    )
                                  ]
            for pair in PairsToPlot:
                self.PlotsToDo += [
                                    ( (pair + "_num_unmatched_clusters",),
                                      {'type': 'TH1F',
                                       'title': "Number of Unmatched Clusters; # of Unmatched Clusters; Number of Events",
                                       'xbins':  21,
                                       'xmin':  -0.5,
                                       'xmax':   20.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_diff_cells;" + pair + "_cluster_diff_cells_zoom_0",),
                                      {'type': 'TH1F',
                                       'title': "Different Cell Assignments; # of Differently Assigned Cells; Number of Clusters",
                                       'xbins':  21,
                                       'xmin':  -25,
                                       'xmax':   1025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_diff_cells;" + pair + "_cluster_diff_cells_zoom_1",),
                                      {'type': 'TH1F',
                                       'title': "Different Cell Assignments; # of Differently Assigned Cells; Number of Clusters",
                                       'xbins':  21,
                                       'xmin':  -12.5,
                                       'xmax':   512.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_diff_cells;" + pair + "_cluster_diff_cells_zoom_2",),
                                      {'type': 'TH1F',
                                       'title': "Different Cell Assignments; # of Differently Assigned Cells; Number of Clusters",
                                       'xbins':  21,
                                       'xmin':  -2.5,
                                       'xmax':   102.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_diff_cells;" + pair + "_cluster_diff_cells_zoom_3",),
                                      {'type': 'TH1F',
                                       'title': "Different Cell Assignments; # of Differently Assigned Cells; Number of Clusters",
                                       'xbins':  21,
                                       'xmin':  -0.5,
                                       'xmax':   20.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_E_ref," + pair + "_cluster_E_test",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Energy Comparison; E^{(CPU)} [MeV]; E^{(GPU)} [MeV]",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -50.5,
                                       'ymax':   50.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_E_ref," + pair + "_cluster_delta_E_rel_ref",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Energy Resolution; E^{(CPU)} [MeV]; #Delta E / #(){E^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_Et_ref," + pair + "_cluster_Et_test",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Transverse Energy Comparison; E_T^{(CPU)} [MeV]; E_T^{(GPU)} [MeV]",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -50.5,
                                       'ymax':   50.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_Et_ref," + pair + "_cluster_delta_Et_rel_ref",),
                                      {'type': 'TH2F',
                                       'title': "Cluster Transverse Energy Resolution; E_T^{(CPU)} [MeV]; #Delta E_T / #(){E_T^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -50.5,
                                       'xmax':   50.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_eta_ref," + pair + "_cluster_eta_test",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #eta Comparison; #eta^{(CPU)}; #eta^{(GPU)}",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -10.5,
                                       'ymax':   10.5,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_eta_ref," + pair + "_cluster_delta_eta_rel_ref",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #eta Resolution; #eta^{(CPU)}; #Delta #eta / #(){#eta^{(CPU)}}",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_phi_ref," + pair + "_cluster_phi_test",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #phi Comparison; #phi^{(CPU)}; #phi^{(GPU)}",
                                       'xbins':  63,
                                       'xmin':  -3.3,
                                       'xmax':   3.3,
                                       'ybins':  63,
                                       'ymin':  -3.3,
                                       'ymax':   3.3,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_phi_ref," + pair + "_cluster_delta_phi_in_range",),
                                      {'type': 'TH2F',
                                       'title': "Cluster #phi Resolution; #phi^{(CPU)}; #Delta #phi",
                                       'xbins':  63,
                                       'xmin':  -10.5,
                                       'xmax':   10.5,
                                       'ybins':  63,
                                       'ymin':  -0.025,
                                       'ymax':   0.025,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cluster_delta_phi_in_range",),
                                      {'type': 'TH1F',
                                       'title': "Cluster #phi; #phi; Number of Clusters",
                                       'xbins':  61,
                                       'xmin':  -3.25,
                                       'xmax':   3.25,
                                       'path': "EXPERT"}
                                    ),
                                    ( (pair + "_cell_secondary_weight_ref," + pair + "_cell_secondary_weight_test",),
                                      {'type': 'TH2F',
                                       'title': "Shared Cell Secondary Weight Comparison; w_2^{(CPU)}; w_2^{(GPU)}",
                                       'xbins': 51,
                                       'xmin':  -0.05,
                                       'xmax':  0.505,
                                       'ybins': 51,
                                       'ymin':  -0.05,
                                       'ymax':  0.505,
                                       'path': "EXPERT"}
                                    )
                                  ]
        if DoMoments:
            for pair in PairsToPlot:
                for mom, (prettynames, limits) in name_to_moment_map.items():
                    for i in range(0, len(limits)):
                      self.PlotsToDo += [ ( (pair + "_cluster_moments_" + mom + "_ref," + pair + "_cluster_moments_" + mom + "_test;" + pair + "_" + mom + "_versus_zoom_" + str(i + 1),),
                                            {'type':  'TH2F',
                                             'title': prettynames[0] + " (" + str(i+1) + "); CPU " + prettynames[1] + "; GPU " + prettynames[1],
                                             'xbins': 63,
                                             'xmin':  limits[i][0],
                                             'xmax':  limits[i][1],
                                             'ybins': 63,
                                             'ymin':  limits[i][0],
                                             'ymax':  limits[i][1],
                                             'path':  "EXPERT"},
                                          ),
                                        ]
                      self.PlotsToDo += [ ( (pair + "_cluster_moments_" + mom + "_ref," + pair + "_cluster_delta_moments_" + mom + "_rel_ref;" + pair + "_" + mom + "_error_zoom_" + str(i + 1),),
                                            {'type':   'TH2F',
                                             'title': prettynames[0] + " (" + str(i+1) + "); CPU " + prettynames[1] + "; #Delta " + prettynames[1] + " / #(){CPU " + prettynames[1] + "}",
                                             'xbins':  63,
                                             'xmin':   limits[i][0],
                                             'xmax':   limits[i][1],
                                             'ybins':  63,
                                             'ymin':  -0.025,
                                             'ymax':   0.025,
                                             'path':   "EXPERT"}
                                          ),
                                        ]
                      self.PlotsToDo += [ ( (pair + "_cluster_delta_moments_" + mom + "_rel_ref;" + pair + "_cluster_delta_moments_" + mom + "_rel_ref_zoom_0",),
                                            {'type': 'TH1F',
                                             'title': prettynames[0] + "; #Delta " + prettynames[1] + "; Number of Clusters",
                                             'xbins': 51,
                                             'xmin':  -1,
                                             'xmax':  1,
                                             'path': "EXPERT"}
                                           ),
                                        ]
    def __call__(self, Plotter):
        for plotdef in self.PlotsToDo:
            Plotter.MonitoringTool.defineHistogram(*plotdef[0], **plotdef[1])
        return Plotter

#For pretty printing things in axes when it comes to moments:
#<MOMENT_NAME>: <PLOT TITLE> <AXIS TITLE> <UNITS>
name_to_moment_map =  {
   #"time"                        :  (("time",                "time",               "[#mu s]"),[]),
    "FIRST_PHI"                   :  (("firstPhi",            "firstPhi",            ""),[(-3.2, 3.2)]),
    "FIRST_ETA"                   :  (("firstEta",            "firstEta",            ""),[(-10.1, 10.1)]),
    "SECOND_R"                    :  (("secondR",             "secondR",             ""),[(0, 1.25e6)]),
    "SECOND_LAMBDA"               :  (("secondLambda",        "secondLambda",        ""),[(0, 2.5e6)]),
    "DELTA_PHI"                   :  (("deltaPhi",            "deltaPhi",            ""),[(-3.2, 3.2)]),
    "DELTA_THETA"                 :  (("deltaTheta",          "deltaTheta",          ""),[(-1.6, 1.6)]),
    "DELTA_ALPHA"                 :  (("deltaAlpha",          "deltaAlpha",          ""),[(-0.1, 1.6)]),
    "CENTER_X"                    :  (("centerX",             "centerX",             ""),[(-4000, 4000)]),
    "CENTER_Y"                    :  (("centerY",             "centerY",             ""),[(-4000, 4000)]),
    "CENTER_Z"                    :  (("centerZ",             "centerZ",             ""),[(-7000, 7000)]),
    "CENTER_MAG"                  :  (("centerMag",           "centerMag",           ""),[(1000, 7500)]),
    "CENTER_LAMBDA"               :  (("centerLambda",        "centerLambda",        ""),[(0., 25000.),(0., 5000.)]),
    "LATERAL"                     :  (("lateral",             "lateral",             ""),[(-0.05, 1.05)]),
    "LONGITUDINAL"                :  (("longitudinal",        "longitudinal",        ""),[(-0.05, 1.05)]),
    "ENG_FRAC_EM"                 :  (("engFracEM",           "engFracEM",           ""),[(-0.1, 1.1)]),
    "ENG_FRAC_MAX"                :  (("engFracMax",          "engFracMax",          ""),[(-0.1, 1.1)]),
    "ENG_FRAC_CORE"               :  (("engFracCore",         "engFracCore",         ""),[(-0.1, 1.1)]),
    "FIRST_ENG_DENS"              :  (("firstEngDens",        "firstEngDens",        ""),[(-0.1, 5.1)]),
    "SECOND_ENG_DENS"             :  (("secondEngDens",       "secondEngDens",       ""),[(-0.5, 50.5)]),
    "ISOLATION"                   :  (("isolation",           "isolation",           ""),[(-0.05, 2.05)]),
    "ENG_BAD_CELLS"               :  (("engBadCells",         "engBadCells",         ""),[(0., 100000.)]),
    "N_BAD_CELLS"                 :  (("nBadCells",           "nBadCells",           ""),[(-0.5, 25.5)]),
    "N_BAD_CELLS_CORR"            :  (("nBadCellsCorr",       "nBadCellsCorr",       ""),[(-0.5, 25.5)]),
    "BAD_CELLS_CORR_E"            :  (("badCellsCorrE",       "badCellsCorrE",       ""),[(-0.1, 25000.)]),
    "BADLARQ_FRAC"                :  (("badLArQFrac",         "badLArQFrac",         ""),[(-2500., 2500.)]),
    "ENG_POS"                     :  (("engPos",              "engPos",              ""),[(-0.1, 250000.)]),
    "SIGNIFICANCE"                :  (("significance",        "significance",        ""),[(-500., 500.)]),
    "CELL_SIGNIFICANCE"           :  (("cellSignificance",    "cellSignificance",    ""),[(-0.1, 100.)]),
    "CELL_SIG_SAMPLING"           :  (("cellSigSampling",     "cellSigSampling",     ""),[(-0.1, 30.)]),
    "AVG_LAR_Q"                   :  (("avgLArQ",             "avgLArQ",             ""),[(-0.1, 70000.)]),
    "AVG_TILE_Q"                  :  (("avgTileQ",            "avgTileQ",            ""),[(-0.1, 300.)]),
   #"ENG_BAD_HV_CELLS"            :  (("engBadHVCells",       "engBadHVCells",       ""),[]),
   #"N_BAD_HV_CELLS"              :  (("nBadHVCells",         "nBadHVCells",         ""),[]),
    "PTD"                         :  (("PTD",                 "PTD",                 ""),[(-0.05, 1.05)]),
    "MASS"                        :  (("mass",                "mass",                ""),[(0., 200000.), (0., 100.)]),
   #"EM_PROBABILITY"              :  (("EMProbability",       "EMProbability",       ""),[]),
   #"HAD_WEIGHT"                  :  (("hadWeight",           "hadWeight",           ""),[]),
   #"OOC_WEIGHT"                  :  (("OOCweight",           "OOCweight",           ""),[]),
   #"DM_WEIGHT"                   :  (("DMweight",            "DMweight",            ""),[]),
   #"TILE_CONFIDENCE_LEVEL"       :  (("tileConfidenceLevel", "tileConfidenceLevel", ""),[]),
    "SECOND_TIME"                 :  (("secondTime",          "secondTime",          ""),[(0., 1e7)])
   #"number_of_cells"             :  (("numCells",            "numCells",            ""),[]),
   #"VERTEX_FRACTION"             :  (("vertexFraction",      "vertexFraction",      ""),[]),
   #"NVERTEX_FRACTION"            :  (("nVertexFraction",     "nVertexFraction",     ""),[]),
   #"ETACALOFRAME"                :  (("etaCaloFrame",        "etaCaloFrame",        ""),[]),
   #"PHICALOFRAME"                :  (("phiCaloFrame",        "phiCaloFrame",        ""),[]),
   #"ETA1CALOFRAME"               :  (("eta1CaloFrame",       "eta1CaloFrame",       ""),[]),
   #"PHI1CALOFRAME"               :  (("phi1CaloFrame",       "phi1CaloFrame",       ""),[]),
   #"ETA2CALOFRAME"               :  (("eta2CaloFrame",       "eta2CaloFrame",       ""),[]),
   #"PHI2CALOFRAME"               :  (("phi2CaloFrame",       "phi2CaloFrame",       ""),[]),
   #"ENG_CALIB_TOT"               :  (("engCalibTot",         "engCalibTot",         ""),[]),
   #"ENG_CALIB_OUT_L"             :  (("engCalibOutL",        "engCalibOutL",        ""),[]),
   #"ENG_CALIB_OUT_M"             :  (("engCalibOutM",        "engCalibOutM",        ""),[]),
   #"ENG_CALIB_OUT_T"             :  (("engCalibOutT",        "engCalibOutT",        ""),[]),
   #"ENG_CALIB_DEAD_L"            :  (("engCalibDeadL",       "engCalibDeadL",       ""),[]),
   #"ENG_CALIB_DEAD_M"            :  (("engCalibDeadM",       "engCalibDeadM",       ""),[]),
   #"ENG_CALIB_DEAD_T"            :  (("engCalibDeadT",       "engCalibDeadT",       ""),[]),
   #"ENG_CALIB_EMB0"              :  (("engCalibEMB0",        "engCalibEMB0",        ""),[]),
   #"ENG_CALIB_EME0"              :  (("engCalibEME0",        "engCalibEME0",        ""),[]),
   #"ENG_CALIB_TILEG3"            :  (("engCalibTileG3",      "engCalibTileG3",      ""),[]),
   #"ENG_CALIB_DEAD_TOT"          :  (("engCalibDeadTot",     "engCalibDeadTot",     ""),[]),
   #"ENG_CALIB_DEAD_EMB0"         :  (("engCalibDeadEMB0",    "engCalibDeadEMB0",    ""),[]),
   #"ENG_CALIB_DEAD_TILE0"        :  (("engCalibDeadTile0",   "engCalibDeadTile0",   ""),[]),
   #"ENG_CALIB_DEAD_TILEG3"       :  (("engCalibDeadTileG3",  "engCalibDeadTileG3",  ""),[]),
   #"ENG_CALIB_DEAD_EME0"         :  (("engCalibDeadEME0",    "engCalibDeadEME0",    ""),[]),
   #"ENG_CALIB_DEAD_HEC0"         :  (("engCalibDeadHEC0",    "engCalibDeadHEC0",    ""),[]),
   #"ENG_CALIB_DEAD_FCAL"         :  (("engCalibDeadFCAL",    "engCalibDeadFCAL",    ""),[]),
   #"ENG_CALIB_DEAD_LEAKAGE"      :  (("engCalibDeadLeakage", "engCalibDeadLeakage", ""),[]),
   #"ENG_CALIB_DEAD_UNCLASS"      :  (("engCalibDeadUnclass", "engCalibDeadUnclass", ""),[]),
   #"ENG_CALIB_FRAC_EM"           :  (("engCalibFracEM",      "engCalibFracEM",      ""),[]),
   #"ENG_CALIB_FRAC_HAD"          :  (("engCalibFracHad",     "engCalibFracHad",     ""),[]),
   #"ENG_CALIB_FRAC_REST"         :  (("engCalibFracRest"     "engCalibFracRest"     ""),[])
}
