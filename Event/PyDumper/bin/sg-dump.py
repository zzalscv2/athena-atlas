#!/usr/bin/env python

# Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
# @file: PyDumper/bin/sg-dump.py
# @purpose: a simple python script to run pyathena and use PySgDumper to dump
#           a (set of) event(s) from a POOL (esd/aod) file into an ASCII file
# @author: Sebastien Binet <binet@cern.ch>
# @date:   October 2008
#
# @example:
# @verbatim
# sg-dump aod.pool -o aod.ascii
# sg-dump /castor/cern.ch/foo.pool -o foo.ascii
# sg-dump aod1.pool aod2.pool -o aods.ascii
# @endcode

from __future__ import with_statement
import sys
import os

__version__ = "$Revision: 1.4 $"
__author__  = "Sebastien Binet <binet@cern.ch>"
__doc__ = """\
a simple python script to run pyathena and use PySgDumper to dump
a (set of) event(s) from a POOL (esd/aod) file into an ASCII file\
"""

from optparse import OptionParser

import PyDumper.SgDumpLib as SgDumpLib

def _str_to_slice (slice_str):
    """translate a string into a python slice
    """
    sl= [int(s) if s != '' else None for s in slice_str.split(':')]
    if len(sl)==2:
        sl.append (None)
    assert len(sl)==3, 'invalid input slice string'
    start = sl[0]
    stop  = sl[1]
    step  = sl[2]
    return slice (start, stop, step)

if __name__ == "__main__":
    parser = OptionParser(
        usage="usage: %prog [options] -o out.ascii in1.pool "\
              "[in2.pool [in3.pool [...]]]"
        )
    _add = parser.add_option
    _add("-o",
         "--output",
         dest = "oname",
         default = None,
         help = "Name of the output file which will contain the informations"\
                " gathered during athena processing. These informations "\
                "will be stored into an ascii file.")

    _add('--evts',
         dest    = 'evts',
         type    = int,
         default = -1,
         help    = "list of events or event-max or (python) slice of events."\
                   " (dummy for now: only understand event-max syntax)")

    _add('--skip',
         dest    = 'skip',
         type    = int,
         default = 0,
         help    = "number of events to skip.")

    _add("--dump-jobo",
         dest    = "dump_jobo",
         default = None,
         help    = "tell application to save the automatically generated "\
                   "joboption under some name for (mainly) debugging "\
                   "and/or customization purposes.")

    _add("--do-clean-up",
         dest    = "do_clean_up",
         action  = "store_true",
         default = True,
         help    = "switch to enable the attempt at removing all the (temporary) files sg-dump produces during the course of its execution")

    _add("--no-clean-up",
         dest    = "do_clean_up",
         action  = "store_false",
         help    = "switch to enable the attempt at removing all the (temporary) files sg-dump produces during the course of its execution")

    _add("--athena-opts",
         dest = "athena_opts",
         default = None,
         help = "space-separated list of athena command-line options. "\
         "these will be passed to the validation job. (e.g. "\
         "'--perfmon --stdcmalloc')" )

    _add("--pyalg",
         dest     = "pyalg_cls",
         default  = "PyDumper.PyComps:PySgDumper",
         help     = "name of the class to use to process the file(s) content (default: '%default'. validation uses: 'PyDumper.PyComps:DataProxyLoader')")

    _add("--include",
         dest = "include",
         default = "*",
         help = "comma-separated list of type#key containers to dump (default: all)")

    _add("--exclude",
         dest = "exclude",
         default = "",
         help = "comma-separated list of glob patterns of keys/types to ignore")
         
    _add("--conditions-tag",
         dest = "conditions_tag",
         default = "",
         help = "override setting of global conditions tag")
         
    _add("--full-log",
         dest = "full_log",
         action = "store_true",
         default = False,
         help = "preserve full log file")
         
    (options, args) = parser.parse_args()

    input_files = []
    
    from AthenaCommon.Logging import logging
    msg = logging.getLogger ('sg-dumper')
    msg.setLevel (logging.INFO)

    input_files=[]
    if len(args) > 0:
        input_files = [arg for arg in args if arg[0] != "-"]
        pass

    if len(input_files) == 0 or options.oname is None:
        parser.print_help()
        msg.error('please provide an output filename and '
                  'at least one input file')
        raise SystemExit(1)

    sc = 1
    try:
        sc, out = SgDumpLib.run_sg_dump(
            files=input_files,
            output=options.oname,
            nevts=int(options.evts),
            skip=int(options.skip),
            dump_jobo=options.dump_jobo,
            pyalg_cls=options.pyalg_cls,
            include=options.include,
            exclude=options.exclude,
            do_clean_up=options.do_clean_up,
            athena_opts=options.athena_opts,
            conditions_tag = options.conditions_tag,
            full_log = options.full_log,
            msg=msg
            )
    except Exception as err:
        msg.error('problem while running sg-dump:\n%s', err)
        import traceback
        traceback.print_exc()
    sys.exit(sc)
