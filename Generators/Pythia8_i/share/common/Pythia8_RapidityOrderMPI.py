## this fragment makes sure that the MPI rapidity order is set consistently for Pythia versions 8.219 and later
## Since it depends on the tune settings it must be included *after* the main tune fragment in the JO

addRapidityOrderMPI = True
rapidityOrderMPICommand = []

for cmd in genSeq.Pythia8.Commands:
  
  if "SpaceShower:rapidityOrderMPI = " in cmd:
    addRapidityOrderMPI = False
  
  if "SpaceShower:rapidityOrder" in cmd and "SpaceShower:rapidityOrderMPI" not in cmd and addRapidityOrderMPI:
    
    val = cmd.split("=")[-1]

    rapidityOrderMPICommand = ["SpaceShower:rapidityOrderMPI = " + val]

if addRapidityOrderMPI and len(rapidityOrderMPICommand) != 0:
  genSeq.Pythia8.Commands += rapidityOrderMPICommand
