#include "../include/aligner.hpp"
#include <cmath>
#include <cstring>
#include <gsl/gsl_eigen.h>
#include <gsl/gsl_matrix.h>
#include <iostream>

#define MAX_ITERATION 50

namespace post_ana {
inline double aligner::norm( double* vec )
{
    return std::sqrt( vec[ 0 ] * vec[ 0 ] + vec[ 1 ] * vec[ 1 ] + vec[ 2 ] * vec[ 2 ] );
}

inline double aligner::error( double* vec1, double* vec2 )
{
    double error[ 3 ] = { vec1[ 0 ] - vec2[ 0 ], vec1[ 1 ] - vec2[ 1 ], vec1[ 2 ] - vec2[ 2 ] };
    return this->norm( error );
}

void aligner::recenter( double* coordinates, double* masses, int particleNumber )
{
    double center[ 3 ]    = { 0, 0, 0 };
    double old_value[ 3 ] = { 0, 0, 0 };
    double totalMass      = 0;
    for ( int i = 0; i < particleNumber; ++i )
    {
        if ( this->norm( coordinates + 3 * i ) < 100 )
        {
            // only consider the particles within 100 kpc
            for ( int j = 0; j < 3; ++j )
            {
                center[ j ] += masses[ i ] * coordinates[ 3 * i + j ];
            }
            totalMass += masses[ i ];
        }
    }

    // calculate the center of mass and initialize the old value
    for ( int j = 0; j < 3; ++j )
        center[ j ] /= totalMass;

    int counter = 0;
    do
    {
        // update the old value
        std::memcpy( old_value, center, 3 * sizeof( double ) );

        // initialize the center of mass and the total mass
        std::memset( center, 0, 3 * sizeof( double ) );
        totalMass = 0;

        for ( int i = 0; i < particleNumber; ++i )
        {
            if ( this->error( coordinates + 3 * i, old_value ) < 20 )
            {
                // only consider the particles within 20 kpc from the old center
                for ( int j = 0; j < 3; ++j )
                {
                    center[ j ] += masses[ i ] * coordinates[ 3 * i + j ];
                }
                totalMass += masses[ i ];
            }
        }

        // update the center of mass
        for ( int j = 0; j < 3; ++j )
            center[ j ] /= totalMass;

        // check whether reach the maximum iteration
        if ( counter++ >= MAX_ITERATION )
        {
            std::cerr << "Maximum iteration reached in recentering the particles!" << std::endl;
            break;
        }

        // check the convergence
    } while ( this->error( center, old_value ) >= 0.1 );

    // recenter the particles
    for ( int i = 0; i < particleNumber; ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            coordinates[ 3 * i + j ] -= center[ j ];
        }
    }
}

void aligner::align( double* coordinates, double* masses, int particleNumber )
{
    this->recenter( coordinates, masses, particleNumber );

    // calculate the moment of inertia tensor inside 10 kpc
    double tensor[ 3 ][ 3 ] = { { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
    for ( int i = 0; i < particleNumber; ++i )
    {
        if ( this->norm( coordinates + 3 * i ) < 10 )
        {
            // non-diagonal elements
            for ( int j = 0; j < 3; ++j )
            {
                for ( int k = 0; k < 3; ++k )
                {
                    if ( j != k )
                    {
                        tensor[ j ][ k ] -=
                            masses[ i ] * ( coordinates[ 3 * i + j ] * coordinates[ 3 * i + k ] );
                    }
                }
            }

            // diagonal elements
            for ( int j = 0; j < 3; ++j )
            {
                tensor[ j ][ j ] +=
                    masses[ i ] * this->norm( coordinates + 3 * i )
                        * this->norm( coordinates + 3 * i )
                    - masses[ i ] * coordinates[ 3 * i + j ] * coordinates[ 3 * i + j ];
            }
        }
    }

    // calculate the eigenvalues and eigenvectors of the moment of inertia tensor with gsl
    gsl_matrix* m = gsl_matrix_alloc( 3, 3 );
    for ( int i = 0; i < 3; ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            gsl_matrix_set( m, i, j, tensor[ i ][ j ] );
        }
    }

    gsl_vector*                eval = gsl_vector_alloc( 3 );
    gsl_matrix*                evec = gsl_matrix_alloc( 3, 3 );
    gsl_eigen_symmv_workspace* w    = gsl_eigen_symmv_alloc( 3 );
    gsl_eigen_symmv( m, eval, evec, w );
    gsl_eigen_symmv_free( w );
    // sort the eigenvalues and eigenvectors in decreasing order of absolute value
    gsl_eigen_symmv_sort( eval, evec, GSL_EIGEN_SORT_ABS_DESC );

    // calculate the determinant of the eigenvectors
    double det =
        gsl_matrix_get( evec, 0, 0 ) * gsl_matrix_get( evec, 1, 1 ) * gsl_matrix_get( evec, 2, 2 )
        + gsl_matrix_get( evec, 0, 1 ) * gsl_matrix_get( evec, 1, 2 ) * gsl_matrix_get( evec, 2, 0 )
        + gsl_matrix_get( evec, 0, 2 ) * gsl_matrix_get( evec, 1, 0 ) * gsl_matrix_get( evec, 2, 1 )
        - gsl_matrix_get( evec, 0, 2 ) * gsl_matrix_get( evec, 1, 1 ) * gsl_matrix_get( evec, 2, 0 )
        - gsl_matrix_get( evec, 0, 1 ) * gsl_matrix_get( evec, 1, 0 ) * gsl_matrix_get( evec, 2, 2 )
        - gsl_matrix_get( evec, 0, 0 ) * gsl_matrix_get( evec, 1, 2 )
              * gsl_matrix_get( evec, 2, 1 );
    // correct the sign of the eigenvectors
    if ( det < 0 )
    {
        for ( int i = 0; i < 3; ++i )
        {
            gsl_matrix_set( evec, i, 2, -gsl_matrix_get( evec, i, 2 ) );
        }
    }

    // calculate the rotation matrix: transpose of the eigenvectors
    gsl_matrix* target = gsl_matrix_alloc( 3, 3 );
    std::memcpy( target->data, evec->data, 9 * sizeof( double ) );
    gsl_matrix_transpose( target );

    // calculate the rotation matrix: transpose of the eigenvectors
    double rotation[ 3 ][ 3 ];
    for ( int i = 0; i < 3; ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            rotation[ i ][ j ] = gsl_matrix_get( target, i, j );
        }
    }

    // rotate the coordinates
    double temp[ 3 ];
    for ( int i = 0; i < particleNumber; ++i )
    {
        for ( int j = 0; j < 3; ++j )
        {
            temp[ j ] = 0;
            for ( int k = 0; k < 3; ++k )
            {
                temp[ j ] += rotation[ j ][ k ] * coordinates[ 3 * i + k ];
            }
        }

        for ( int j = 0; j < 3; ++j )
        {
            coordinates[ 3 * i + j ] = temp[ j ];
        }
    }

    // free the memory
    gsl_matrix_free( m );
    gsl_vector_free( eval );
    gsl_matrix_free( evec );
    gsl_matrix_free( target );
}

}  // namespace post_ana
