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
    if ( argc != 3 && argc != 6 )
    {
        printf( "Usage: %s [simulation filename] [log filename] [optional parameters]\n",
                argv[ 0 ] );
        printf( "Optional parameters are: [max radius] [radius bin number] [azimuthal bin "
                "number]\n" );
        printf( "Optional parameters can be omitted or be given simutaneously.\n" );
        return 1;
    }

    printf( "Reading simulation data from the file: %s\n", argv[ 1 ] );
    // ( void )argc;
    // ( void )argv;
    // post_ana::h5io* io = new post_ana::h5io(
    //     "/home/bhchen/FeGradient/Simulation/hr_sigma150/snapshot_000.hdf5", "./test/test.hdf5" );
    post_ana::h5io* io = new post_ana::h5io( argv[ 1 ], argv[ 2 ] );

    double Rmax      = 30;  // the maximum radius to calculate the rotation curve
    int    rBinNum   = 30;  // the number of bins in the radius direction
    int    phiBinNum = 16;  // the number of bins in the azimuthal direction
    if ( argc == 6 )        // if the user specifies the maximum radius
    {
        Rmax      = std::stod( argv[ 3 ] );
        rBinNum   = std::stoi( argv[ 4 ] );
        phiBinNum = std::stoi( argv[ 5 ] );
    }

    vector< int >     partNums;
    vector< double* > coordinates;
    vector< double* > masses;
    io->read_datasets( partNums, coordinates, masses );

    printf( "Number of particles: " );
    for ( auto i : partNums )
    {
        printf( "%d ", i );
    }
    printf( "\n" );

    printf( "Calculating the rotation curve ...\n" );
    post_ana::galaxy* galaxy = new post_ana::galaxy( coordinates, masses, partNums );
    galaxy->cal_rc( 0.1, Rmax, rBinNum, phiBinNum );
    auto rs  = galaxy->get_rs();
    auto rvs = galaxy->get_rvs();
    printf( "Logging into the file: %s ...\n", argv[ 2 ] );
    io->write_results( rs, rvs );
    delete io;
    printf( "Done.\n" );

    for ( int i = 0; i < ( int )partNums.size(); ++i )
    {
        delete[] coordinates[ i ];
        delete[] masses[ i ];
    }
    delete galaxy;

    return 0;
}
