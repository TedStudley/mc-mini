#include <iostream>
#include <cmath>

#include "boost/math/constants/constants.hpp"

#include "debug/exception.h"
#include "geometry/dataWindow.h"
#include "geometry/geometry.h"
#include "problem/problem.h"
#include "params.h"
#include "debug.h"

const double pi = boost::math::constants::pi<double>();

/*
 *
 * Data initialization routines
 *
 */

void ProblemStructure::initializeProblem() {
  initializeTimestep();
  initializeTemperature();
  initializeTemperatureBoundary();
  initializeVelocityBoundary();
  initializeViscosity();
}

void ProblemStructure::initializeTimestep() {
  deltaT = cfl * h / diffusivity;
  int nTimestep = (endTime - time) / deltaT;
  if (abs (nTimestep * deltaT + time - endTime) > 1E-06)
    deltaT = (endTime - time) / ++nTimestep;
  #ifdef DEBUG
    std::cout << "<Timestep initialized to " << deltaT << ">" << std::endl;
  #endif
}

void ProblemStructure::initializeTemperature() {
  DataWindow<double> temperatureWindow (geometry.getTemperatureData(), N, M);

  double referenceTemperature;
  double temperatureScale;

  params.push("problemParams"); {
    params.push("initialTemperatureParams"); {
      params.queryParam<double>(
              "referenceTemperature",
              referenceTemperature,
              273.15);
      params.queryParam<double>(
              "temperatureScale",
              temperatureScale,
              100.0);

      params.pop();
    }
    params.pop();
  }

  if (temperatureModel == "constant") {

    for (int i = 0; i < M; ++i)
      for (int j = 0; j < N; ++j)
        temperatureWindow (j, i) = referenceTemperature;

  } else if (temperatureModel == "sineWave") {
    int xModes;
    int yModes;

    params.push("problemParams"); {
      params.tryPush("initialTemperatureParams"); {
        params.queryParam<int>("xModes", xModes, 2);
        params.queryParam<int>("yModes", yModes, 2);

        params.pop();
      }
      params.pop();
    }

    for (int i = 0; i < M; ++i)
      for (int j = 0; j < N; ++j)
        temperatureWindow (j, i) = referenceTemperature +
                                   sin ((i + 0.5) * h * xModes * pi / xExtent) *
                                   sin ((j + 0.5) * h * yModes * pi / yExtent) *
                                   temperatureScale;

  } else if (temperatureModel == "squareWave") {
    for (int i = 0; i < M; ++i)
      for (int j = 0; j < N; ++j) {
        if ((M / 4 < j && j < 3 * M / 4) && (N / 4 < i && i < 3 * N / 4))
          temperatureWindow (j, i) = referenceTemperature + temperatureScale;
        else
          temperatureWindow (j, i) = referenceTemperature;
      }
  } else if (temperatureModel == "circle") {
     double center_x;
     double center_y;
     double radius;
     params.push("problemParams"); {
       params.tryPush("initialTemperatureParams"); {
         params.getParam<double>("radius", radius);
         params.getParam<double>("xCenter", center_x);
         params.getParam<double>("yCenter", center_y);

         params.pop();
       }
       params.pop();
     }

     for (int i = 0; i < M; ++i)
       for (int j= 0; j < N; ++j) {
         if ( std::sqrt(std::pow((i*h+h/2)-(center_y),2.0) + std::pow((j*h+h/2)-(center_x),2.0))  < radius )
           temperatureWindow (j, i) = referenceTemperature + temperatureScale;
         else
           temperatureWindow (j, i) = referenceTemperature;
       }
  } else {
    THROW_WITH_TRACE(InvalidArgument() <<
            errmsg_info("Unexpected temperature model: '" + temperatureModel + "'"));
  }

  #ifdef DEBUG
    cout << "<Initialized temperature model as: \"" << temperatureModel << "\">" << endl;
    cout << "<Temperature Data>" << endl;
    cout << temperatureWindow.displayMatrix() << endl << endl;
  #endif
}

void ProblemStructure::initializeTemperatureBoundary() {
  DataWindow<double> temperatureBoundaryWindow (geometry.getTemperatureBoundaryData(), N, 2);

  double upperTemperature;
  double lowerTemperature;

  params.push("problemParams"); {
    params.push("temperatureBoundaryParams"); {
      params.getParam<double>("upperBoundaryTemperature", upperTemperature);
      params.getParam<double>("lowerBoundaryTemperature", lowerTemperature);

      params.pop();
    }
    params.pop();
  }

  for (int j = 0; j < N; ++j) {
    temperatureBoundaryWindow (j, 0) = lowerTemperature;
    temperatureBoundaryWindow (j, 1) = upperTemperature;
  }
}

void ProblemStructure::initializeVelocityBoundary() {
  DataWindow<double> uVelocityBoundaryWindow (geometry.getUVelocityBoundaryData(), 2, M);
  DataWindow<double> vVelocityBoundaryWindow (geometry.getVVelocityBoundaryData(), N, 2);


  if (boundaryModel == "tauBenchmark") {
    for (int i = 0; i < M; ++i)
      for (int j = 0; j < 2; ++j)
        uVelocityBoundaryWindow (j, i) = cos (j * N * h) * sin ((i + 0.5) * h);
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < N; ++j)
        vVelocityBoundaryWindow (j, i) = -sin ((j + 0.5) * h) * cos (i * M * h);
  } else if (boundaryModel == "solCXBenchmark" ||
             boundaryModel == "solKZBenchmark" ||
             boundaryModel == "noFlux") {
    for (int i = 0; i < M; ++i)
      for (int j = 0; j < 2; ++j)
        uVelocityBoundaryWindow (j, i) = 0;
    for (int i = 0; i < 2; ++i)
      for (int j = 0; j < N; ++j)
        vVelocityBoundaryWindow (j, i) = 0;
  } else {
    THROW_WITH_TRACE(InvalidArgument() <<
            errmsg_info("Unexpected boundary model: '" + boundaryModel + "'"));
  }

  #ifdef DEBUG
    cout << "<Initialized boundary model as: \"" << boundaryModel << "\">" << endl;
    cout << "<U Velocity Boundary Data>" << endl;
    cout << uVelocityBoundaryWindow.displayMatrix() << endl;
    cout << "<V Velocity Boundary Data>" << endl;
    cout << vVelocityBoundaryWindow.displayMatrix() << endl << endl;
  #endif
}

void ProblemStructure::initializeViscosity() {
  DataWindow<double> viscosityWindow (geometry.getViscosityData(), N + 1, M + 1);

  double viscosity;

  if (viscosityModel == "constant") {
    params.push("problemParams"); {
      params.tryPush("initialViscosity"); {
        params.queryParam<double>("viscosityScale", viscosity, 1.0);

        params.pop();
      }
      params.pop();
    }

    for (int i = 0; i < (M + 1); ++i)
      for (int j = 0; j < (N + 1); ++j)
        viscosityWindow (j, i) = viscosity;
  } else if (viscosityModel == "tauBenchmark") {
    viscosity = 1.0;
  } else if (viscosityModel == "solCXBenchmark") {
    for (int i = 0; i < (M + 1); ++i)
      for (int j = 0; j < (N + 1); ++j)
        viscosityWindow (j, i) = (j <= N / 2) ? 1.0 : 1.0E06;
  } else if (viscosityModel == "solKZBenchmark") {
    for (int i = 0; i < (M + 1); ++i)
      for (int j = 0; j < (N + 1); ++j)
        viscosityWindow (j, i) = 1.0 + j * h * 1.0E06;
  } else {
    THROW_WITH_TRACE(InvalidArgument() <<
            errmsg_info("Unexpected viscosity model: '" + viscosityModel + "'"));
  }

  #ifdef DEBUG
    cout << "<Viscosity model initialized as: \"" << viscosityModel << "\">" << endl;
    cout << "<Viscosity Data>" << endl;
    cout << viscosityWindow.displayMatrix() << endl << endl;
  #endif
}
