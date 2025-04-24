import sys
import os
import functools

class Job:
    def __init__(self):
        self.id = 0         # job number
        self.release = 0    # job release date (ignored)
        self.due = 0        # job due date (ignored)
        self.weight = 0     # job weight
        self.proc = 1       # job processing time
        self.wp = 0         # w_j / p_j

    def __repr__(self):
        return f"Job(id={self.id}, release={self.release}, due={self.due}, weight={self.weight}, proc={self.proc}, W/P={self.wp})"

# For the single machine setting, we don't even have to read the
# machine file.  But I'll do it to check that it actually represents
# a single machine setting.

def readMch():
    # No need to use the qualified path.  Just "_user.mch".
    mchFileName = "_user.mch"

    # Check if the file exists
    # If not, it is an error: probably we were called
    # standalone, NOT from LEKIN.
    # Our way of handling errors is exit(1).
    # It is a good idea to print something before exitting.

    if not os.path.isfile(mchFileName):
        print("Machine file not found!")
        exit(1)

    Fmch = open(mchFileName, "r")

    # Check the first line in the file.
    # If it is not "Ordinary:", too bad.

    buffer = Fmch.readline().strip()
    if buffer != "Ordinary:":
        print("We do not support flexible workcenters!")
        exit(1)

    # Now we skip several lines.

    buffer = Fmch.readline().strip()
    # buffer = "Workcenter:        Wkc000",
    # but we don't care.

    buffer = Fmch.readline().strip()
    # buffer = "Setup:"

    buffer = Fmch.readline().strip()
    # buffer = "Machine:            Wkc000.000"

    # we don't need the avaiability time and the starting status
    # for the machine, but I'll read it just to show how it's done.

    buffer = Fmch.readline().strip()
    # buffer = "Release: ###"
    avail = int(buffer.split()[1])

    buffer = Fmch.readline().strip()
    # buffer = "Status: #"
    status = buffer.split()[1]

    # Let's check status -- just in case...
    if len(status) != 1 or status < "A" or status > "Z":
        print("The file is corrupt, bad status!")
        exit(1)

    # OK, now the rest of the file must contain nothing but
    # a bunch of white-space characters.

    while True:
        buffer = Fmch.readline()
        if buffer == "":
            break
        if buffer.strip() != "":
            print("The file contains at least 2 workcenters!")
            exit(1)

    Fmch.close()
#################################################

def checkAndRead(file, prefix):
    buffer = file.readline().strip()
    if buffer.split()[0] != prefix:
        print("The file is corrupt, \"", prefix, "\" expected!")
        exit(1)
    return int(buffer.split()[1])
#################################################


# With the job file, it is not that easy.
# We actually have to read the stream of jobs.

def readJob():
    jobFileName = "_user.job";
    if not os.path.isfile(jobFileName):
        print("Job file not found!")
        exit(1)

    Fjob = open(jobFileName, "r")
    jobList = []

    buffer = Fjob.readline().strip()
    # buffer = "Shop: xxx"
    # check if single machine
    if buffer.split()[1] != "Single":
        print("This is not a single machine file!")
        exit(1)

    while True:
        buffer = Fjob.readline()
        if buffer == "":
            break

        ## there may be an empty line between jobs
        buffer = buffer.strip()
        if buffer == "":
            continue

        # buffer = "Job:  Job###"

        if buffer.split()[0] != "Job:":
            print("The file is corrupt, \"Job\" expected!")
            exit(1)

        job = Job()
        job.id = len(jobList)

        # "Release: ###"
        job.release = checkAndRead(Fjob, "Release:")

        # "Due: ###"
        job.due = checkAndRead(Fjob, "Due:")

        # "Weight: ###"
        job.weight = checkAndRead(Fjob, "Weight:")

        buffer = Fjob.readline().strip()
        # buffer = "Oper: Wkc000;#;A"
        # we need the #
        if buffer.split()[0] != "Oper:":
            print("The file is corrupt, \"Oper\" expected!")
            exit(1)

        oper = buffer.split()[1]
        job.proc = int(oper.split(";")[1])

        if job.proc <= 0:
            print("The file is corrupt, processing time not positive")
            exit(1)

        job.wp = float(job.weight) / job.proc;
        jobList.append(job)

    if len(jobList) == 0:
        print("No jobs defined!")
        exit(1);

    Fjob.close()
    return jobList
#################################################

def writeSeq(jobList):
    seqFileName = "_user.seq"
    Fseq = open(seqFileName, "w")

    Fseq.write("Schedule: Python WSPT rule\n") # schedule name
    Fseq.write("Machine: Wkc000.000\n")  # name of the first (and last) machine

    # now enumerate the operations
    for job in jobList:
        Fseq.write("Oper: Job" + str(job.id) + "\n")

    Fseq.close()
#################################################

def sortFunc(job):
    return job.wp
#################################################

print("Running Python!")
print("Script:", sys.argv[0])
print("Current dir:", os.getcwd())

readMch()
jobList = readJob()

jobList = sorted(jobList, key = sortFunc, reverse = True)

writeSeq(jobList)
print("SUCCESS!")
