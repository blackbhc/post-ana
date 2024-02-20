// numerical derivatives
#include "../include/derivatives.hpp"

// define a class for numerical derivatives
namespace post_ana {
// define a class for numerical derivatives
std::vector< double > derivatives::dy_dx( double* x, double* y, int n )
{
    if ( n < 2 )  // check the number of data points
    {
        std::cerr << "Error: the number of data points should be at least 2." << std::endl;
        exit( 1 );
    }

    std::vector< double > dy_dx( n - 1 );
    for ( int i = 0; i < n - 1; ++i )
    {
        dy_dx[ i ] = ( y[ i + 1 ] - y[ i ] ) / ( x[ i + 1 ] - x[ i ] );
    }

    return dy_dx;
}

std::vector< double > derivatives::dy_dx( double* y, int n, double x_min, double x_max )
{
    if ( n < 2 )  // check the number of data points
    {
        std::cerr << "Error: the number of data points should be at least 2." << std::endl;
        exit( 1 );
    }

    std::vector< double > x( n );
    double                dx = ( x_max - x_min ) / ( n - 1 );
    for ( int i = 0; i < n; ++i )
    {
        x[ i ] = x_min + i * dx;
    }

    return dy_dx( x.data(), y, n );
}

}  // namespace post_ana
