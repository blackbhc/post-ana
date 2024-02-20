#ifndef GALAXY_HPP
#define GALAXY_HPP
#include "aligner.hpp"
#include "derivatives.hpp"
#include "gravity.hpp"
#include <vector>

#define V_FACTOR 0.97779222  // the factor to convert the velocity unit from kpc/Gyr to km/s

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
    void                    cal_rc( double r_min, double r_max, int rBin,
                                    int phiBin );  // calculate the rotation curve
    void                    cal_rv( double r_min, double r_max, int rBin,
                                    int phiBin );  // calculate the rotation curve
    inline vector< double > get_rs()
    {
        vector< double > Rs;
        for ( int i = 0; i < this->binSize[ 0 ]; ++i )
        {
            Rs.push_back( this->rs[ i ] );
        }
        return Rs;
    }

    inline vector< vector< double > > get_rvs()
    {
        vector< vector< double > > RVs;
        for ( int i = 0; i < this->compNum; ++i )
        {
            vector< double > rv;
            for ( int j = 0; j < this->binSize[ 0 ] * this->binSize[ 1 ]; ++j )
            {
                rv.push_back( this->rv[ i ][ j ] * V_FACTOR );
            }
            RVs.push_back( rv );
        }
        return RVs;
    }

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
