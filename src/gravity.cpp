// gravity calculation by direct summation
#include "../include/gravity.hpp"

namespace post_ana {
gravity::gravity( double* coordinates, double* masses, int particleNumber )
{
    if ( particleNumber < 1 )
    {
        std::cerr << "Error: particle number must be greater than 0, get " << particleNumber
                  << std::endl;
        exit( 1 );
    }
    this->coordinates    = coordinates;
    this->masses         = masses;
    this->particleNumber = particleNumber;
}

inline double gravity::distance( double* pos1, double* pos2 )
{
    return std::sqrt( ( pos1[ 0 ] - pos2[ 0 ] ) * ( pos1[ 0 ] - pos2[ 0 ] )
                      + ( pos1[ 1 ] - pos2[ 1 ] ) * ( pos1[ 1 ] - pos2[ 1 ] )
                      + ( pos1[ 2 ] - pos2[ 2 ] ) * ( pos1[ 2 ] - pos2[ 2 ] ) );
}

void gravity::force( double* pos, double* force )
{
    // initialize the forces
    for ( int i = 0; i < 3; ++i )
        force[ i ] = 0;

    double r;
    for ( int i = 0; i < this->particleNumber; ++i )
    {
        r = this->distance( pos, this->coordinates + 3 * i );
        for ( int j = 0; j < 3; ++j )
        {
            force[ j ] += G * this->masses[ i ] * ( this->coordinates[ 3 * i + j ] - pos[ j ] )
                          / ( r * r * r );
        }
    }
}

void gravity::potential( double* pos, double* pot )
{
    *pot = 0;  // initialize the potential
    for ( int i = 0; i < this->particleNumber; ++i )
    {
        *pot += -G * this->masses[ i ] / this->distance( pos, this->coordinates + 3 * i );
    }
}

void gravity::forces( double* poses, double* forces, int n )
{
    for ( int i = 0; i < n; ++i )
    {
        this->force( poses + 3 * i, forces + 3 * i );
    }
}

void gravity::potentials( double* poses, double* potentials, int n )
{
    for ( int i = 0; i < n; ++i )
    {
        this->potential( poses + 3 * i, potentials + i );
    }
}

}  // namespace post_ana
