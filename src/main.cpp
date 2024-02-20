// main function to produce the executable
#include "../include/aligner.hpp"
#include "../include/derivatives.hpp"
#include "../include/galaxy.hpp"
#include "../include/gravity.hpp"
#include "../include/h5io.hpp"

#include <cstdio>
#include <string>
#include <vector>

using std::vector;

int main( int argc, char* argv[] )
{
    if ( argc != 3 )
    {
        printf( "Usage: %s [simulation filename] [log filename]\n", argv[ 0 ] );
        return 1;
    }

    printf( "Reading simulation data from the file: %s\n", argv[ 1 ] );
    post_ana::h5io*   io = new post_ana::h5io( argv[ 1 ], argv[ 2 ] );
    vector< int >     partNums;
    vector< double* > coordinates;
    vector< double* > masses;
    io->read_datasets( partNums, coordinates, masses );

    printf( "Number of particles: \n" );
    for ( auto i : partNums )
    {
        printf( "%d ", i );
    }
    printf( "\n" );

    printf( "Calculating the rotation curve ...\n" );
    post_ana::galaxy* galaxy = new post_ana::galaxy( coordinates, masses, partNums );
    galaxy->cal_rc( 0.1, 16.0, 4, 4 );
    auto rs = galaxy->get_rs();
    auto rv = galaxy->get_rv();
    printf( "Rotation curve: \n" );
    for ( int i = 0; i < ( int )rs.size(); ++i )
    {
        printf( "%f ", rs[ i ] );
    }
    printf( "\n" );
    for ( int i = 0; i < ( int )rv.size(); ++i )
    {
        printf( "Component %d: \n", i );
        for ( int j = 0; j < ( int )rv[ i ].size(); ++j )
        {
            printf( "%f ", rv[ i ][ j ] );
        }
        printf( "\n" );
    }


    printf( "Log into the file: %s\n", argv[ 2 ] );
    delete io;

    for ( int i = 0; i < ( int )partNums.size(); ++i )
    {
        delete[] coordinates[ i ];
        delete[] masses[ i ];
    }
    delete galaxy;

    return 0;
}
