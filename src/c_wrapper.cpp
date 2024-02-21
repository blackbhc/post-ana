// C wrapper for the post-ana library
#include "../include/aligner.hpp"
#include "../include/derivatives.hpp"
#include "../include/galaxy.hpp"
#include "../include/gravity.hpp"
#include "../include/h5io.hpp"

#include <cstdio>
#include <string>
#include <vector>

using std::vector;

struct info  // struct for data transformation between C++ and python
{
    int     comp;   // the number of components
    double* rs;     // the radius
    double* rv;     // the rotation velocity
    int*    types;  // the particle type for each component
};

extern "C" info* get_the_info( const char* filename, double Rmax, int rBinNum, int phiBinNum )
{
    static info     the_info;
    post_ana::h5io* io = new post_ana::h5io( filename );

    vector< int >     partNums;
    vector< double* > coordinates;
    vector< double* > masses;
    int               offset = 0;
    io->read_datasets( partNums, coordinates, masses, &offset );

    post_ana::galaxy* galaxy = new post_ana::galaxy( coordinates, masses, partNums );
    galaxy->cal_rc( 0.1, Rmax, rBinNum, phiBinNum );
    vector< double >           rs  = galaxy->get_rs();
    vector< vector< double > > rvs = galaxy->get_rvs();

    int dim        = ( rBinNum + 1 ) * phiBinNum;
    the_info.comp  = ( int )partNums.size();
    the_info.rs    = new double[ rBinNum + 1 ];
    the_info.rv    = new double[ the_info.comp * dim ];
    the_info.types = new int[ the_info.comp ];
    for ( int i = 0; i < the_info.comp; ++i )
    {
        the_info.types[ i ] = i + offset;
    }
    for ( int i = 0; i < ( int )rs.size(); ++i )
    {
        the_info.rs[ i ] = rs[ i ];
    }
    for ( int i = 0; i < ( int )rvs.size(); ++i )
        for ( int j = 0; j < dim; ++j )
            the_info.rv[ i * dim + j ] = rvs[ i ][ j ];

    for ( int i = 0; i < ( int )partNums.size(); ++i )
    {
        delete[] coordinates[ i ];
        delete[] masses[ i ];
    }

    delete galaxy;
    delete io;
    return &the_info;
}

extern "C" void free_the_source( info* the_info )
{
    delete[] the_info->rs;
    delete[] the_info->rv;
    delete[] the_info->types;
}
