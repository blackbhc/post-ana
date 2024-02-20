#ifndef GALAXY_HPP
#define GALAXY_HPP
#include "aligner.hpp"
#include "derivatives.hpp"
#include "gravity.hpp"
#include <vector>
using std::vector;
namespace post_ana {

class galaxy
{
public:
    galaxy( vector< double* > coordinates, vector< double* > masses, vector< int > partNums );
    // NOTE: the disk ids should be the real ids in the file, e.g. if disk+halo only, it should be 1
    // in general.
    ~galaxy();
    // APIs for the analysis
    void cal_rc( double r_min, double r_max, int rBin,
                 int phiBin );  // calculate the rotation curve
    void cal_rv( double r_min, double r_max, int rBin,
                 int phiBin );  // calculate the rotation curve

private:
    vector< int >                disk_ids;
    post_ana::aligner*           aligner     = nullptr;
    post_ana::derivatives*       derivatives = nullptr;
    int                          compNum     = 0;
    vector< post_ana::gravity* > gravity;
    vector< double* >            masses;
    vector< double* >            coordinates;
    vector< double* >            rv;
    vector< double* >            rc;
    double*                      rs;  // the radius of the RC
    vector< int >                partNum;
    vector< int >                binSize = { 0, 0 };  // rBin, phiBin of the rv grids
    bool                         aligned = false;
    bool                         has_rc  = false;
    void                         ensure_aligned();
};

}  // namespace post_ana
#endif  // GALAXY_HPP
