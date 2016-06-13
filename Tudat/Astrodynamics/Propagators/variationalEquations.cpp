#include <map>

#include <boost/function.hpp>

#include <Eigen/Core>

#include "Tudat/Astrodynamics/Propagators/variationalEquations.h"
#include "Tudat/Astrodynamics/OrbitDetermination/AccelerationPartials/accelerationPartial.h"


namespace tudat
{

namespace propagators
{

using orbit_determination::partial_derivatives::StateDerivativePartialsMap;
using namespace tudat::estimatable_parameters;

//! Calculates matrix containing partial derivatives of state derivatives w.r.t. body state.
void VariationalEquations::setBodyStatePartialMatrix( )
{
    using namespace orbit_determination::partial_derivatives;

    // Initialize partial matrix
    variationalMatrix_.setZero( );

    if( dynamicalStatesToEstimate_.count( propagators::transational_state ) > 0 )
    {
        int startIndex = stateTypeStartIndices_.at( propagators::transational_state );
        for( unsigned int i = 0; i < dynamicalStatesToEstimate_.at( propagators::transational_state ).size( ); i++ )
        {
            variationalMatrix_.block( startIndex + i * 6, startIndex + i * 6 + 3, 3, 3 ).setIdentity( );
        }
    }

    // Iterate over all bodies undergoing accelerations for which initial condition is to be estimated.
    for( std::map< IntegratedStateType, std::vector< std::multimap< std::pair< int, int >, boost::function< void( Eigen::Block< Eigen::MatrixXd > ) > > > >::iterator
         typeIterator = statePartialList_.begin( ); typeIterator != statePartialList_.end( ); typeIterator++ )
    {
        int startIndex = stateTypeStartIndices_.at( typeIterator->first );
        int currentStateSize = getSingleIntegrationSize( typeIterator->first );
        int entriesToSkipPerEntry = currentStateSize - currentStateSize / getSingleIntegrationDifferentialEquationOrder( typeIterator->first );
        for( unsigned int i = 0; i < typeIterator->second.size( ); i++ )
        {
            // Iterate over all bodies exerting an acceleration on this body.
            for( statePartialIterator_ = typeIterator->second.at( i ).begin( ); statePartialIterator_ != typeIterator->second.at( i ).end( );
                 statePartialIterator_++ )
            {
                statePartialIterator_->second(
                            variationalMatrix_.block(
                                        startIndex + entriesToSkipPerEntry + i * currentStateSize, statePartialIterator_->first.first,
                                        currentStateSize - entriesToSkipPerEntry, statePartialIterator_->first.second ) );

            }
        }
    }

   for( unsigned int i = 0; i < statePartialAdditionIndices_.size( ); i++ )
   {
       variationalMatrix_.block( 0, statePartialAdditionIndices_.at( i ).second, totalDynamicalStateSize_, 3 ) +=
               variationalMatrix_.block( 0, statePartialAdditionIndices_.at( i ).first, totalDynamicalStateSize_, 3 );
   }
}


//! This function updates all state derivative models to the current time and state.

void VariationalEquations::updatePartials( const double currentTime )
{
    for( stateDerivativeTypeIterator_ = stateDerivativePartialList_.begin( ); stateDerivativeTypeIterator_ != stateDerivativePartialList_.end( );
         stateDerivativeTypeIterator_++ )
    {
        for( unsigned int i = 0; i < stateDerivativeTypeIterator_->second.size( ); i++ )
        {
            for( unsigned int j = 0; j < stateDerivativeTypeIterator_->second.at( i ).size( ); j++ )
            {
                stateDerivativeTypeIterator_->second.at( i ).at( j )->resetTime( TUDAT_NAN );
            }

        }
    }

    // Update all acceleration partials to current state and time. Information is passed indirectly from here, through
    // (function) pointers set in acceleration partial classes
    for( stateDerivativeTypeIterator_ = stateDerivativePartialList_.begin( ); stateDerivativeTypeIterator_ != stateDerivativePartialList_.end( );
         stateDerivativeTypeIterator_++ )
    {
        for( unsigned int i = 0; i < stateDerivativeTypeIterator_->second.size( ); i++ )
        {
            for( unsigned int j = 0; j < stateDerivativeTypeIterator_->second.at( i ).size( ); j++ )
            {
                stateDerivativeTypeIterator_->second.at( i ).at( j )->update( currentTime );
            }

        }
    }

    for( stateDerivativeTypeIterator_ = stateDerivativePartialList_.begin( ); stateDerivativeTypeIterator_ != stateDerivativePartialList_.end( );
         stateDerivativeTypeIterator_++ )
    {
        for( unsigned int i = 0; i < stateDerivativeTypeIterator_->second.size( ); i++ )
        {
            for( unsigned int j = 0; j < stateDerivativeTypeIterator_->second.at( i ).size( ); j++ )
            {
                stateDerivativeTypeIterator_->second.at( i ).at( j )->updateParameterPartials( );
            }

        }
    }
}

void VariationalEquations::setStatePartialFunctionList( )
{
    using namespace orbit_determination::partial_derivatives;

    std::pair< boost::function< void( Eigen::Block< Eigen::MatrixXd > ) >, int > currentDerivativeFunction;

    for( std::map< propagators::IntegratedStateType, orbit_determination::partial_derivatives::StateDerivativePartialsMap >::iterator
         stateDerivativeTypeIterator_ = stateDerivativePartialList_.begin( ); stateDerivativeTypeIterator_ != stateDerivativePartialList_.end( );
         stateDerivativeTypeIterator_++ )
    {
        // Iterate over all bodies undergoing accelerations for which initial condition is to be estimated.
        for( unsigned int i = 0; i < stateDerivativeTypeIterator_->second.size( ); i++ )
        {
            std::multimap< std::pair< int, int >, boost::function< void( Eigen::Block< Eigen::MatrixXd > ) > > currentBodyPartialList;
            // Iterate over all accelerations from single body on other single body
            for( unsigned int j = 0; j < stateDerivativeTypeIterator_->second.at( i ).size( ); j++ )
            {
                for( std::map< propagators::IntegratedStateType, std::vector< std::pair< std::string, std::string > > >::iterator
                     estimatedStateIterator = dynamicalStatesToEstimate_.begin( ); estimatedStateIterator != dynamicalStatesToEstimate_.end( );
                     estimatedStateIterator++ )
                {
                    // Iterate over all bodies to see if body exerting acceleration is also to be estimated (cross-terms)
                    for( unsigned int k = 0; k < estimatedStateIterator->second.size( ); k++ )
                    {
                        currentDerivativeFunction = stateDerivativeTypeIterator_->second.at( i ).at( j )->
                                getDerivativeFunctionWrtStateOfIntegratedBody(
                                    estimatedStateIterator->second.at( k ), estimatedStateIterator->first );

                        if( currentDerivativeFunction.second != 0 )
                        {
                            currentBodyPartialList.insert(
                                        std::make_pair(
                                            std::make_pair( k * getSingleIntegrationSize( estimatedStateIterator->first ) +
                                                            stateTypeStartIndices_.at( estimatedStateIterator->first ),
                                                            getSingleIntegrationSize( estimatedStateIterator->first ) ),
                                            currentDerivativeFunction.first ) );
                        }
                    }
                }
            }
            statePartialList_[ stateDerivativeTypeIterator_->first ].push_back( currentBodyPartialList );
        }
    }
}

}

}
