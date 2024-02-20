// numerical derivatives
#ifndef DERIVATIVES_HPP
#define DERIVATIVES_HPP
#include <iostream>
#include <vector>
namespace post_ana {
// define a class for numerical derivatives
class derivatives
{
public:
    derivatives()  = default;
    ~derivatives() = default;
    // first derivative
    std::vector< double > dy_dx( double* x, double* y, int n );
    // n: number of data points, be careful with the size of the array
    std::vector< double > dy_dx( double* y, int n, double x_min, double x_max );
};
}  // namespace post_ana
#endif
