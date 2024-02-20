#include "../include/h5io.hpp"
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
    // H5Dclose( this->rc_id );
    // H5Dclose( this->rs_id );
    // H5Sclose( this->rc_space_id );
    // H5Sclose( this->rs_space_id );
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
    int offset = 0;         // the offset of the particle type, if there is 0 number gas component
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
        offset = 1;

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
        string  partTypeName = "PartType" + std::to_string( i + offset );
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

void h5io::create_datasets()
{
    // create the datasets in the analysis result file
    // Datasets: RC, RS with the same size, 1D array
    hsize_t dims[ 1 ] = { 1 };
    hid_t   rc_space  = H5Screate_simple( 1, dims, NULL );
    hid_t   rs_space  = H5Screate_simple( 1, dims, NULL );
    hid_t rc_set = H5Dcreate( this->anaResFile, "Rotation Velocities", H5T_NATIVE_DOUBLE, rc_space,
                              H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
    hid_t rs_set = H5Dcreate( this->anaResFile, "Radiuses", H5T_NATIVE_DOUBLE, rs_space,
                              H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT );
    this->rc_id  = rc_set;
    this->rs_id  = rs_set;
    this->rc_space_id = rc_space;
    this->rs_space_id = rs_space;
}
}  // namespace post_ana
