#!/usr/bin/env python3

import sys
import subprocess

sys.path.append("@CMAKE_SOURCE_DIR@/bin")
from madjsoncompare import madjsoncompare



def localizer_run(localizer):
    prefix='madtest'+localizer

    # prefix='mad_@BINARY@_@TESTCASE@'
    outputfile=prefix+'.calc_info.json'
    referencefile="@SRCDIR@/"+prefix+".calc_info.ref.json"


    cmd='rm '+outputfile+'; ./@BINARY@ --geometry=h2o --dft="maxiter=10; econv=3.e-5; k=8; localize='+localizer+'; dconv=1.e-3; prefix='+prefix+'"'
    print("executing \n ",cmd)
#    output=subprocess.run(cmd,shell=True,capture_output=True, text=True).stdout
    p=subprocess.run(cmd,shell=True,stdout=subprocess.PIPE, stderr=subprocess.PIPE , universal_newlines=True)

    print("finished with run1")

    cmp=madjsoncompare(outputfile,referencefile)
    cmp.compare(["return_energy"],1.e-4)
    return cmp.success

if __name__ == "__main__":

    # some user output
    print("Testing @BINARY@/@TESTCASE@")
    print(" reference files found in directory: @SRCDIR@")


    success=localizer_run('canon')
    success=localizer_run('boys') and success
    success=localizer_run('new') and success

    print("final success: ",success)
    ierr=0
    if (not success):
        ierr=1
    sys.exit(ierr)