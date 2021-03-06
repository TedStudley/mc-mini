/** \file problem.cpp
    \brief ProblemStructure relevant member functions
 */

#include <iostream>
#include <cassert>
#include <limits>

#include <Eigen/Sparse>

#include "geometry/geometry.h"
#include "problem/problem.h"
#include "params.h"
#include "debug.h"

using namespace Eigen;
using namespace std;

ProblemStructure::ProblemStructure(Params &p,
                                   GeometryStructure &gs) :
  /** References to the ParamParser and GeometryStructure objects created
   *  in main() allow ProblemStructure to ask the ParamParser and
   *  GeometryStructure for information it would not otherwise have access to,
   *  such as the size of data in memory, pointers to all of the problem data 
   *  arrays, and values of parameters from the parameter file passed in by the
   *  user. 
   */
    params   (p),
    geometry (gs) {
  /** The majority of calls to the shared GeometryStructure object come from
   *  requests for the pointers to data in memory, but access to
   *  GeometryStructure is also required to find the values of M and N, the
   *  height and width of the domain in cells. 
   */
  M = geometry.getM();
  N = geometry.getN();
 
  /** ProblemStructure uses ParamParser to read all of the relevant information
   *  from the parameter file. Parameters regarding this structure may be found
   *  under the 'problemParams' section in the the example parameter file. 
   *
   *  In order to add more parameter-specified attributes to ProblemStructure,
   *  changes must be made in several places. First, if the attribute is
   *  'persistent', or if we want to use the attribute without calling the
   *  parser every time, we need to declare a member variable to hold the value
   *  of that attribute. This is not always necessary, but prevents messy calls
   *  to the ParamParser whenever we want to find, for example, the size of the
   *  problem domain.
   *
   *  If the attribute will be persistent through a member variable, it must be
   *  initialized before it can be used. Attributes should be populated from
   *  the parameter file in the ProblemStructure() constructor, to prevent mess
   *  by using multiple ParamParser::push and ParamParser::pop calls
   *  elsewhere in the code.
   */
  params.push("problemParams"); {
    params.getParam<double>("cfl", cfl);

    params.getParam<double>("startTime", time);
    params.queryParam<double>(
            "endTime",
            endTime,
            std::numeric_limits<int>::max());
    params.queryParam<int>(
            "endStep",
            endStep,
            std::numeric_limits<int>::max());
    timestepNumber = 0;

    params.queryParam<double>(
            "xExtent",
            xExtent,
            0.0);
    params.queryParam<double>(
            "yExtent",
            yExtent,
            0.0);

    assert (((xExtent == 0) ^ (yExtent == 0)));

    if (xExtent == 0) {
      h      = yExtent / double(M);
      xExtent = h * double(N);
    } else {
      h      = xExtent / double(N);
      yExtent = h * double(M);
    }

    params.getParam<double>("diffusivity", diffusivity);

    params.queryParam<std::string>(
            "forcingModel",
            forcingModel,
            "tauBenchmark");
    params.queryParam<std::string>(
            "temperatureModel",
            temperatureModel,
            "constant");
    params.queryParam<std::string>(
            "viscosityModel",
            viscosityModel,
            "constant");
    params.queryParam<std::string>(
            "boundaryModel",
            boundaryModel,
            "tauBenchmark");

    params.queryParam<std::string>(
            "advectionMethod",
            advectionMethod,
            "upwindMethod");
    params.push("advectionParams"); {
      params.queryParam<std::string>(
              "fluxLimiter",
              fluxLimiter,
              "vanLeer");

      params.pop();
    }

    params.queryParam<std::string>(
            "diffusionMethod",
            diffusionMethod,
            "backwardEuler");

    params.queryParam<std::string>(
            "outputFile",
            outputFile,
            "output.h5");

    params.pop();
  }
}

/** advanceTimestep() advances the problem time forward by one timestep, 
 *  incrementing the current timestep number in the process, and checks to see 
 *  whether completion conditions (either passing the final problem time or the 
 *  final timestep number) have been met, and, if so, sends a signal to the 
 *  main loop to finalize the simulation.
 */
bool ProblemStructure::advanceTimestep() {
  time += deltaT;
  timestepNumber++;

  return ((time < endTime) && (timestepNumber < endStep));
}

/** recalculateTimestep() calculates the current time step based on the user-
 *  specified CFL condition and the current state of the simulation. The
 *  timestep is calculated based on the maximum rates of advection and
 *  diffusion in the current problem. In future, this should take into account
 *  whether either advection or diffusion have been disabled in the parameter
 *  file.
 */
void ProblemStructure::recalculateTimestep() {
  Map<VectorXd> uVelocityVector (geometry.getUVelocityData(), M *       (N - 1));
  Map<VectorXd> vVelocityVector (geometry.getVVelocityData(), (M - 1) * N);
  Map<VectorXd> uVelocityBoundaryVector (geometry.getUVelocityBoundaryData(), 2 * N);
  Map<VectorXd> vVelocityBoundaryVector (geometry.getVVelocityBoundaryData(), 2 * M);

/** The current time step \f$ \Delta{t} \f$ rate of advection is calculated by finding the maximum
   *  lateral and transverse velocities in the problem domain, then combining
   *  both to gain a "worst-case" idea of the maximum velocity inside the
   *  domain. This is then compared against the CFL condition using the formula
   *  \f[ \Delta{t} = \frac {\sigma h} {||{\vec u}||} \f]
   *  where \f$\Delta{t}\f$ is the time step, \f$\sigma\f$ is the CFL number,
   *  \f$h\f$ is the grid spacing, and \f$ \| \vec u \| f$ is the maximum velocity in
   *  the domain.
   **/
  double maxInteriorUVelocity = uVelocityVector.maxCoeff();
  double maxBoundaryUVelocity = uVelocityBoundaryVector.maxCoeff();
  double maxUVelocity = (maxInteriorUVelocity > maxBoundaryUVelocity) ? maxInteriorUVelocity : maxBoundaryUVelocity;

  /** This needs to be rewritten, it is incorrect, since it is an overestimate of
   *  the maximum velocity in the domain. EGP - 2015-09-27
   */
  /** The current time step \f[ \Delta{t} \f] is calculated by finding the maximum
    *  lateral and  transverse velocities in the problem domain, then combining
    *  both to gain a "worst-case" idea of the maximum velocity inside the
    *  domain. This is then compared against the CFL condition using the formula
    *  \f[ \Delt
   **/

  double maxInteriorVVelocity = vVelocityVector.maxCoeff();
  double maxBoundaryVVelocity = vVelocityBoundaryVector.maxCoeff();
  double maxVVelocity = (maxInteriorVVelocity > maxBoundaryVVelocity) ? maxInteriorVVelocity : maxBoundaryVVelocity;
  double advectionDeltaT = cfl * h / sqrt (maxUVelocity * maxUVelocity + maxVVelocity * maxVVelocity);
  if (maxUVelocity == 0 || maxVVelocity == 0)
    advectionDeltaT = std::numeric_limits<int>::max();

  /** THIS IS INCORRECT! If the solver is explicit (e.g., first order Euler)
   *  then there needs to be a 2 h*h or a 4 * h * h in the denominator!
   *
   *  Then the time step must be \f[ \Delta{t} = \frac {\sigma h} {\kappa} \f]
   *  the maximum velocity in the domain. EGP - 2015-09-27:
   * The calculation for the diffusion is similar, but much simpler. The
   *  diffusive delta-t is calculated using the formula
   *  \f[ \Delta{t} = \frac {\sigma h} {\kappa} \f]
   *  where \f$\Delta{t}\f$
   *
   **/
  double diffusionDeltaT = cfl * h / diffusivity;

  if (advectionDeltaT < diffusionDeltaT) {
    deltaT = advectionDeltaT;
  } else {
    deltaT = diffusionDeltaT;
    cout << "<CAUTION! Recalculated the time step as " << deltaT << "based on an IMPLICIT diffusion solver!>" << endl;
  }

  if (time + deltaT > endTime) { deltaT = endTime - time; }

  #ifdef DEBUG
    cout << "<Recalculated the time step as " << deltaT << ">" << endl;
  #endif
}

double ProblemStructure::getH() {
  return h;
}

double ProblemStructure::getTime() {
  return time;
}

int ProblemStructure::getTimestepNumber() {
  return timestepNumber;
}

double ProblemStructure::getEndTime() {
  return endTime;
}
