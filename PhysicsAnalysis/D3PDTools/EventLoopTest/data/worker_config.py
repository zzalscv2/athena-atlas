from AnaAlgorithm.DualUseConfig import createAlgorithm

def fillWorkerConfig (config) :
    alg = createAlgorithm ('EL::UnitTestAlg7', 'myalg')
    config.add (alg)
