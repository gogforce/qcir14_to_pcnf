'''
Created on Apr 27, 2019

@author: Georgi Marinov, gogmarinov@gmail.com

This script runs a QCIR-to-QDIMACS translation tool over the file
contents of a directory.

You need to specify the path to the tool, the input and output directories,
as well as if you want execution per instance to timeout after a certain
amount of seconds.

A 'processing_summary' file will be generated in output directory,
containing statistics of the whole run.
'''

from os import listdir
from os.path import isfile, join
import subprocess
import resource
import time


if __name__ == '__main__':
    toolpath = "" # Provide path to tool
    path = "" # Use if input and output folders are in the same directory
    rpath = path + "" # This will be the path to read from
    wpath = path + "" # This will be the path to write output files
    onlyfiles = sorted([f for f in listdir(rpath) if isfile(join(rpath, f))], key=str.lower)
    tout = None # if you desire execution per instance to terminate after a certain amount of seconds, set it here, otherwise leave None
    successes = 0
    timeouts = 0
    summary = open(wpath + "processing_summary", "w")
    subprocess_total_time = sum(resource.getrusage(resource.RUSAGE_CHILDREN)[:2])
    timeStarted = time.time() 
    for file in onlyfiles:
        fpath = rpath + "/" + file
        o = open(wpath + file + ".qdimacs", "w")
        try:
            subprocess.run([toolpath, fpath], stdout=o, timeout=tout)
            new_total_time = sum(resource.getrusage(resource.RUSAGE_CHILDREN)[:2])
            used_time = new_total_time - subprocess_total_time
            subprocess_total_time = new_total_time
            summary.write(file + " processed in " + str(round(used_time, 3)) + "\n")
            successes += 1
        except subprocess.TimeoutExpired:
            subprocess_total_time = sum(resource.getrusage(resource.RUSAGE_CHILDREN)[:2])
            o.write("Timeout after " + str(tout) + " seconds.")
            summary.write(file + " timed out after " + str(tout) + " seconds.\n")
            timeouts += 1
        o.close()
    timeDelta = time.time() - timeStarted
    summary.write("\n------------------\nSuccesses: " + str(successes) + "\nTimeouts: " + str(timeouts) + "\nTotal: " + str(successes + timeouts) + "\nTotal time of this script: " + str(round(timeDelta, 3)))
    summary.close