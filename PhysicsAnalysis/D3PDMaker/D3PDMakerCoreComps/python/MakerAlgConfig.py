# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
#
# @file D3PDMakerCoreComps/python/MakerAlgConfig.py
# @author scott snyder <snyder@bnl.gov>
# @date Dec 2023, from old config
# @brief Configure algorithm for making a D3PD tree.
#


from AthenaConfiguration.ComponentFactory import CompFactory
from D3PDMakerCoreComps.D3PDObject import D3PDObject
from D3PDMakerConfig.D3PDMakerFlags import D3PDMakerFlags

D3PD = CompFactory.D3PD


class MakerAlg:
    def __init__ (self, name, flags, acc, registry, *args, **kw):
        self.alg = D3PD.MakerAlg (name, *args, **kw)
        self.registry = registry
        self.flags = flags
        self.acc = acc
        return


    def __iadd__( self, configs ):
        """Add a new IObjFillerTool to a tree."""

        # FIXME: should make sure name is unique within alg.
        nchild = len (self.alg.Tools)
        if not isinstance(configs, list):
            configs = [configs]
        self.alg.Tools += configs

        for c in self.alg.Tools[nchild:]:
            # Scan children to set the proper collection getter registry.
            self._setRegistry (c)

            D3PDObject.runHooks (c, self.flags, self.acc)
        return self


    def _setRegistry (self, conf):
        """Scan CONF and all children to set the proper
        collection getter registry for this tree.
"""
        
        if 'CollectionGetterRegistry' in conf.getDefaultProperties():
            conf.CollectionGetterRegistry = self.registry
        if 'BlockFillers' in conf.getDefaultProperties():
            for c in conf.BlockFillers:
                self._setRegistry (c)
                D3PDObject.runHooks (c, self.flags, self.acc)
        if 'Getter' in conf.getDefaultProperties():
            self._setRegistry (conf.Getter)
        if 'SelectionGetter' in conf.getDefaultProperties():
            self._setRegistry (conf.SelectionGetter)
        if 'Associator' in conf.getDefaultProperties():
            self._setRegistry (conf.Associator)
        return
    

def MakerAlgConfig (flags, acc, stream, file,
                    clevel = D3PDMakerFlags.CompressionLevel,
                    autoflush = D3PDMakerFlags.AutoFlush,
                    ExistDataHeader = True, **kw):
    """Configure algorithm for making a D3PD tree.

    Each distinct D3PD tree is make by a separate algorithm.
    This function is used to configure these algorithms.

    Arguments:

      flags: The configuration flags.
      name: The name of the algorithm (required).
      stream: Athena stream for the tuple.
      file: Name of the file containing the tuple.
            If it starts with `pool:', then the tree is being emitted
            into a POOL file.  In that case, stream is just the name
            of the tree.
      clevel: Compresson level for the output ROOT file. By default it is
              controlled by the D3PDMakerFlags.CompressionLevel flag, but
              can be controlled D3PD-by-D3PD as well.
      autoflush: Allows overriding the global autoflush setting.
"""



    
    name = stream + 'D3PDMaker'
    tuplename = stream

    if file.startswith ('pool:'):
        acc.addService (D3PD.RootD3PDSvc (AutoFlush = autoflush,
                                          IndexMajor = '',
                                          IndexMinor = ''))

        TuplePath = f'{file}/{tuplename}'

    else:

        acc.addService (CompFactory.THistSvc (Output = [
            f"{stream} DATAFILE='{file}' OPT='RECREATE' CL={clevel}"]))

        acc.addService (D3PD.RootD3PDSvc (AutoFlush = autoflush))

        st = CompFactory.AANTupleStream (stream,
                                         ExtraRefNames = ['StreamESD',
                                                          'StreamRDO',
                                                          'StreamAOD'],
                                         OutputName = file,
                                         ExistDataHeader = ExistDataHeader,
                                         WriteInputDataHeader = True,
                                         StreamName = stream)
        acc.addEventAlgo (st)

        TuplePath = f'/{stream}/{tuplename}'


    registry = D3PD.CollectionGetterRegistryTool (name + '_CollectionGetterRegistry')
    acc.addPublicTool (registry)

    acc.addEventAlgo (D3PD.DummyInitAlg (name + 'DummyInit'))

    alg = MakerAlg (name, flags, acc,
                    registry = registry,
                    TuplePath = TuplePath,
                    **kw)

    return alg
