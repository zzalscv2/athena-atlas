# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaPython.PyAthenaComps import Alg,StatusCode
import csv
import ROOT
import os
import math
from InDetMeasurementUtilities.CSV_DictFormats import CSV_DictFormats

def getCSVFilename(outputDir, container, eventNumber):
    return f"{outputDir}/{container}_event_{eventNumber}.csv"

class CSV_InDetImporterAlg(Alg):
    """
    Algorithm to load InDet tracks data from CSV files to xAOD collections
    Conventions in files naming:
     - the files should reside in one dir and alg needs to be pointed to it through indir property, the naming convention: container_event_XYZ.csv should be used
     - files CSV format needs to be identical to the one used in export
     - if a given file is missing the data is not loaded, just reporting in the log about it
     - If need to modify this behavior, hey, go for it, it is python
    """
    def __init__(self, name):
        Alg.__init__ (self, name)
        self.indir = None
        self.trackParticleName = None
        return

    def initialize(self):
        if self.indir is None:
            self.msg.error("Input directory not configured")
            return StatusCode.Failure
        if not os.path.exists(self.indir):
            self.msg.error("Missing input directory %s", self.indir)
            return StatusCode.Failure

        return StatusCode.Success

    def execute(self):
        if self.trackParticleName:
            if not self.readTrackParticle():
                return StatusCode.Failure

        return StatusCode.Success

    def getEventNumber(self):
        ei = self.evtStore.retrieve("xAOD::EventInfo", "EventInfo")
        return ei.eventNumber()

    def readTrackParticle(self):
        inputFileName = getCSVFilename(self.indir, self.trackParticleName, self.getEventNumber())
        if not os.path.exists(inputFileName):
            self.msg.warning("Missing file %s, this will result in missing collections in certain events which is not allowed by POOL &ROOT  ", inputFileName)
            return StatusCode.Recoverable


        c = ROOT.xAOD.TrackParticleContainer()
        aux = ROOT.xAOD.TrackParticleAuxContainer()
        c.setStore (aux)
        ROOT.SetOwnership (c,False)
        ROOT.SetOwnership (aux,False)

        # data reading
        with open(inputFileName, "r") as f:
            reader = csv.DictReader(f)
            for k in reader.fieldnames:
                if k not in CSV_DictFormats["InDetTrackParticles"].keys():
                    self.msg.error("A key: %s found in data that does not seem to be known", k)
                    return StatusCode.Failure

            for data in reader:
                tp = ROOT.xAOD.TrackParticle()
                c.push_back(tp)
                ROOT.SetOwnership (tp, False)
                covm = ROOT.xAOD.ParametersCovMatrix_t()
                covm.setZero()
                theta = 2.0 * math.atan( math.exp(-float(data['eta'])))
                tp.setDefiningParameters(float(data['d0']), float(data['z0']), float(data['phi']),
                                         theta,
                                         float(data['charge']) * math.sin(theta)/float(data['pt']))
                tp.setDefiningParametersCovMatrix(covm)
                pass

        if not self.evtStore.record (c, self.trackParticleName, False):
            return StatusCode.Failure
        if not self.evtStore.record (aux, f'{self.trackParticleName}Aux.', False):
            return StatusCode.Failure

        return StatusCode.Success


def CSV_InDetImporterCfg(flags, indir, trackParticleName):
    from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
    from OutputStreamAthenaPool.OutputStreamConfig import addToESD, addToAOD

    ca = ComponentAccumulator()

    algo = CSV_InDetImporterAlg("CSV_InDetImporter")
    algo.indir = indir
    if trackParticleName:
        algo.trackParticleName = trackParticleName
        output = [f'xAOD::TrackParticleContainer#{trackParticleName}', f'xAOD::TrackParticleAuxContainer#{trackParticleName}Aux.']
        algo.ExtraOutput = output[:1]
        ca.merge(addToESD(flags, output))
        ca.merge(addToAOD(flags, output))

    ca.addEventAlgo(algo)
    return ca


if __name__ == '__main__':
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    flags = initConfigFlags()
    flags.addFlag('CSVInputDir','testdir')
    flags.addFlag("TrackParticlesName",'NewTP')
    flags.Exec.MaxEvents=2
    flags.fillFromArgs()
    flags.Output.AODFileName="outAOD.pool.root"
    flags.lock()

    acc=MainServicesCfg(flags)
    # if need to read POOL file
    from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
    acc.merge(PoolReadCfg(flags))

    acc.merge(CSV_InDetImporterCfg(flags, indir=flags.CSVInputDir, trackParticleName=flags.TrackParticlesName))

    acc.printConfig(withDetails=True)
    # either
    status = acc.run()
    if status.isFailure():
        import sys
        sys.exit(-1)
