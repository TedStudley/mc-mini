#pragma once

#include <fstream>

#include "geometry/geometry.h"
#include "problem/problem.h"
#include "params.h"

using namespace std;

class OutputStructure {
  public:
    OutputStructure (Params            &p,
                     GeometryStructure &gs,
                     ProblemStructure  &ps);

    ~OutputStructure();

    void outputData (const int timestep = 0);

    void writeHDF5File (const int timestep = 0);

  private:
    Params            &params;
    GeometryStructure &geometry;
    ProblemStructure  &problem;

    int M;
    int N;

    double dx;

    string outputFormat;
    string outputPath;
    string outputFilename;

    std::ofstream problemXdmfFile;
};
