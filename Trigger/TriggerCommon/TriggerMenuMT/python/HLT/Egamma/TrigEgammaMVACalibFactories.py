# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

__doc__ = """Tool and service factories to instantiate MVA calibration"""

from egammaRec.Factories import ToolFactory, ServiceFactory

from egammaMVACalib import egammaMVACalibConf
from xAODEgamma.xAODEgammaParameters import xAOD


def trigPrecCaloEgammaMVASvc(flags):
   """Calibration service for precision calo step"""
   electronTool = ToolFactory(
      egammaMVACalibConf.egammaMVACalibTool,
      name="trigPrecCaloElectronMVATool",
      ParticleType=xAOD.EgammaParameters.electron,
      folder=flags.Trigger.egamma.Calib.precCaloMVAVersion)

   convPhotonTool = ToolFactory(
      egammaMVACalibConf.egammaMVACalibTool,
      name="trigPrecCaloConvPhotonMVATool",
      ParticleType=xAOD.EgammaParameters.convertedPhoton,
      folder=flags.Trigger.egamma.Calib.precCaloMVAVersion)

   unconvPhotonTool = ToolFactory(
      egammaMVACalibConf.egammaMVACalibTool,
      name="trigPrecCaloUnconvPhotonMVATool",
      ParticleType=xAOD.EgammaParameters.unconvertedPhoton,
      folder=flags.Trigger.egamma.Calib.precCaloMVAVersion)

   return ServiceFactory(
      egammaMVACalibConf.egammaMVASvc,
      name="trigPrecCaloEgammaMVASvc",
      ElectronTool=electronTool,
      ConvertedPhotonTool=convPhotonTool,
      UnconvertedPhotonTool=unconvPhotonTool)


def trigPrecEgammaMVASvc(flags):
   """Calibration service for precision electron and photon step"""
   electronTool = ToolFactory(
      egammaMVACalibConf.egammaMVACalibTool,
      name="trigPrecElectronMVATool",
      ParticleType=xAOD.EgammaParameters.electron,
      folder=flags.Trigger.egamma.Calib.precEgammaMVAVersion)

   convPhotonTool = ToolFactory(
      egammaMVACalibConf.egammaMVACalibTool,
      name="trigPrecConvPhotonMVATool",
      ParticleType=xAOD.EgammaParameters.convertedPhoton,
      folder=flags.Trigger.egamma.Calib.precEgammaMVAVersion)

   unconvPhotonTool = ToolFactory(
      egammaMVACalibConf.egammaMVACalibTool,
      name="trigPrecUnconvPhotonMVATool",
      ParticleType=xAOD.EgammaParameters.unconvertedPhoton,
      folder=flags.Trigger.egamma.Calib.precEgammaMVAVersion)

   return ServiceFactory(
      egammaMVACalibConf.egammaMVASvc,
      name="trigPrecEgammaMVASvc",
      ElectronTool=electronTool,
      UnconvertedPhotonTool=unconvPhotonTool,
      ConvertedPhotonTool=convPhotonTool)
