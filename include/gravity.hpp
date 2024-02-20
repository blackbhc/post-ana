// gravity calculation by direct summation
#ifndef GRAVITY_HPP
#define GRAVITY_HPP
#include "derivatives.hpp"
#include <cmath>
#include <cstring>

#define G 4.498502151469554e-06  // gravitational constant in kpc^3 Msun^-1 Gyr^-2
namespace post_ana {
class gravity
{
public:
    gravity( double* coordinates, double* masses, int particleNumber );
    ~gravity() = default;
    // calculate the potential and force at a given position or positions
    void force( double* pos, double* force );
    void potential( double* pos, double* pot );
    void forces( double* poses, double* forces, int n );
    void potentials( double* poses, double* pots, int n );

private:
    double*       coordinates;
    double*       masses;
    int           particleNumber;
    inline double distance( double* pos1, double* pos2 );
};
}  // namespace post_ana
#endif
