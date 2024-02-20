#include "../include/galaxy.hpp"
#include "../include/aligner.hpp"
#include <algorithm>
#include <cstring>
namespace post_ana {
galaxy::galaxy( vector< double* > coordinates, vector< double* > masses, vector< int > partNums )
{
    // TODO: read the data from the file
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

void galaxy::cal_rv( double r_min, double r_max, int rBin, int phiBin )
{
    static bool first_call = true;
    this->ensure_aligned();

    if ( first_call )
    {
        // allocate memory for the velocity curve at the first call
        for ( int i = 0; i < this->compNum; ++i )
        {
            this->rv.push_back( new double[ rBin * phiBin ] );
            std::memset( this->rv[ i ], 0, rBin * phiBin * sizeof( double ) );
            this->binSize = { rBin, phiBin };
        }
        first_call = false;
    }

    double logr_bin_size = std::log10( r_max / r_min ) / rBin;
    double phi_bin_size  = 2 * M_PI / phiBin;

    double pos[ 3 ]   = { 0, 0, 0 };  // temporary position
    double force[ 3 ] = { 0, 0, 0 };  // temporary force
    // norm of vector
    auto norm = []( double* vec ) -> double {
        return std::sqrt( vec[ 0 ] * vec[ 0 ] + vec[ 1 ] * vec[ 1 ] + vec[ 2 ] * vec[ 2 ] );
    };

    for ( int k = 0; k < this->compNum; ++k )
        for ( int i = 0; i < rBin; ++i )
        {
            for ( int j = 0; j < phiBin; ++j )
            {
                pos[ 0 ] =
                    ( r_min + std::pow( 10, i * logr_bin_size ) ) * std::cos( j * phi_bin_size );
                pos[ 1 ] =
                    ( r_min + std::pow( 10, i * logr_bin_size ) ) * std::sin( j * phi_bin_size );
                pos[ 2 ] = 0;
                this->gravity[ k ]->force( pos, force );
                this->rv[ k ][ i * phiBin + j ] = std::sqrt( norm( force ) * norm( pos ) );
            }
        }
}

void galaxy::cal_rc( double r_min, double r_max, int rBin, int phiBin )
{
    static bool first_call = true;
    if ( first_call )
    {
        // allocate memory for the velocity curve at the first call
        for ( int i = 0; i < this->compNum; ++i )
        {
            this->rc.push_back( new double[ rBin ] );
            std::memset( this->rc[ i ], 0, rBin * sizeof( double ) );
        }
        this->rs             = new double[ rBin ];
        double logr_bin_size = std::log10( r_max / r_min ) / rBin;
        for ( int i = 0; i < rBin; ++i )
            this->rs[ i ] = r_min + std::pow( 10, i * logr_bin_size );

        this->has_rc = true;
    }

    this->cal_rv( r_min, r_max, rBin, phiBin );

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
