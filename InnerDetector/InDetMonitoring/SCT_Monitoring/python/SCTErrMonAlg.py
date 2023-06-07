#
#  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
#

'''@file SCTErrMonAlg_jobOptions.py
@author Susumu Oda
@date 2020-10-08
@brief New style configuration of SCTErrMonAlg
'''


def SCTErrMonAlgConfig(flags):

    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    result = ComponentAccumulator()

    from AthenaMonitoring import AthMonitorCfgHelper
    helper = AthMonitorCfgHelper(flags, 'SCTErrMonCfg')

    from AthenaConfiguration.ComponentFactory import CompFactory
    myMonAlg = helper.addAlgorithm(CompFactory.SCTErrMonAlg, 'SCTErrMonAlg')
    myMonAlg.TriggerChain = ""

    from SCT_ConditionsTools.SCT_ConditionsToolsConfig import SCT_ConditionsSummaryToolCfg
    myMonAlg.SCT_ConditionsSummaryTool = result.popToolsAndMerge(
        SCT_ConditionsSummaryToolCfg(flags))

    # Pass the InDet.useDCS flag to the algorithm
    myMonAlg.UseDCS = flags.InDet.useDCS

    myMonAlg.doOnlineMon = flags.Common.isOnline

    from LumiBlockComps.BunchCrossingCondAlgConfig import BunchCrossingCondAlgCfg
    result.merge(BunchCrossingCondAlgCfg(flags))

    myMonGroup = helper.addGroup(myMonAlg, "SCTErrMonitor", "SCT/")

    # Configure histograms

    from ROOT import SCT_Monitoring as sctMon  # import SCT_MonitoringNumbers.h

    # Filled in fillHistograms
    myMonGroup.defineHistogram(varname="lumiBlock;NumberOfEventsVsLB",
                               cutmask="is1D",
                               type="TH1F",
                               title="Num of events per LB ;LumiBlock",
                               path="GENERAL/Conf",
                               xbins=sctMon.NBINS_LBs,
                               xmin=0.5,
                               xmax=sctMon.NBINS_LBs+0.5,
                               opt='kAlwaysCreate')

    # Filled in fillHistograms
    myMonGroup.defineHistogram(varname="lumiBlock;NumberOfSCTFlagErrorsVsLB",
                               cutmask="sctFlag",
                               type="TH1F",
                               title="Num of SCT Flag errors per LB ;LumiBlock",
                               path="GENERAL/Conf",
                               xbins=sctMon.NBINS_LBs,
                               xmin=0.5,
                               xmax=sctMon.NBINS_LBs+0.5,
                               opt='kAlwaysCreate')

    # Filled in fillHistograms
    myMonGroup.defineHistogram(varname="lumiBlock, sctFlag;FractionOfSCTFlagErrorsPerLB",
                               type="TProfile",
                               title="Frac of SCT Flag errors per LB ;LumiBlock",
                               path="GENERAL/Conf",
                               xbins=sctMon.NBINS_LBs,
                               xmin=0.5,
                               xmax=sctMon.NBINS_LBs+0.5,
                               opt='kAlwaysCreate')

    # Filled in fillConfigurationDetails
    myMonGroup.defineHistogram(varname="detailedConfBin, nBad;SCTConfDetails",
                               type="TProfile",
                               title="Exclusion from the Configuration",
                               path="GENERAL/Conf",
                               xbins=sctMon.ConfbinsDetailed,
                               xmin=-0.5,
                               xmax=sctMon.ConfbinsDetailed-0.5,
                               xlabels=["Modules", "Link 0", "Link 1",
                                        "Chips", "Strips (10^{2})"],
                               opt='kAlwaysCreate')

    # Filled in fillHistograms
    myMonGroup.defineHistogram(varname="moduleOutBin, moduleOut;SCTConfOutM",
                               type="TProfile",
                               title="Num of Out Modules in All Region",
                               path="GENERAL/Conf",
                               xbins=1,
                               xmin=-0.5,
                               xmax=0.5,
                               xlabels=["Mod Out"],
                               opt='kAlwaysCreate')

    # Fiiled in fillByteStreamErrors
    from ROOT import SCT_ByteStreamErrors
    for i in range(SCT_ByteStreamErrors.ErrorType.NUM_ERROR_TYPES):
        myMonGroup.defineHistogram(
            varname="lumiBlock, n_" +
            SCT_ByteStreamErrors.ErrorTypeDescription[i] +
            ";SCT_" +
            SCT_ByteStreamErrors.ErrorTypeDescription[i] +
            "VsLbs",
            type="TProfile",
            title="Ave. " +
            SCT_ByteStreamErrors.ErrorTypeDescription[i] +
            " per LB in All Region;LumiBlock;Num of " +
            SCT_ByteStreamErrors.ErrorTypeDescription[i],
            path="GENERAL/Conf",
            xbins=sctMon.NBINS_LBs,
            xmin=0.5,
            xmax=sctMon.NBINS_LBs+0.5,
            opt='kAlwaysCreate')

    # Fiiled in fillByteStreamErrors
    for i in range(sctMon.N_ERRCATEGORY):
        myMonGroup.defineHistogram(
            varname="lumiBlock, n_" +
            sctMon.CategoryErrorsNames[i]+";SCT_LinksWith" +
            sctMon.CategoryErrorsNames[i]+"VsLbs",
            type="TProfile",
            title="Ave. Num of Links with " +
            sctMon.CategoryErrorsNames[i] +
            " per LB in All Region;LumiBlock;Num of Links with " +
            sctMon.CategoryErrorsNames[i],
            path="GENERAL/Conf",
            xbins=sctMon.NBINS_LBs,
            xmin=0.5,
            xmax=sctMon.NBINS_LBs+0.5,
            opt='kAlwaysCreate')

    # Filled in fillByteStreamErrors
    for errCate in range(sctMon.N_ERRCATEGORY):
        for region in range(sctMon.N_REGIONS):
            for layer in range(sctMon.N_ENDCAPSx2):
                myMonGroup.defineHistogram(
                    varname="eta, phi, hasError_" +
                    sctMon.CategoryErrorsNames[errCate]+"_" +
                    sctMon.subDetNameShort[region].Data()+"_" +
                    str(layer//2)+"_"+str(layer % 2) +
                    ";SCT_NumberOf"+sctMon.CategoryErrorsNames[errCate] +
                    sctMon.subDetNameShort[region].Data()+"_" +
                    str(layer//2)+"_"+str(layer % 2),
                    type="TProfile2D",
                    title="Num of " +
                    sctMon.CategoryErrorsNames[errCate]+" per "+sctMon.layerName[region].Data()+str(
                        layer//2)+"_"+str(layer % 2),
                    path="SCT" +
                    sctMon.subDetNameShort[region].Data() +
                    "/errors/"+sctMon.CategoryErrorsNames[errCate],
                    xbins=sctMon.N_ETA_BINS if region == sctMon.BARREL_INDEX else sctMon.N_ETA_BINS_EC,
                    xmin=(
                        sctMon.FIRST_ETA_BIN if region == sctMon.BARREL_INDEX else sctMon.FIRST_ETA_BIN_EC)-0.5,
                    xmax=(
                        sctMon.LAST_ETA_BIN if region == sctMon.BARREL_INDEX else sctMon.LAST_ETA_BIN_EC)+0.5,
                    ybins=sctMon.N_PHI_BINS if region == sctMon.BARREL_INDEX else sctMon.N_PHI_BINS_EC,
                    ymin=(
                        sctMon.FIRST_PHI_BIN if region == sctMon.BARREL_INDEX else sctMon.FIRST_PHI_BIN_EC)-0.5,
                    ymax=(
                        sctMon.LAST_PHI_BIN if region == sctMon.BARREL_INDEX else sctMon.LAST_PHI_BIN_EC)+0.5,
                    duration="lb",
                    opt='kAlwaysCreate')

                if myMonAlg.doOnlineMon and sctMon.CategoryErrorsNames[errCate] == "Errors":
                    myMonGroup.defineHistogram(
                        varname="eta, phi, hasError_" +
                        sctMon.CategoryErrorsNames[errCate]+"_recent_" +
                        sctMon.subDetNameShort[region].Data()+"_" +
                        str(layer//2)+"_"+str(layer % 2)+";SummaryErrsRecent_" +
                        sctMon.subDetNameShort[region].Data()+"_" +
                        str(layer//2)+"_"+str(layer % 2),
                        type="TProfile2D",
                        title="Num of " +
                        sctMon.CategoryErrorsNames[errCate] +
                        " per "+sctMon.layerName[region].Data()
                        + str(layer//2)+"_"+str(layer % 2)+" - recent",
                        path="SCT" +
                        sctMon.subDetNameShort[region].Data() +
                        "/errors",
                        xbins=sctMon.N_ETA_BINS if region == sctMon.BARREL_INDEX else sctMon.N_ETA_BINS_EC,
                        xmin=(
                            sctMon.FIRST_ETA_BIN if region == sctMon.BARREL_INDEX else sctMon.FIRST_ETA_BIN_EC)-0.5,
                        xmax=(
                            sctMon.LAST_ETA_BIN if region == sctMon.BARREL_INDEX else sctMon.LAST_ETA_BIN_EC)+0.5,
                        ybins=sctMon.N_PHI_BINS if region == sctMon.BARREL_INDEX else sctMon.N_PHI_BINS_EC,
                        ymin=(
                            sctMon.FIRST_PHI_BIN if region == sctMon.BARREL_INDEX else sctMon.FIRST_PHI_BIN_EC)-0.5,
                        ymax=(
                            sctMon.LAST_PHI_BIN if region == sctMon.BARREL_INDEX else sctMon.LAST_PHI_BIN_EC)+0.5,
                        duration="lowStat",
                        opt='kAlwaysCreate')

    # Filled in fillByteStreamErrorsHelper
    myMonGroup.defineHistogram(varname="maskedLinksBin;Masked Links",
                               weight="maskedLinks",
                               type="TProfile",
                               title="Number of Masked Links for SCT,ECA,B,ECC",
                               path="GENERAL/errors",
                               xbins=sctMon.N_REGIONS_INC_GENERAL,
                               xmin=-0.5,
                               xmax=sctMon.N_REGIONS_INC_GENERAL-0.5,
                               xlabels=["EndCapC", "Barrel", "EndCapA", "All"],
                               opt='kAlwaysCreate')

    # Filled in fillHistograms
    myMonGroup.defineHistogram(varname="flaggedWafersIndices, nFlaggedWafers;FlaggedWafers",
                               type="TProfile",
                               title="Number of flagged wafers for SCT,ECA,B,ECC",
                               path="GENERAL/errors",
                               xbins=sctMon.N_REGIONS_INC_GENERAL,
                               xmin=-0.5,
                               xmax=sctMon.N_REGIONS_INC_GENERAL-0.5,
                               xlabels=["EndCapC", "Barrel", "EndCapA", "All"],
                               opt='kAlwaysCreate')

    # Filled in fillByteStreamErrors
    coverageTitles = [
        "",  # All (not used)
        "Ave. Coverage of Enabled Links per LB",  # All - Disabled
        "Ave. Coverage of Links with No Bad LinkLevelError per LB",  # All - BadLinkLevelError
        "Ave. Coverage of Links with No Bad RODLevelError per LB",  # All - BadRODLevelError
        "Ave. Coverage of Links with No Bad Error per LB",  # All - BadError
        "Ave. Coverage of links Not Affected by PS Trip",  # All - PSTrip (DCS)
        "Ave. Coverage of Links With No Bad Problem per LB"  # All - Summary
    ]
    for iProblem in range(1, sctMon.numberOfProblemForCoverage):
        myMonGroup.defineHistogram(
            varname="lumiBlock, detectorCoverage" +
            sctMon.coverageVarNames[iProblem]+";SCT_Coverage" +
            sctMon.coverageVarNames[iProblem]+"VsLbs",
            type="TProfile",
            title=coverageTitles[iProblem] +
            ";LumiBlock;Detector Coverage [%]",
            path="DetectorCoverage",
            xbins=sctMon.NBINS_LBs,
            xmin=0.5,
            xmax=sctMon.NBINS_LBs+0.5,
            opt='kAlwaysCreate')

    # Fiiled in fillByteStreamErrors
    myMonGroup.defineHistogram(varname="lumiBlock, psTripModules;SCT_ModulesWithPSTripVsLbs",
                               type="TProfile",
                               title="Ave. Num of Modules Affected by PS Trip per LB in All Region;LumiBlock;Num. of Modules Affected by PS Trip",
                               path="DetectorCoverage",
                               xbins=sctMon.NBINS_LBs,
                               xmin=0.5,
                               xmax=sctMon.NBINS_LBs+0.5,
                               opt='kAlwaysCreate')

    # Module maps
    for region in range(sctMon.N_REGIONS):
        for layer in range(sctMon.n_layers[region]*2):
            myMonGroup.defineHistogram(
                varname="eta_out, phi_out, modulemap" +
                sctMon.subDetNameShort[region].Data()+str(layer//2)+"_" +
                str(layer % 2)+";modulemap" +
                sctMon.subDetNameShort[region].Data()+str(layer//2)+"_" +
                str(layer % 2),
                type="TProfile2D",
                title="Module out of configuration: "+sctMon.layerName[region].Data()+str(layer//2)+" side "+str(
                    layer % 2) + ";Index in the direction of #eta;Index in the direction of #phi",
                path="SCT" +
                sctMon.subDetNameShort[region].Data()+"/Conf/",
                xbins=sctMon.N_ETA_BINS if region == sctMon.BARREL_INDEX else sctMon.N_ETA_BINS_EC,
                xmin=(sctMon.FIRST_ETA_BIN if region ==
                      sctMon.BARREL_INDEX else sctMon.FIRST_ETA_BIN_EC)-0.5,
                xmax=(
                    sctMon.LAST_ETA_BIN if region == sctMon.BARREL_INDEX else sctMon.LAST_ETA_BIN_EC)+0.5,
                ybins=sctMon.N_PHI_BINS if region == sctMon.BARREL_INDEX else sctMon.N_PHI_BINS_EC,
                ymin=(sctMon.FIRST_PHI_BIN if region ==
                      sctMon.BARREL_INDEX else sctMon.FIRST_PHI_BIN_EC)-0.5,
                ymax=(
                    sctMon.LAST_PHI_BIN if region == sctMon.BARREL_INDEX else sctMon.LAST_PHI_BIN_EC)+0.5,
                duration="lb",
                opt='kAlwaysCreate')

    # Filled in fillByteStreamErrorsHelper
    xlabels = [SCT_ByteStreamErrors.ErrorTypeDescription[i]
               for i in range(SCT_ByteStreamErrors.ErrorType.NUM_ERROR_TYPES)]
    for reg in range(sctMon.N_REGIONS):
        nLayers = sctMon.n_layers[reg]*2
        ylabels = [str(i//2)+"_"+str(i % 2) for i in range(nLayers)]
        myMonGroup.defineHistogram(varname="errorType, layerSide, errorFraction;RateErrorsPerLumi",
                                   cutmask="is" +
                                   sctMon.subDetNameShort[reg].Data(),
                                   type="TProfile2D",
                                   title="Rate of Error Types for " +
                                   sctMon.layerName[reg].Data() +
                                   " per Lumi-Block",
                                   path="SCT" +
                                   sctMon.subDetNameShort[reg].Data(
                                   )+"/errors",
                                   xbins=SCT_ByteStreamErrors.ErrorType.NUM_ERROR_TYPES,
                                   xmin=-0.5,
                                   xmax=SCT_ByteStreamErrors.ErrorType.NUM_ERROR_TYPES-0.5,
                                   xlabels=xlabels,
                                   ybins=nLayers,
                                   ymin=-0.5,
                                   ymax=nLayers-0.5,
                                   ylabels=ylabels,
                                   duration="lb",
                                   opt='kAlwaysCreate')

    result.merge(helper.result())
    return result
