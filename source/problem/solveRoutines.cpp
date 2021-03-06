#include <iostream>
#include <cmath>

#include <Eigen/Sparse>
#include <Eigen/Dense>

#include "boost/math/constants/constants.hpp"

#include "debug/exception.h"
#include "debug.h"

#ifndef USE_DENSE
#include "matrixForms/sparseForms.h"
#else
#include "matrixForms/denseForms.h"
#endif
#include "geometry/dataWindow.h"
#include "geometry/geometry.h"
#include "problem/problem.h"
#include "params.h"

const double pi = boost::math::constants::pi<double>();

using namespace Eigen;
using namespace std;

/*
 *
 * Main loop routines
 *
 */

// Update the forcing terms
// T -> F
void ProblemStructure::updateForcingTerms() {
  DataWindow<double> uForcingWindow (geometry.getUForcingData(), N - 1, M);
  DataWindow<double> vForcingWindow (geometry.getVForcingData(), N, M - 1);

  #ifdef DEBUG
    cout << "<Calculating forcing model using \"" << forcingModel << "\">" << endl;
  #endif

  if (forcingModel == "tauBenchmark") {
    // Benchmark taken from Tau (1991; JCP Vol. 99)
    for (int i = 0; i < M; ++i)
      for (int j = 0; j < N - 1; ++j)
        uForcingWindow (j, i) = 3 * cos ((j + 1) * h) * sin ((i + 0.5) * h);

    for (int i = 0; i < M - 1; ++i)
      for (int j = 0; j < N; ++j)
        vForcingWindow (j, i) = -sin ((j + 0.5) * h) * cos ((i + 1) * h);

  } else if (forcingModel == "solCXBenchmark" ||
             forcingModel == "solKZBenchmark") {
    // solCX Benchmark taken from Kronbichler et al. (2011)
    for (int i = 0; i < M; ++i)
      for (int j = 0; j < N - 1; ++j)
        uForcingWindow (j, i) = 0;

    for (int i = 0; i < M - 1; ++i)
      for (int j = 0; j < N; ++j)
        vForcingWindow (j, i) = - sin((i + 0.5) * pi * h) * cos ((j + 1) * pi * h);

  } else if (forcingModel == "vorticalFlow") {
    for (int i = 0; i < M; ++i)
      for (int j = 0; j < (N - 1); j++)
        uForcingWindow (j, i) = cos ((j + 1) * h) * sin ((i + 0.5) * h);

    for (int i = 0; i < (M - 1); ++i)
      for (int j = 0; j < N; ++j)
        vForcingWindow (j, i) = -sin ((j + 0.5) * h) * cos ((i + 1) * h);

  } else if (forcingModel == "buoyancy") {
    DataWindow<double> temperatureWindow (geometry.getTemperatureData(), N, M);

    double referenceTemperature;
    double densityConstant;
    double thermalExpansion;

    params.push ("problemParams"); {
      params.push ("buoyancyModelParams"); {
        params.queryParam<double>(
                "referenceTemperature",
                referenceTemperature,
                273.15);
        params.queryParam<double>(
                "densityConstant",
                densityConstant,
                100.0);
        params.queryParam<double>(
                "thermalExpansion",
                thermalExpansion,
                1.0);
        params.pop();
      }
      params.pop();
    }

    for (int i = 0; i < M; ++i)
      for (int j = 0; j < (N - 1); ++j)
        uForcingWindow (j, i) = 0;

    for (int i = 0; i < (M - 1); ++i)
      for (int j = 0; j < N; ++j) {
        vForcingWindow (j, i) =  -1 * densityConstant *
                                  (1 - thermalExpansion *
                                   ((temperatureWindow (j, i) +
                                     temperatureWindow (j, i + 1)) / 2 -
                                      referenceTemperature));
      }
  } else {
    THROW_WITH_TRACE(RuntimeError() <<
            errmsg_info("Unexpected forcing model: '" + forcingModel + "'."));
  }

  #ifdef DEBUG
    cout << "<U Forcing Data>" << endl;
    cout << uForcingWindow.displayMatrix() << endl;
    cout << "<V Forcing Data>" << endl;
    cout << vForcingWindow.displayMatrix() << endl << endl;
  #endif
}

// Solve the stokes equation
// F -> U X P
void ProblemStructure::solveStokes() {
  Map<VectorXd> stokesSolnVector (geometry.getStokesData(), M * (N - 1) + (M - 1) * N + M * N);

  #ifndef USE_DENSE
  static SparseMatrix<double> stokesMatrix   (3 * M * N - M - N, 3 * M * N - M - N);
  static SparseMatrix<double> forcingMatrix  (3 * M * N - M - N, 2 * M * N - M - N);
  static SparseMatrix<double> boundaryMatrix (3 * M * N - M - N, 2 * M + 2 * N);
  static SparseLU<SparseMatrix<double>, COLAMDOrdering<int> > solver;
  #else
  /* Don't use this unless you hate your computer. */
  static MatrixXd stokesMatrix   (3 * M * N - M - N, 3 * M * N - M - N);
  static MatrixXd forcingMatrix  (3 * M * N - M - N, 2 * M * N - M - N);
  static MatrixXd boundaryMatrix (3 * M * N - M - N, 2 * M + 2 * N);
  static PartialPivLU<MatrixXd> solver;
  #endif

  static bool initialized;

  double * viscosityData = geometry.getViscosityData();

  if (!(initialized) || !(viscosityModel=="constant")) {

  #ifndef USE_DENSE
    SparseForms::makeStokesMatrix   (stokesMatrix,   M, N, h, viscosityData);
    stokesMatrix.makeCompressed();
    SparseForms::makeForcingMatrix  (forcingMatrix,  M, N);
    forcingMatrix.makeCompressed();
    SparseForms::makeBoundaryMatrix (boundaryMatrix, M, N, h, viscosityData);
    boundaryMatrix.makeCompressed();

    solver.analyzePattern (stokesMatrix);
    solver.factorize (stokesMatrix);
  #else
    DenseForms::makeStokesMatrix   (stokesMatrix, M, N, h, viscosityData);
    DenseForms::makeForcingMatrix  (forcingMatrix, M, N);
    DenseForms::makeBoundaryMatrix (boundaryMatrix, M, N, h, viscosityData);

    solver.compute (stokesMatrix);
  #endif
    initialized = true;
  }

  stokesSolnVector = solver.solve (forcingMatrix  * Map<VectorXd>(geometry.getForcingData(), 2 * M * N - M - N) +
                                   boundaryMatrix * Map<VectorXd>(geometry.getVelocityBoundaryData(), 2 * M + 2 * N));

  Map<VectorXd> pressureVector (geometry.getPressureData(), M * N);
  double pressureMean = pressureVector.sum() / (M * N);
  pressureVector -= VectorXd::Constant (M * N, pressureMean);

#ifdef DEBUG
  cout << "<Calculated Stokes Equation Solutions>" << endl;
  cout << "<U Velocity Data>" << endl;
  cout << DataWindow<double> (geometry.getUVelocityData(), N - 1, M).displayMatrix() << endl;
  cout << "<V Velocity Data>" << endl;
  cout << DataWindow<double> (geometry.getVVelocityData(), N, M - 1).displayMatrix() << endl;
  cout << "<Pressure Data>" << endl;
  cout << DataWindow<double> (geometry.getPressureData(), N, M).displayMatrix() << endl << endl;
#endif
}

// Solve the advection/diffusion equation
// U X T -> T
void ProblemStructure::solveAdvectionDiffusion() {
  #ifdef DEBUG
    cout << "<Using \"" << advectionMethod << "\" for advection>" << endl;
  #endif
  if (advectionMethod == "upwindMethod") {
    upwindMethod();
  } else if (advectionMethod == "frommMethod") {
    frommMethod();
  } else if (advectionMethod == "none") {
  } else {
    THROW_WITH_TRACE(RuntimeError()
            << errmsg_info("Unexpected advection method: '" + advectionMethod + "'."));
  }

  #ifdef DEBUG
    cout << "<Using \"" << diffusionMethod << "\" for diffusion>" << endl;
  #endif
  if (diffusionMethod == "forwardEuler") {
    forwardEuler();
  } else if (diffusionMethod == "backwardEuler") {
    backwardEuler();
  } else if (diffusionMethod == "crankNicolson") {
    crankNicolson();
  } else if (diffusionMethod == "none") {
  } else {
    THROW_WITH_TRACE(RuntimeError()
            << errmsg_info("Unexpected diffusion method: '" + diffusionMethod + "'."));
  }

  #ifdef DEBUG
    cout << "<Finished Advection/Diffusion Step>" << endl << endl;
  #endif
}
