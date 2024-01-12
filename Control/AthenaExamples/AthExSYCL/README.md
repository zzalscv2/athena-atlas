# SYCL Example Package

This package is meant to hold code demonstrating how to use SYCL directly
from Athena.

Note that building/running SYCL code with/on top of Athena is not super
easy for the time being (19th December 2023). You have to set up a recent
version of [oneAPI](https://www.intel.com/content/www/us/en/developer/tools/oneapi/overview.html)
to make this happen. For instance with something like:

```
asetup Athena,main,latest
source /cvmfs/projects.cern.ch/intelsw/oneAPI/linux/24-all-setup.sh
export SYCLCXX="`which icpx` -fsycl"
```

It is `$SYCLCXX` and `$SYCLFLAGS` that the SYCL build setup respects in how
it should build `.sycl` files.
