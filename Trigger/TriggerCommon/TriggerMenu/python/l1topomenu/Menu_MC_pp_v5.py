# Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration

def defineMenu():
    """
    Defines the following LlTopoFlags:
    
    algos   .... list of all algo names in the menu 

    """
    
    from TriggerMenu.l1topo.L1TopoFlags import L1TopoFlags
    from TriggerMenu.l1topo.TopoOutput import TopoOutput

    full_menu = True

    M8_topomenu = [
        TopoOutput( algoname='INVM_AJ',  module=0, fpga=0, clock=0, firstbit=0 ),
        TopoOutput( algoname='INVM_J',   module=0, fpga=0, clock=0, firstbit=4 ),
        TopoOutput( algoname='HT',       module=0, fpga=0, clock=0, firstbit=8 ),
        TopoOutput( algoname='HT1-AJ0all.ETA49',      module=0, fpga=0, clock=0, firstbit=10 ),
        TopoOutput( algoname='INVM_EMs', module=0, fpga=0, clock=0, firstbit=11 ),
        TopoOutput( algoname='0DETA10-Js1-Js2',       module=0, fpga=0, clock=0, firstbit=14 ),

        # MODULE 0
        # B phys INVM
        TopoOutput( algoname='2INVM999-2MU4ab',                             module=0, fpga=1, clock=0, firstbit=0 ), # M9
        TopoOutput( algoname='2INVM999-MU6ab-MU4ab',                        module=0, fpga=1, clock=0, firstbit=1 ), # M9
        TopoOutput( algoname='2INVM999-2MU6ab',                             module=0, fpga=1, clock=0, firstbit=2 ), # M9
        TopoOutput( algoname='4INVM8-2MU4ab',                               module=0, fpga=1, clock=0, firstbit=3 ), # M9
        TopoOutput( algoname='4INVM8-MU6ab-MU4ab',                          module=0, fpga=1, clock=0, firstbit=4 ), # M9
        TopoOutput( algoname='4INVM8-2MU6ab',                               module=0, fpga=1, clock=0, firstbit=5 ), # M9

        # SM Upsilon DR                                                     
        TopoOutput( algoname='2DR99-2MU4ab',                                module=0, fpga=1, clock=0, firstbit=6 ), # M9
        TopoOutput( algoname='5DETA99-5DPHI99-2MU4ab',                      module=0, fpga=1, clock=0, firstbit=7 ), # M9
        TopoOutput( algoname='5DETA99-5DPHI99-MU6ab-MU4ab',                 module=0, fpga=1, clock=0, firstbit=8 ), # M9
        TopoOutput( algoname='5DETA99-5DPHI99-2MU6ab',                      module=0, fpga=1, clock=0, firstbit=9 ), # M9
                                                                            
        # Exotic LFV DR                                                     
        TopoOutput( algoname='0DR10-MU10ab-MU6ab',                          module=0, fpga=1, clock=0, firstbit=10 ), # M9        
        TopoOutput( algoname='2DR15-2MU4ab',                                module=0, fpga=1, clock=0, firstbit=11 ), # M9 
        TopoOutput( algoname='2DR15-2MU6ab',                                module=0, fpga=1, clock=0, firstbit=12 ), # M9
        TopoOutput( algoname='0DR28-TAU20abi-TAU12abi',                     module=0, fpga=1, clock=0, firstbit=13 ), # M9

        TopoOutput( algoname='2DR15-MU6ab-MU4ab',                           module=0, fpga=1, clock=0, firstbit=14 ), # M9

        TopoOutput( algoname='0DR28-MU10ab-TAU12abi',                       module=0, fpga=1, clock=0, firstbit=15 ), # M9

        ]       
    
    M7_topomenu = [
        TopoOutput( algoname='DPhi_AJsAJs',           module=1, fpga=1, clock=1, firstbit=7 ),
        TopoOutput( algoname='DPhi_EMsTAUs',          module=1, fpga=1, clock=1, firstbit=8 ),
        TopoOutput( algoname='DEta_AJabAJab',         module=1, fpga=1, clock=1, firstbit=9 ),
        TopoOutput( algoname='DEta_EMabTAUab',        module=1, fpga=1, clock=1, firstbit=10 ),
        ]

    Full_topomenu = [

        TopoOutput( algoname='HT0-AJ0all.ETA49',      module=0, fpga=0, clock=0, firstbit=15 ),
        
        TopoOutput( algoname='2INVM999-CMU4ab-MU4ab',                       module=0, fpga=1, clock=1, firstbit=0 ), 
        TopoOutput( algoname='2INVM999-2CMU4ab',                            module=0, fpga=1, clock=1, firstbit=1 ), 
        TopoOutput( algoname='2INVM999-ONEBARREL-MU6ab-MU4ab',              module=0, fpga=1, clock=1, firstbit=2 ), 
        TopoOutput( algoname='2INVM999-CMU6ab-CMU4ab',                      module=0, fpga=1, clock=1, firstbit=3 ), 
        TopoOutput( algoname='4INVM8-CMU4ab-MU4ab',                         module=0, fpga=1, clock=1, firstbit=4 ), 
        TopoOutput( algoname='4INVM8-2CMU4ab',                              module=0, fpga=1, clock=1, firstbit=5 ),
        TopoOutput( algoname='4INVM8-ONEBARREL-MU6ab-MU4ab',                module=0, fpga=1, clock=1, firstbit=6 ),
        TopoOutput( algoname='4INVM8-CMU6ab-CMU4ab',                        module=0, fpga=1, clock=1, firstbit=7 ),

        TopoOutput( algoname='0DETA04-0DPHI03-EM8abi-MU10ab',               module=0, fpga=1, clock=1, firstbit=8 ),
        TopoOutput( algoname='0DETA04-0DPHI03-EM15abi-MUab',                module=0, fpga=1, clock=1, firstbit=9 ),
                                                                            
        # MuonJet                                                           
        TopoOutput( algoname='0DR04-MU4ab-CJ15ab',                          module=0, fpga=1, clock=1, firstbit=10 ), # fpga 1 clock 0
        TopoOutput( algoname='0DR04-MU4ab-CJ30ab',                          module=0, fpga=1, clock=1, firstbit=11 ),
        TopoOutput( algoname='0DR04-MU6ab-CJ25ab',                          module=0, fpga=1, clock=1, firstbit=12 ),
        TopoOutput( algoname='0DR04-MU4ab-CJ17ab',                          module=0, fpga=1, clock=1, firstbit=13 ),
        TopoOutput( algoname='0DR04-MU4ab-CJ20ab',                          module=0, fpga=1, clock=1, firstbit=14 ),
                                                                            
        # ZH
        TopoOutput( algoname='10MINDPHI-Js2-XE50',                          module=1, fpga=1, clock=1, firstbit=0 ),
        TopoOutput( algoname='10MINDPHI-J20s2-XE50',                        module=1, fpga=1, clock=1, firstbit=6 ),


        TopoOutput( algoname='10MINDPHI-J20ab-XE50',                        module=1, fpga=1, clock=1, firstbit=3 ),

        TopoOutput( algoname='10MINDPHI-CJ20ab-XE50',                       module=0, fpga=1, clock=1, firstbit=15 ),
                                                                            
        # VBF INVM
        TopoOutput( algoname='INVM_AJ_HighMass',                            module=1, fpga=0, clock=0, firstbit=7 ), # need 4bits
        TopoOutput( algoname='INVM_AJ_LowMass',                             module=1, fpga=0, clock=0, firstbit=11 ),
        TopoOutput( algoname='INVM_AJ_VLowMass',                            module=1, fpga=1, clock=0, firstbit=12 ),

        # HT                                                                
        TopoOutput( algoname='HT190-AJ15all.ETA20',                         module=1, fpga=0, clock=0, firstbit=0 ),
        TopoOutput( algoname='HT190-J15s5.ETA20',                           module=1, fpga=0, clock=0, firstbit=1 ),
        TopoOutput( algoname='HT150-AJ20all.ETA30',                         module=1, fpga=0, clock=0, firstbit=2 ),
        TopoOutput( algoname='HT150-J20s5.ETA30',                           module=1, fpga=0, clock=0, firstbit=3 ),
        TopoOutput( algoname='HT150-AJj15all.ETA49',                        module=1, fpga=0, clock=0, firstbit=4 ),
        TopoOutput( algoname='HT20-AJj15all.ETA49',                         module=1, fpga=0, clock=0, firstbit=5 ), # we temporary have a problem in the TMC with different clock on same cable
        #TopoOutput( algoname='HT20-AJj0all.ETA49',                          module=1, fpga=0, clock=0, firstbit=5 ), # we temporary have a problem in the TMC with different clock on same cable
        #TopoOutput( algoname='HT0-AJ0all.ETA49',                           module=1, fpga=0, clock=0, firstbit=9 ),
                                                                            
        # JetMatch                                                          
        TopoOutput( algoname='0MATCH-4AJ20.ETA32-4AJj15',                   module=1, fpga=0, clock=0, firstbit=6 ),
                                                                            
        # J/Psi T&P                                                         
        TopoOutput( algoname='1INVM5-EM7s2-EMall',                          module=1, fpga=1, clock=1, firstbit=11 ),
        TopoOutput( algoname='INVM_EMall',                                  module=1, fpga=1, clock=1, firstbit=12 ), # 3bits

        # MODULE 1


        # W T&P MINDPHI(J, XE)
        TopoOutput( algoname='05MINDPHI-AJj20s6-XE0',                       module=1, fpga=0, clock=1, firstbit=0 ),
        TopoOutput( algoname='10MINDPHI-AJj20s6-XE0',                       module=1, fpga=0, clock=1, firstbit=1 ),
        TopoOutput( algoname='15MINDPHI-AJj20s6-XE0',                       module=1, fpga=0, clock=1, firstbit=2 ),

        # NOT-MATCH, RATIO, SUM                                             
        TopoOutput( algoname='NOT-02MATCH-EM10s1-AJj15all.ETA49',           module=1, fpga=0, clock=1, firstbit=3 ),
        TopoOutput( algoname='05RATIO-XE0-SUM0-EM10s1-HT0-AJj15all.ETA49',  module=1, fpga=0, clock=1, firstbit=4 ),
                                                                                    
        # MINDPHI(EM, XE)                                                   
        TopoOutput( algoname='10MINDPHI-EM10s6-XE0',                        module=1, fpga=0, clock=1, firstbit=5 ),
        TopoOutput( algoname='15MINDPHI-EM10s6-XE0',                        module=1, fpga=0, clock=1, firstbit=6 ),
        TopoOutput( algoname='05MINDPHI-EM15s6-XE0',                        module=1, fpga=0, clock=1, firstbit=7 ),
        
        # MT 
        TopoOutput( algoname='25MT-EM10s6-XE0',                             module=1, fpga=0, clock=1, firstbit=8 ),
        TopoOutput( algoname='30MT-EM10s6-XE0',                             module=1, fpga=0, clock=1, firstbit=9 ),
        TopoOutput( algoname='35MT-EM15s6-XE0',                             module=1, fpga=0, clock=1, firstbit=10 ),
                
        # RATIO (XE(^2), HT)                                                
        TopoOutput( algoname='05RATIO-XE0-HT0-AJj15all.ETA49',              module=1, fpga=0, clock=1, firstbit=11 ), 
        TopoOutput( algoname='08RATIO-XE0-HT0-AJj15all.ETA49',              module=1, fpga=0, clock=1, firstbit=12 ), 
        TopoOutput( algoname='90RATIO2-XE0-HT0-AJj15all.ETA49',             module=1, fpga=0, clock=1, firstbit=13 ),
        TopoOutput( algoname='250RATIO2-XE0-HT0-AJj15all.ETA49',            module=1, fpga=0, clock=1, firstbit=14 ),

        # Extotic dedicated                                                 
        TopoOutput( algoname='210RATIO-0MATCH-TAU30si2-EMall',              module=1, fpga=0, clock=1, firstbit=15 ), 
        TopoOutput( algoname='NOT-0MATCH-TAU30si2-EMall',                   module=0, fpga=0, clock=1, firstbit=0 ), 
                                                                            
        # TAU DISAMB, DR                                                    
        TopoOutput( algoname='1DISAMB-TAU12abi-J25ab',                      module=1, fpga=1, clock=0, firstbit=1 ),
        TopoOutput( algoname='DISAMB-EM15abhi-TAU40ab',                     module=1, fpga=1, clock=0, firstbit=2 ),
        TopoOutput( algoname='1DISAMB-TAU20ab-J20ab',                       module=1, fpga=1, clock=0, firstbit=3 ),
        #                                                                   
        #DISAMB-0DR28-EM15abhi-TAU12abi                                     
        TopoOutput( algoname='DISAMB-EM15abhi-TAU12abi',                    module=1, fpga=1, clock=0, firstbit=4 ),
        TopoOutput( algoname='1DISAMB-EM15his2-TAU12abi-J25ab',             module=1, fpga=1, clock=0, firstbit=5 ), # 1DISAMB-EM15his2-TAU12abi-J25ab ?
        TopoOutput( algoname='1DISAMB-J25ab-0DR28-EM15his2-TAU12abi',       module=1, fpga=1, clock=0, firstbit=6 ),
        #                                                                   
        TopoOutput( algoname='1DISAMB-TAU20abi-TAU12abi-J25ab',             module=1, fpga=1, clock=0, firstbit=7 ),

        TopoOutput( algoname='0DETA20-0DPHI20-TAU20abi-TAU12abi',           module=1, fpga=1, clock=0, firstbit=9 ),
        TopoOutput( algoname='1DISAMB-J25ab-0DR28-TAU20abi-TAU12abi',       module=1, fpga=1, clock=0, firstbit=10 ), 
                                                                            
        # LAR                                                               
        TopoOutput( algoname='LAR-EM50s1',                                  module=1, fpga=1, clock=0, firstbit=11 ),
        TopoOutput( algoname='LAR-J100s1',                                  module=1, fpga=0, clock=0, firstbit=15 ),
                                                                            
        # B phys DR                                                         
        TopoOutput( algoname='2DR15-CMU4ab-MU4ab',                          module=1, fpga=1, clock=1, firstbit=1 ),
        TopoOutput( algoname='2DR15-2CMU4ab',                               module=1, fpga=1, clock=1, firstbit=2 ),
        TopoOutput( algoname='2DR15-ONEBARREL-MU6ab-MU4ab',                 module=1, fpga=1, clock=1, firstbit=4 ),
        TopoOutput( algoname='2DR15-CMU6ab-CMU4ab',                         module=1, fpga=1, clock=1, firstbit=5 ),

        TopoOutput( algoname='MULT-CMU4ab', module=0, fpga=0, clock=1, firstbit=1 ), # 2bits
        TopoOutput( algoname='MULT-CMU6ab', module=0, fpga=0, clock=1, firstbit=3 ), # 2bits
        
        ]

    L1TopoFlags.algos = M8_topomenu 
    
    if full_menu:
        L1TopoFlags.algos += M7_topomenu 
        L1TopoFlags.algos += Full_topomenu
