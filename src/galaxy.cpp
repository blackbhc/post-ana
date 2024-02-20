#include "../include/galaxy.hpp"
#include <algorithm>
#include <cstring>
namespace post_ana {
galaxy::galaxy( std::string& filename, std::vector< int > disk_ids )
{
    // TODO: read the data from the file
    ( void )filename;
    this->compNum = 1;
    for ( int i = 0; i < this->compNum; ++i )
    {
        int     comp_part_num = 100;
        double* comp_coord    = new double[ comp_part_num * 3 ];
        double* comp_mass     = new double[ comp_part_num ];
        // TODO: read the data from the file
        post_ana::gravity* comp_grav =
            new post_ana::gravity( comp_coord, comp_mass, comp_part_num );
        this->coordinates.push_back( comp_coord );
        this->masses.push_back( comp_mass );
        this->gravity.push_back( comp_grav );
        this->partNum.push_back( comp_part_num );
    }
    this->disk_ids = disk_ids;
}

galaxy::~galaxy()
{
    for ( int i = 0; i < this->compNum; ++i )
    {
        delete this->gravity[ i ];
        delete[] this->coordinates[ i ];
        delete[] this->masses[ i ];
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

double galaxy::cal_fdisk( double Rd )
{
    double Rc = 2.2 * Rd;
    if ( !this->has_rc )  // make sure the rotation curve is calculated
    {
        throw std::runtime_error( "Rotation curve is not calculated yet, call cal_rc first before "
                                  "calculating the disk mass fraction" );
    }

    // check whether Rc is in the range of the rotation curve
    if ( Rc < this->rs[ 0 ] || Rc > this->rs[ this->binSize[ 0 ] - 1 ] )
    {
        throw std::runtime_error( "2.2*Rd is out of the range of the rotation curve" );
    }

    // find the left bin of Rc for interpolation
    int left_loc = 0;
    for ( left_loc = 0; left_loc < this->binSize[ 0 ]; ++left_loc )
    {
        if ( this->rs[ left_loc + 1 ] > Rc )
            break;
    }
    double delta_r = Rc - this->rs[ left_loc ];  // the delta r between Rc and the left bin
    double interpolate_rc;

    double sum      = 0;
    double disk_sum = 0;
    for ( int k = 0; k < this->compNum; ++k )
    {
        interpolate_rc = this->rc[ k ][ left_loc ]
                         + ( this->rc[ k ][ left_loc + 1 ] - this->rc[ k ][ left_loc ] ) * delta_r
                               / ( this->rs[ left_loc + 1 ] - this->rs[ left_loc ] );
        sum += interpolate_rc * interpolate_rc;

        // if the component is a disk
        if ( std::find( this->disk_ids.begin(), this->disk_ids.end(), k ) != this->disk_ids.end() )
        {
            disk_sum += interpolate_rc * interpolate_rc;
        }
    }

    return disk_sum / sum;
}

}  // namespace post_ana
