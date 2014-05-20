# timer for an executable written in a script style
# Shomiron Ghose (RONNCC)

# is this windows or linux
import time
import argparse
import os
import subprocess
from sys import platform as _platform
from argparse import RawTextHelpFormatter
from sys import argv

# let's first get the program to run from the command line

parser = argparse.ArgumentParser(description=("Find the avg exec time" 
                                " of an executable"),
                                formatter_class=RawTextHelpFormatter,
                                epilog=("Example(s):\n\tpython time.py "
                                        "--ntimes 5 --burnin 5 " 
                                        "--withoutput \n\t"
                                        "python time.py echo 'hi' --withoutput "
                                        ""))
parser.add_argument('prog', nargs="+",
                    help=("the program to time, can be used for arbitrary"
                           " commands as it uses the first word to check "
                           " that the command exists, and the rest is"
                           " passed to the shell (minus redirections)"
                           " which are eaten by the cell"))
parser.add_argument('--ntimes', metavar="N", type=int, default=5,
                    help=("the number of times to run the executable"
                            " when calculating the avg time"))
parser.add_argument('--burnin', metavar="B", type=int, default=10,
                    help=("the number of times to run the executable "
                            "before timing the executable in order to "
                            "warm up the cpu ..."))
parser.add_argument('--withoutput',action='store_true',
                    help=("use the option if you want the output"
                          "everytime the function "
                          "is run, False if you do not"))
args = parser.parse_args()

if( args.ntimes <= 0 or args.burnin <= 0):
    print 'Invalid Arguments Passed for ntimes and burnin'
    exit(1)

def exe_exists(prog):
    def get_path(program):
        def is_exe(fpath):
            return os.path.isfile(fpath) and os.access(fpath, os.X_OK)
        fpath, fname = os.path.split(program)
        if fpath:
            if is_exe(program):
                return program
        else:
            for path in os.environ["PATH"].split(os.pathsep):
                path = path.strip('"')
                exe_file = os.path.join(path,program)
                if is_exe(exe_file):
                    return exe_file
        return None
    path = get_path(prog)
    if path == None:
        return False
    else:
        return os.path.isfile(path)

if not exe_exists(args.prog[0]):
    print 'Invalid or No Program Passed'
    exit(1)

# sort of wish python had switch statements :(
# instead of dictionary 'switches' right now...
timer_func = ""

# set the timer func
if _platform == "win32":
    # we're on windows
    timer_func = time.clock
elif _platform == "linux" or  _platform == "linux2":
    # we're on linux
    timer_func = time.time

if timer_func == "":
    raise OSError("Was not able to set timer function")

# ok now that we have our timing function
# we sequentially run them to try and avoid the scheduler
# causing substantial differences

#if we supress the output
if(args.withoutput):
    err_out = None
    normal_out = None
else:
    err_out = open(os.devnull,'w')
    normal_out = open(os.devnull,'w')

#run the burn-in stage
for x in range(args.burnin):
    p = subprocess.Popen(args.prog,stdout=normal_out, stderr=err_out)
    p.wait() #sequential timing

#now actually time the function
avgsum  = 0
for x in range(args.ntimes):
    start = timer_func()
    p = subprocess.Popen(args.prog,stdout=normal_out, stderr=err_out)
    p.wait() #sequential timing
    avgsum += timer_func() - start

print "Average User Time: ",(1.0 * avgsum) / args.ntimes

