#ifndef ALIGNER_HPP
#define ALIGNER_HPP
namespace post_ana {
class aligner
{
public:
    aligner()  = default;
    ~aligner() = default;
    // align the possible bar to the coordinate axes
    void align( double* coordinates, double* masses, int particleNumber );

private:
    // recenter the particles to the center of mass
    void          recenter( double* coordinates, double* masses, int particleNumber );
    inline double norm( double* vec );
    inline double error( double* vec1, double* vec2 );
};
}  // namespace post_ana
#endif
