// This is a C++ sample code that can be "linked" to LEKIN.
// It implements the WSPT rule for a single machine, no release times
// and no sequence-dependent setup times.  The setting is purposedly
// kept very simple in order to emphasize file i/o as opposed to
// specific algorithms or data structures.

#define _CRT_SECURE_NO_WARNINGS
#include <io.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

// we will need a data structure to store job data

struct TJob
{
    int id;       // job number
    int release;  // job release date (ignored)
    int due;      // job due date (ignored)
    int weight;   // job weight
    int proc;     // job processing time
    double wp;    // w_j / p_j
};

std::vector<TJob> jobArray;

// For the single machine setting, we don't even have to read the
// machine file.  But I'll do it to check that it actually represents
// a single machine setting.

void readMch()
{
    // No need to use the qualified path.  Just "_user.mch".
    std::string mchFileName = "_user.mch";

    // Check if the file exists
    // If not, it is an error: probably we were called
    // standalone, NOT from LEKIN.
    // Our way of handling errors is exit(1).
    // It is a good idea to print something before exitting.

    if (_access(mchFileName.c_str(), 4) != 0)
    {
        std::cout << "Machine file not found!\n";
        exit(1);
    }

    std::ifstream Fmch(mchFileName);
    std::string buffer;

    // Check the first line in the file.
    // If it is not "Ordinary:", too bad.

    std::getline(Fmch, buffer);
    if (buffer != "Ordinary:")
    {
        std::cout << "We do not support flexible workcenters!\n";
        exit(1);
    }

    // Now we skip several lines.  There are 2 ways to skip: getline or ignore.
    // getline allows you to check what you are skipping.

    std::getline(Fmch, buffer);
    // buffer = "Workcenter:        Wkc000",
    // but we don't care.

    std::getline(Fmch, buffer);  // skip "Setup:"
    std::getline(Fmch, buffer);  // skip "Machine:"

    // we don't need the avaiability time and the starting status
    // for the machine, but I'll read it just to show how it's done.
    Fmch >> buffer;  // skip "Release:" keyword
    int avail = 0;
    Fmch >> avail;
    Fmch >> buffer;  // skip "Status:" keyword

    // counting spaces is not a good idea,
    // so just read till the first non-space
    Fmch >> std::ws;
    char status = Fmch.get();

    // Let's check status -- just in case...
    if (status < 'A' || status > 'Z')
    {
        std::cout << "The file is corrupt!\n";
        exit(1);
    }

    // OK, now the rest of the file must contain nothing but
    // a bunch of white-space characters.

    Fmch >> std::ws;
    if (!Fmch.eof())
    {
        std::cout << "The file contains at least 2 workcenters!\n";
        exit(1);
    }
}

// With the job file, it is not that easy.
// We actually have to read the stream of jobs.

void readJob()
{
    std::string jobFileName = "_user.job";
    if (_access(jobFileName.c_str(), 4) != 0)
    {
        std::cout << "Job file not found!\n";
        exit(1);
    }

    std::ifstream Fjob(jobFileName);
    std::string buffer;

    Fjob >> buffer;  // buffer = "Shop:", ignore
    Fjob >> buffer;  // check if single machine
    if (buffer != "Single")
    {
        std::cout << "This is not a single machine file!\n";
        exit(1);
    }

    while (true)
    {
        Fjob >> buffer;  // buffer = "Job:"

        // Check if the file has ended
        if (!Fjob || buffer != "Job:")
        {
            break;
        }

        Fjob >> buffer;  // buffer = "Job###", ignore

        TJob job;
        job.id = jobArray.size();

        Fjob >> buffer;  // buffer = "Release:"
        Fjob >> job.release;

        Fjob >> buffer;  // buffer = "Due:"
        Fjob >> job.due;

        Fjob >> buffer;  // buffer = "Weight:"
        Fjob >> job.weight;

        Fjob >> buffer;  // buffer = "Oper:"
        Fjob >> buffer;  // buffer = "Wkc000;#;A", and we need the #

        size_t index1 = buffer.find(';');
        if (index1 == std::string::npos)
        {
            break;
        }
        ++index1;

        size_t index2 = buffer.find(';', index1);
        if (index2 == std::string::npos)
        {
            break;
        }

        job.proc = std::stoi(buffer.substr(index1, index2 - index1));
        job.wp = double(job.weight) / job.proc;
        jobArray.push_back(job);
    }

    if (jobArray.size() == 0)
    {
        std::cout << "No jobs defined!\n";
        exit(1);
    }
}

// compare function for sorting

bool compare(const TJob& job1, const TJob& job2)
{
    double a = job1.wp - job2.wp;
    return a < 0 || (a == 0 && job1.id < job2.id);
}

// Since this is just a single machine, we can
// implement any rule by simple sorting on the job array.
// We'll use the C standard qsort function.

void sortJobs()
{
    std::sort(jobArray.begin(), jobArray.end(), compare);
}

// Output the schedule file

void writeSeq()
{
    std::ofstream Fsch("_user.seq");
    Fsch << "Schedule: WSPT rule\n";  // schedule name
    Fsch << "Machine: Wkc000.000\n";  // name of the first (and last) machine
                                      // now enumerate the operations
    for (const TJob& job : jobArray)
    {
        Fsch << "Oper: Job" << job.id << "\n";
    }
}

int main(int argc, char* argv[])
{
    // We have to have exactly 2 command line arguments:
    // objective function and time limit.

    if (argc != 3)
    {
        std::cout << "Illegal call!\n";
        exit(1);
    }

    // Check the objective function.  The WSPT rule is for
    // Total Weighted Flow Time (ID = 6, see LEKIN online manual).
    // We'll complain if called for a different objective.

    // don't bother to use _stscanf
    if (std::string(argv[1]) != "6")
    {
        std::cout << "The only objective supported is \\sum w_j C_j.\n";
        exit(1);
    }

    readMch();
    readJob();
    sortJobs();
    writeSeq();

    std::cout << "Success\n";
    return 0;
}
