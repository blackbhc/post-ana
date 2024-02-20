#ifndef GALAXY_HPP
#define GALAXY_HPP
#include "aligner.hpp"
#include "derivatives.hpp"
#include "gravity.hpp"
namespace post_ana {

class galaxy
{
public:
    galaxy(
        std::string&       filename,
        std::vector< int > disk_ids );  // NOTE: the disk ids should be the real ids in the file,
                                        // e.g. if disk+halo only, it should be 1 in general
    ~galaxy();
    // APIs for the analysis
    void   cal_rc( double r_min, double r_max, int rBin,
                   int phiBin );  // calculate the rotation curve
    void   cal_rv( double r_min, double r_max, int rBin,
                   int phiBin );    // calculate the rotation curve
    double cal_fdisk( double Rd );  // calculate the disk mass fraction

private:
    std::vector< int >                disk_ids;
    post_ana::aligner*                aligner     = nullptr;
    post_ana::derivatives*            derivatives = nullptr;
    int                               compNum     = 0;
    std::vector< post_ana::gravity* > gravity;
    std::vector< double* >            masses;
    std::vector< double* >            coordinates;
    std::vector< double* >            rv;
    std::vector< double* >            rc;
    double*                           rs;  // the radius of the RC
    std::vector< int >                partNum;
    std::vector< int >                binSize = { 0, 0 };  // rBin, phiBin of the rv grids
    bool                              aligned = false;
    bool                              has_rc  = false;
    void                              ensure_aligned();
};

}  // namespace post_ana
#endif  // GALAXY_HPP
