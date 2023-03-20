#g++ -O2 'root-config --libs --cflags' CalibrateIBL.C -o CalibrateIBL.exe

g++ -O2 -L/cvmfs/atlas-nightlies.cern.ch/repo/sw/master_Athena_x86_64-centos7-gcc11-opt/sw/lcg/releases/LCG_101_ATLAS_24/ROOT/6.24.06a/x86_64-centos7-gcc11-opt/lib -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -pthread -lm -ldl -rdynamic -pthread -std=c++17 -m64 -I/cvmfs/atlas-nightlies.cern.ch/repo/sw/master_Athena_x86_64-centos7-gcc11-opt/sw/lcg/releases/LCG_101_ATLAS_24/ROOT/6.24.06a/x86_64-centos7-gcc11-opt/include ../common/PixelMapping.cxx CalibrateIBL.C -o CalibrateIBL.exe

