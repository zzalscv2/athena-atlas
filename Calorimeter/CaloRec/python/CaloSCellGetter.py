# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

# specifies CaloSCell getting

from AthenaCommon.Logging import logging
from RecExConfig.Configured import Configured

class CaloSCellGetter ( Configured )  :
    #_outputType = "CaloCellContainer"
    #_output = { _outputType : "SCell" } # do not want to list all the IDC
        
    def configure(self):
        mlog = logging.getLogger( 'Py:CaloSCellGetter::configure %s:' % self.__class__ )

        from AthenaCommon.AlgSequence import AlgSequence
        topSequence = AlgSequence()
        from AthenaCommon.AppMgr import ToolSvc


        # get LArDigitGetter in MC case
        from AthenaCommon.DetFlags import DetFlags
        if DetFlags.digitize.LAr_on() :
            try:
                from LArL1Sim.LArSCL1Getter import LArSCL1Getter
                theLArSCL1Getter = LArSCL1Getter()
            except Exception as configException:
                print configException
                mlog.error("could not get handle to LArDigitGetter Quit")
                return False
            if not theLArSCL1Getter.usable():
                mlog.error("LArSCL1Getter unusable. Quite")
                return False

        from LArROD.LArRODFlags import larRODFlags

        from AthenaCommon.GlobalFlags import globalflags

        # ADC2MeV tool
        from LArRecUtils.LArADC2MeVSCToolDefault import LArADC2MeVSCToolDefault
        theADC2MeVSCTool = LArADC2MeVSCToolDefault('LArADC2MeVSCTool')
        ToolSvc += theADC2MeVSCTool


        from AthenaCommon.AppMgr import ServiceMgr as svcMgr

# Only MC case

        if True :

                from CaloRec.CaloRecConf import CaloSuperCellBuilderDriver
                theCaloSCellBuilder=CaloSuperCellBuilderDriver("CaloSuperCellBuilder")

                theCaloSCellBuilder.LArRawChannelContainerName="LArSuperCells"
                theCaloSCellBuilder.DataLocation="LArDigitSCL1"
                theCaloSCellBuilder.AddTileTriggerTowers=True
                self_LArSCellBuilder = theCaloSCellBuilder
                topSequence += theCaloSCellBuilder
                from LArRecUtils.LArRecUtilsConf import LArFlatConditionSvc
                if not hasattr(svcMgr,"LArFlatConditionSvc"):
                   svcMgr+=LArFlatConditionSvc()
                   svcMgr.ProxyProviderSvc.ProviderNames += [ "LArFlatConditionSvc" ]
                svcMgr.LArFlatConditionSvc.DoSuperCells=True

                # Pulse reconstruction
                # main method: OFC iteration
                from LArRecUtils.LArOFCSCToolDefault import LArOFCSCToolDefault
                theLArOFCSCTool = LArOFCSCToolDefault('LArOFCSCToolDefault')
                ToolSvc+=theLArOFCSCTool
                from LArROD.LArRODConf import LArRawChannelBuilderToolOFC
                theLArRawChannelBuilderToolOFC=LArRawChannelBuilderToolOFC('LArRawChannelBuilderSCToolOFC')
                theLArRawChannelBuilderToolOFC.OFCTool=theLArOFCSCTool
                theLArRawChannelBuilderToolOFC.KeyShape = "LArShapeSC"
                ToolSvc+=theLArRawChannelBuilderToolOFC 
                theCaloSCellBuilder.BuilderTools += [theLArRawChannelBuilderToolOFC]
                theCaloSCellBuilder+=theLArRawChannelBuilderToolOFC 

                # no fallback when emulating exactly DSP computation
                # fallback(1): cubic method
                from LArROD.LArRODConf import LArRawChannelBuilderToolCubic
                theLArRawChannelBuilderToolCubic=LArRawChannelBuilderToolCubic('LArRawChannelBuilderSCToolCubic')
                theLArRawChannelBuilderToolCubic.minADCforCubic=2 
                ToolSvc+=theLArRawChannelBuilderToolCubic
                theCaloSCellBuilder.BuilderTools  += [theLArRawChannelBuilderToolCubic]
                theCaloSCellBuilder += theLArRawChannelBuilderToolCubic 


                # Pedestal
                # main method from database
                from LArROD.LArRODConf import LArRawChannelBuilderPedestalDataBase
                theLArRawChannelBuilderPedestalDataBase=LArRawChannelBuilderPedestalDataBase('LArRawChannelBuilderPedestalSCDataBase')
                theLArRawChannelBuilderPedestalDataBase.LArPedestalKey = "LArPedestalSC"
                ToolSvc+=theLArRawChannelBuilderPedestalDataBase
                theCaloSCellBuilder.PedestalTools  = [theLArRawChannelBuilderPedestalDataBase]
                theCaloSCellBuilder += theLArRawChannelBuilderPedestalDataBase 
                
                # no fallback when emulating exactly DSP computation
                # fallback. sample 0
                from LArROD.LArRODConf import LArRawChannelBuilderPedestalSampleZero
                theLArRawChannelBuilderPedestalSampleZero=LArRawChannelBuilderPedestalSampleZero()
                ToolSvc+= theLArRawChannelBuilderPedestalSampleZero
                theCaloSCellBuilder.PedestalTools  += [theLArRawChannelBuilderPedestalSampleZero]
                theCaloSCellBuilder += theLArRawChannelBuilderPedestalSampleZero

                # ADC to energy
                # main method from database
                from LArROD.LArRODConf import LArRawChannelBuilderADC2EDataBase
                theLArRawChannelBuilderADC2EDataBase=LArRawChannelBuilderADC2EDataBase(name='LArRawChannelBuilderADC2ESCDataBase',IsSuperCell=True)
                theLArRawChannelBuilderADC2EDataBase.RampMaxHighGain=6000
                theLArRawChannelBuilderADC2EDataBase.RampMaxMediumGain=49000
                ToolSvc += theLArRawChannelBuilderADC2EDataBase 
                theCaloSCellBuilder.ADCtoEnergyTools  = [theLArRawChannelBuilderADC2EDataBase]
                theLArRawChannelBuilderADC2EDataBase.ADC2MeVTool = theADC2MeVSCTool
                theCaloSCellBuilder += theLArRawChannelBuilderADC2EDataBase 

                # no fallback when emulating exactly DSP computation
                # fallback, constant conversion factors
                from LArROD.LArRODConf import LArRawChannelBuilderADC2EConstants
                theLArRawChannelBuilderADC2EConstants=LArRawChannelBuilderADC2EConstants()
                ToolSvc+=theLArRawChannelBuilderADC2EConstants
                theCaloSCellBuilder.ADCtoEnergyTools += [theLArRawChannelBuilderADC2EConstants]
                theCaloSCellBuilder += theLArRawChannelBuilderADC2EConstants


        return True

# would work only if one output object type
    def CaloSCellBuilder(self):
        return self._CaloSCellBuilder
    def outputKey(cls):
        return cls._output[cls._outputType]

    def outputType(self):
        return cls._outputType

    def outputTypeKey(self):
        return str(self.outputType()+"#"+self.outputKey())
