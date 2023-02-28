# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator

def TrigMuonRoIToolCfg(flags):
    tool = CompFactory.TrigMuonRoITool('TrigMuonRoITool')
    tool.DaqMuCTPiROBid = 0x760000
    tool.DecodeMuCTPiFromROB = False
    tool.MUCTPILocation = 'MUCTPI_RDO'
    acc = ComponentAccumulator()
    # TODO: Relying on a Converter to provide MUCTPI_RDO may be not thread safe.
    # Run-3 MUCTPI should provide a decoder algorithm which should be scheduled before any alg using this tool.
    # For now, still using the Run-2 Converter-based solution:
    if tool.MUCTPILocation and not tool.DecodeMuCTPiFromROB:
        rdoType = 'MuCTPI_RDO'

        # Tell SGInputLoader to load MUCTPI_RDO from the input 
        loadFromSG = [( rdoType, 'StoreGateSvc+%s' % tool.MUCTPILocation )]
        from SGComps.SGInputLoaderConfig import SGInputLoaderCfg
        acc.merge(SGInputLoaderCfg(flags, Load=loadFromSG))


        # Enable using the Converter to load MUCTPI_RDO from ByteStream
        if not flags.Input.isMC:
            type_names = [ '%s/%s' % (rdoType, tool.MUCTPILocation) ]
            address_provider = CompFactory.ByteStreamAddressProviderSvc(
                TypeNames=type_names)
            acc.addService(address_provider)

    acc.setPrivateTools(tool)

    return acc
