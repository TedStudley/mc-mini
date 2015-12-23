// Necessary for output in the command line.
#include <iostream>

// Necessary for displaying exception information
#include <boost/exception/diagnostic_information.hpp>

// Exceptions and related typedefs
#include "debug/exception.h"
// Functions and data structures related to the geometry of the problem.
#include "geometry/geometry.h"
// Functions and data structures related to the implementation of numerical methods.
#include "problem/problem.h"
// Functions and data structures related to the output of the solution data.
#include "output/output.h"
// Functions and data structures related to the parser of parameter files.
#include "parser/parser.h"

int main(int argc, char ** argv) {
  // Install the segfault handler with backtrace
  signal(SIGSEGV, handler);

  try {
    // The valid command line usage is "./mc-mini <parameter file>". Otherwise, throw an exception.
    if (argc == 1) {
      THROW_WITH_TRACE(InvalidArgument() <<
              errmsg_info("usage: " + static_cast<std::string>(argv[0]) + " <parameter file>."));
    }

    // Initialize the parser with the specified parameter file.
    ParamParser parser(static_cast<std::string>(argv[1]));
    // Initialize geometry parameters.
    GeometryStructure geometry (parser);
    // Initialize parameters related to the implementation of the employed numerical method.
    ProblemStructure  problem  (parser, geometry);
    // Initialize parameters related to output structure.
    OutputStructure   output   (parser, geometry, problem);

    // Initialize the initial data for the problem to be solved.
    problem.initializeProblem();

    // Main loop where computations are made and data is output for each timestep of the problem.
    do {
      // 1. Solve Stokes equations.
      problem.solveStokes();
      // 2. Initialize the right hand side (forcing terms).
      problem.updateForcingTerms();
      // 3. Recalculate time step.
      problem.recalculateTimestep();
      // 4. Output the solution data.
      output.outputData (problem.getTimestepNumber());
      // 5. Solve advection-diffusion equation.
      problem.solveAdvectionDiffusion();
      // 6. Output which time step is being computed.
      std::cout << "Timestep: " << problem.getTimestepNumber() << ": t=" << problem.getTime() << std::endl;
    } while (problem.advanceTimestep()); // Loop termination criterion: problem.getTimestepNumber() = end_timestep.

    // Solve the Stokes equations.
    problem.solveStokes();
    // Update forcing terms
    problem.updateForcingTerms();
    // Output the solution data.
    output.outputData (problem.getTimestepNumber());
  } catch (std::exception& e) {
    std::cerr << boost::diagnostic_information(e);
  }

  return 0;
}
