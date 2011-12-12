/*! \file unitTestKeplerPropagator.cpp
 *    Source file that defines a unit test that tests the Kepler propagator included in Tudat.
 *
 *    Path              : /Astrodynamics/Propagators/
 *    Version           : 4
 *    Check status      : Checked
 *
 *    Author            : K. Kumar
 *    Affiliation       : Delft University of Technology
 *    E-mail address    : K.Kumar@tudelft.nl
 *
 *    Checker           : E. Iorfida
 *    Affiliation       : Delft University of Technology
 *    E-mail address    : elisabetta_iorfida@yahoo.it
 *
 *    Date created      : 16 February, 2011
 *    Last modified     : 20 September, 2011
 *
 *    References
 *      Melman, J. Propagate software, J.C.P.Melman@tudelft.nl, 2010.
 *
 *    Notes
 *      Test runs code and verifies result against expected value.
 *      If the tested code is erroneous, the test function returns a boolean
 *      true; if the code is correct, the function returns a boolean false.
 *
 *      Currently, this file makes use of benchmark data provided by J. Melman.
 *      In future, it is desirable that the benchmark data is the direct output
 *      of a commercial package such as STK, where are initial conditions of
 *      the simulation are known.
 *
 *    Copyright (c) 2010-2011 Delft University of Technology.
 *
 *    This software is protected by national and international copyright.
 *    Any unauthorized use, reproduction or modification is unlawful and
 *    will be prosecuted. Commercial and non-private application of the
 *    software in any form is strictly prohibited unless otherwise granted
 *    by the authors.
 *
 *    The code is provided without any warranty; without even the implied
 *    warranty of merchantibility or fitness for a particular purpose.
 *
 *    Changelog
 *      YYMMDD    Author            Comment
 *      110216    K. Kumar          First creation of code.
 *      110217    E. Iorfida        Minor changes made.
 *      110221    K. Kumar          Updated variable-naming to comply with protocol.
 *      110920    K. Kumar          Corrected simple errors outlined by M. Persson.
 */

// Include statements.
#include <cmath>
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include "Astrodynamics/Bodies/celestialBody.h"
#include "Astrodynamics/Bodies/planet.h"
#include "Astrodynamics/Bodies/vehicle.h"
#include "Astrodynamics/EnvironmentModels/centralGravityField.h"
#include "Astrodynamics/Propagators/keplerPropagator.h"
#include "Astrodynamics/Propagators/seriesPropagator.h"
#include "Astrodynamics/States/cartesianElements.h"
#include "Astrodynamics/States/state.h"
#include "Basics/basicFunctions.h"
#include "Mathematics/basicMathematicsFunctions.h"
#include "Mathematics/RootFindingMethods/newtonRaphson.h"
#include "Mathematics/unitConversions.h"

//! Test implementation of Kepler propagator class.
int main( )
{
    // Using declarations.
    using namespace tudat;

    // Test to see if the orbit of a satellite around the Earth is correctly
    // reproduced with respect to benchmark reference data.

    // Test result initialised to false.
    bool isKeplerPropagatorErroneous = false;

    // Load benchmark data.
    // This data originates from J. Melman and is generated by the software
    // package Propagate. The benchmark data was verified against output from
    // Satellite Toolkit (STK).

    // Load file with benchmark data.
    std::string relativePathToBenchmarkData = "Astrodynamics/Propagators/twoBodyKeplerData.dat";

    std::string absolutePathToBenchmarkData = basic_functions::getRootPath( )
            + relativePathToBenchmarkData;

    std::ifstream twoBodyKeplerBenchmarkFile( absolutePathToBenchmarkData.c_str( ) );

    // Check if file could be opened.
    if ( !twoBodyKeplerBenchmarkFile )
    {
        std::cerr << "Error: Two-body Kepler benchmark data file could not be opened."
                  << std::endl;
        std::cerr << absolutePathToBenchmarkData << std::endl;
    }

    // Create propagation history map for benchmark data to be stored in.
    std::map < double, CartesianElements > benchmarkKeplerPropagationHistory;

    // Declare elapsed time.
    double elapsedTime = 0.0;

    // Initialize counter.
    unsigned int twoBodyKeplerDataCounter = 0;

    // Populate propagation history map with benchmark data from file.
    while ( !twoBodyKeplerBenchmarkFile.eof( ) )
    {
        // Store elapsed time which is given in first coloumn.
        twoBodyKeplerBenchmarkFile >> elapsedTime;

        // Store state date from file.
        for ( unsigned int i = 0; i < 6; i++ )
        {
            twoBodyKeplerBenchmarkFile >> benchmarkKeplerPropagationHistory[
                                          twoBodyKeplerDataCounter * 3600.0 ].state( i );
        }

        // Increment counter.
        twoBodyKeplerDataCounter++;
    }

    // Close benchmark data file.
    twoBodyKeplerBenchmarkFile.close( );

    // Run Kepler propagator simulation.

    // Create pointer to the state of satellite Asterix given in Cartesian
    // elements.
    CartesianElements stateOfAsterix;

    // Fill initial state vector with position and
    // velocity given for Asterix.
    // Position is given in kilometers and
    // velocity is given in kilometers per second.
    stateOfAsterix.setCartesianElementX( 6.75e3 );
    stateOfAsterix.setCartesianElementY( 0.0 );
    stateOfAsterix.setCartesianElementZ( 0.0 );
    stateOfAsterix.setCartesianElementXDot( 0.0 );
    stateOfAsterix.setCartesianElementYDot( 8.0595973215 );
    stateOfAsterix.setCartesianElementZDot( 0.0 );

    // Convert initial state vector to meters from
    // kilometers.
    stateOfAsterix.state = unit_conversions::convertKilometersToMeters( stateOfAsterix.state );

    // Create map of propagation history of Asterix using a Kepler propagator
    // and a map iterator.
    std::map < double, State > asterixKeplerPropagationHistory;
    std::map < double, State >::iterator iteratorAsterixKeplerPropagationHistory;

    // Create a pointer to new vehicle for Asterix.
    Vehicle asterix;

    // Create Earth central gravity field.
    CentralGravityField earthCentralGravityField;

    // Set Earth gravitational parameter.
    earthCentralGravityField.setGravitationalParameter( 3.986004415e14 );

    // Create Earth object and set central gravity field.
    Planet earth;
    earth.setGravityFieldModel( &earthCentralGravityField );

    // Create Newton-Raphson object.
    NewtonRaphson newtonRaphson;

    // Create Kepler propagator object.
    KeplerPropagator keplerPropagator;

    // Set Newton-Raphson method.
    keplerPropagator.setNewtonRaphson( &newtonRaphson );

    // Add Asterix as the body that has to be propagated.
    keplerPropagator.addBody( &asterix );

    // Set the central body for Asterix
    keplerPropagator.setCentralBody( &asterix, &earth );

    // Create series propagator.
    SeriesPropagator seriesPropagator;

    // Set the series propagation start time.
    seriesPropagator.setSeriesPropagationStart( 0.0 );

    // Set the propagation end time.
    seriesPropagator.setSeriesPropagationEnd( 86400.0 );

    // Set fixed output interval for series propagation.
    seriesPropagator.setFixedOutputInterval( 3600.0 );

    // Set Kepler propagator for series propagation.
    seriesPropagator.setPropagator( &keplerPropagator );

    // Set initial state of Asterix.
    seriesPropagator.setInitialState( &asterix, &stateOfAsterix );

    // Run simulation.
    seriesPropagator.execute( );

    // Get series propagation history of Asterix.
    asterixKeplerPropagationHistory
            = seriesPropagator
              .getPropagationHistoryAtFixedOutputIntervals( &asterix );

    // Compute propagation history state data from meters to kilometers.
    for ( iteratorAsterixKeplerPropagationHistory = asterixKeplerPropagationHistory.begin( );
          iteratorAsterixKeplerPropagationHistory != asterixKeplerPropagationHistory.end( );
          iteratorAsterixKeplerPropagationHistory++ )
    {
        iteratorAsterixKeplerPropagationHistory->second.state
                = unit_conversions::convertMetersToKilometers(
                    iteratorAsterixKeplerPropagationHistory->second.state );
    }

    // Declare tolerance between benchmark data and simulation data.
    double toleranceBetweenBenchmarkAndSimulationData = 1e-5;

    // Declare difference between benchmark data and simulation data.
    double differenceKeplerData;

    // Check if results match benchmark data.
    for ( unsigned int i = 1; i < ( seriesPropagator.getSeriesPropagationEnd( )
                                    / seriesPropagator.getFixedOutputInterval( ) ); i++ )
    {
        // Initialize difference in data.
        differenceKeplerData = 0.0;

        for ( unsigned int j = 0; j < 6; j++ )
        {
            differenceKeplerData += std::fabs(
                        asterixKeplerPropagationHistory[ i * seriesPropagator
                        .getFixedOutputInterval( ) ].state( j )
                        - benchmarkKeplerPropagationHistory[  i * seriesPropagator
                        .getFixedOutputInterval( ) ].state( j ) );
        }

        if ( differenceKeplerData > toleranceBetweenBenchmarkAndSimulationData )
        {
            isKeplerPropagatorErroneous = true;

            std::cerr << "The Kepler propagator does not produce consistent results, as running a "
                      << "simulation with does not yield the same results as the benchmark data "
                      << "given the same initial conditions." << std::endl;
            std::cerr << "Expected: " << benchmarkKeplerPropagationHistory
                         [ i * seriesPropagator.getFixedOutputInterval( ) ].state.transpose( )
                      << std::endl
                      << "Actual: " << asterixKeplerPropagationHistory
                         [ i * seriesPropagator.getFixedOutputInterval( ) ].state.transpose( )
                      << std::endl
                      << "Difference: " << differenceKeplerData << std::endl;
        }
    }

    // Return test result.
    // If test is successful return false; if test fails, return true.
    if ( isKeplerPropagatorErroneous )
    {
        std::cerr << "testKeplerPropagator failed!" << endl;
    }

    return isKeplerPropagatorErroneous;
}

// End of file.
