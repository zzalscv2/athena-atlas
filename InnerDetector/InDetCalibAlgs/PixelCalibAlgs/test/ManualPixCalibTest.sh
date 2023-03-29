#!/bin/bash


#primitive manual test to enable further development
#assumes you have an output.dat to compare
#assumes you are running from this directory
root -b -q ../Macro/ExtractSampleData.C
pushd ../ChargeCalibration/pixel
root -b -q "PixelCalib.C(true)" >output.new
diff output.new output.dat
if [ $? -eq 0 ]
then
  popd
  exit 0  # success
else
  popd
  exit 1  # error
fi


