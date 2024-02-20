#include "../include/h5io.hpp"
#include "H5public.h"
#include <iostream>
#include <string>
using std::string;

namespace post_ana {

h5io::h5io( std::string simFile, std::string anaResFile )
{
    this->simFile    = H5Fopen( simFile.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT );
    this->anaResFile = H5Fcreate( anaResFile.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT );
}

h5io::~h5io()
{
    H5Fclose( this->simFile );
    H5Fclose( this->anaResFile );
}

void h5io::read_datasets( vector< int >& partNums, vector< double* >& coordinates,
                          vector< double* >& masses )
{
    // check whether the given vectors are uninitialized
    if ( partNums.size() != 0 || coordinates.size() != 0 || masses.size() != 0 )
    {
        std::cerr << "The given vectors should be uninitialized." << std::endl;
        return;
    }

    partNums.resize( 50 );  // 50 is large enough for the number of particles
    for ( int i = 0; i < 50; i++ )
    {
        partNums[ i ] = -1;
    }

    hid_t header  = H5Gopen( this->simFile, "/Header", H5P_DEFAULT );
    hid_t attr_id = H5Aopen( header, "NumPart_ThisFile", H5P_DEFAULT );
    H5Aread( attr_id, H5T_NATIVE_INT, &partNums[ 0 ] );

    int           types = 0;  // the number of effective particle types
    vector< int > effectivePartNums;
    for ( int i = 0; i < 50; i++ )
    {
        if ( partNums[ i ] > 0 )
        {
            effectivePartNums.push_back( partNums[ i ] );
        }
    }
    types = ( int )effectivePartNums.size();

    if ( partNums[ 0 ] == 0 )  // if there is no gas component, add an offset for PartTypes
        this->parttype_offset = 1;

    // resize the vectors
    partNums.resize( types );
    coordinates.resize( types );
    masses.resize( types );

    for ( int i = 0; i < types; i++ )
    {
        partNums[ i ] = effectivePartNums[ i ];
    }

    // read the coordinates and masses
    for ( int i = 0; i < types; i++ )
    {
        string  partTypeName = "PartType" + std::to_string( i + this->parttype_offset );
        hid_t   component    = H5Gopen( this->simFile, partTypeName.c_str(), H5P_DEFAULT );
        hid_t   coordSet     = H5Dopen( component, "Coordinates", H5P_DEFAULT );
        hid_t   massSet      = H5Dopen( component, "Masses", H5P_DEFAULT );
        hid_t   coordSpace   = H5Dget_space( coordSet );
        hid_t   massSpace    = H5Dget_space( massSet );
        hsize_t dims[ 2 ];
        H5Sget_simple_extent_dims( coordSpace, dims, NULL );
        coordinates[ i ] = new double[ dims[ 0 ] * dims[ 1 ] ];
        masses[ i ]      = new double[ dims[ 0 ] ];
        H5Dread( coordSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, coordinates[ i ] );
        H5Dread( massSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, masses[ i ] );
        H5Dclose( coordSet );
        H5Dclose( massSet );
        H5Sclose( coordSpace );
        H5Sclose( massSpace );
        H5Gclose( component );
    }

    H5Aclose( attr_id );
    H5Gclose( header );
}

void h5io::write_results( vector< double >& rs, vector< vector< double > >& rvs )
{
    // create the datasets in the analysis result file
    // Datasets: RC, RS with the same size, 1D array
    int     rBin         = ( int )rs.size();
    int     phiBin       = ( int )rvs[ 0 ].size() / rBin;
    hsize_t dims_1d[ 1 ] = { ( hsize_t )rBin };
    hsize_t dims_2d[ 2 ] = { ( hsize_t )rBin, ( hsize_t )phiBin };
    hid_t   rSet =
        H5Dcreate( this->anaResFile, "Radius", H5T_NATIVE_DOUBLE,
                   H5Screate_simple( 1, dims_1d, NULL ), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );

    H5Dwrite( rSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rs.data() );
    H5Dclose( rSet );

    hid_t rvGroup =
        H5Gcreate( this->anaResFile, "RV Square", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );

    for ( int i = 0; i < ( int )rvs.size(); ++i )  // for each component
    {
        string datasetName = "PartType" + std::to_string( i + this->parttype_offset );
        hid_t  vSet        = H5Dcreate( rvGroup, datasetName.c_str(), H5T_NATIVE_DOUBLE,
                                        H5Screate_simple( 2, dims_2d, NULL ), H5P_DEFAULT, H5P_DEFAULT,
                                        H5P_DEFAULT );
        H5Dwrite( vSet, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, rvs[ i ].data() );
        H5Dclose( vSet );
    }

    H5Gclose( rvGroup );
}
}  // namespace post_ana
