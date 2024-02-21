#include "../include/galaxy.hpp"
#include "../include/aligner.hpp"
#include <algorithm>
#include <cstring>
#include <thread>

using std::vector;

namespace post_ana {

galaxy::galaxy( vector< double* > coordinates, vector< double* > masses, vector< int > partNums )
{
    this->compNum = ( int )coordinates.size();

    for ( int i = 0; i < this->compNum; ++i )
    {
        int                comp_part_num = partNums[ i ];
        post_ana::gravity* comp_grav =
            new post_ana::gravity( coordinates[ i ], masses[ i ], comp_part_num );
        this->coordinates.push_back( coordinates[ i ] );
        this->masses.push_back( masses[ i ] );
        this->gravity.push_back( comp_grav );
        this->partNum.push_back( comp_part_num );
    }
}

galaxy::~galaxy()
{
    for ( int i = 0; i < this->compNum; ++i )
    {
        delete this->gravity[ i ];
        delete[] this->rv[ i ];
        delete[] this->rc[ i ];
    }

    delete[] this->rs;
    delete this->aligner;
    delete this->derivatives;
}

void galaxy::ensure_aligned()
{
    if ( this->aligned )
    {
        return;
    }
    for ( int i = 0; i < this->compNum; ++i )
        this->aligner->align( this->coordinates[ i ], this->masses[ i ], this->partNum[ i ] );
    this->aligned = true;
}

void galaxy::cal_rv( double rMin, double rMax, int rBin, int phiBin )
{
    static bool first_call = true;
    this->ensure_aligned();

    if ( first_call )
    {
        // allocate memory for the velocity curve at the first call
        for ( int i = 0; i < this->compNum; ++i )
        {
            this->rv.push_back( new double[ ( rBin + 1 ) * phiBin ] );
            std::memset( this->rv[ i ], 0, ( rBin + 1 ) * phiBin * sizeof( double ) );
            this->binSize = { rBin + 1, phiBin };
        }
        first_call = false;
    }

    double logr_bin_size = std::log10( rMax / rMin ) / rBin;
    double phi_bin_size  = 2 * M_PI / phiBin;
    double lgRMin        = std::log10( rMin );

    // inner product of vector
    auto inner = []( double* vec1, double* vec2 ) -> double {
        return vec1[ 0 ] * vec2[ 0 ] + vec1[ 1 ] * vec2[ 1 ] + vec1[ 2 ] * vec2[ 2 ];
    };

    int n_thread = std::thread::hardware_concurrency() / 8;  // the number of threads
    n_thread     = n_thread >= 8 ? n_thread : 8;             // at least 8 threads

    // calculate the circular velocity at each position
    double force[ 3 ] = { 0, 0, 0 };  // temporary force
    // 1. calculate the positions
    double* locs     = new double[ this->compNum * ( rBin + 1 ) * phiBin * 3 ];
    int     sub_dim1 = phiBin * 3;
    int     sub_dim2 = ( rBin + 1 ) * phiBin * 3;
    for ( int comp = 0; comp < this->compNum; ++comp )
        for ( int i = 0; i < rBin + 1; ++i )
            for ( int j = 0; j < phiBin; ++j )
            {
                locs[ comp * sub_dim2 + i * sub_dim1 + j * 3 ] =
                    std::pow( 10, lgRMin + i * logr_bin_size ) * std::cos( j * phi_bin_size );
                locs[ comp * sub_dim2 + i * sub_dim1 + j * 3 + 1 ] =
                    std::pow( 10, lgRMin + i * logr_bin_size ) * std::sin( j * phi_bin_size );
                locs[ comp * sub_dim2 + i * sub_dim1 + j * 3 + 2 ] = 0;
            }
    // 2. calculator of rv for per component, per thread
    auto rv_thread = [ this, locs, &force, &inner, &rBin, &phiBin, &sub_dim1, &sub_dim2,
                       &n_thread ]( int comp, int start ) {
        for ( int i = start; i < ( rBin + 1 ) * phiBin; i += n_thread )
        {
            int row = i / phiBin;
            int col = i % phiBin;
            this->gravity[ comp ]->force( locs + comp * sub_dim2 + row * sub_dim1 + col * 3,
                                          force );
            this->rv[ comp ][ i ] =
                std::sqrt( -inner( force, locs + comp * sub_dim2 + row * sub_dim1 + col * 3 ) );
        }
    };
    // 3. multi-threading for each component
    for ( int comp = 0; comp < this->compNum; ++comp )
    {
        vector< std::thread > threads;
        for ( int i = 0; i < n_thread; ++i )
            threads.push_back( std::thread( rv_thread, comp, i ) );

        for ( int i = 0; i < n_thread; ++i )
            threads[ i ].join();
    }

    delete[] locs;
}

void galaxy::cal_rc( double rMin, double rMax, int rBin, int phiBin )
{
    static bool first_call = true;
    if ( first_call )
    {
        // allocate memory for the velocity curve at the first call
        for ( int i = 0; i < this->compNum; ++i )
        {
            this->rc.push_back( new double[ rBin + 1 ] );
            std::memset( this->rc[ i ], 0, ( rBin + 1 ) * sizeof( double ) );
        }
        this->rs             = new double[ rBin + 1 ];
        double logr_bin_size = std::log10( rMax / rMin ) / rBin;
        double lgRMin        = std::log10( rMin );
        for ( int i = 0; i < rBin + 1; ++i )
            this->rs[ i ] = std::pow( 10, lgRMin + i * logr_bin_size );

        this->has_rc = true;
    }

    this->cal_rv( rMin, rMax, rBin, phiBin );

    // RC: average of velocity w.r.t. phi
    for ( int k = 0; k < this->compNum; ++k )
        for ( int i = 0; i < rBin; ++i )
        {
            double sum = 0;
            for ( int j = 0; j < phiBin; ++j )
            {
                sum += this->rv[ k ][ i * phiBin + j ];
            }
            this->rc[ k ][ i ] = sum / phiBin;  // average the velocity w.r.t. phi
        }
}

}  // namespace post_ana
