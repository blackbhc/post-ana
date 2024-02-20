#ifndef h5io_HPP
#define h5io_HPP
#include "hdf5.h"
#include <cstring>
#include <iostream>
#include <vector>
using std::vector;

namespace post_ana {
class h5io
{
public:
    h5io( std::string sim_data, std::string analysis_data );
    ~h5io();
    void read_datasets( vector< int >& partNums, vector< double* >& coordinates,
                        vector< double* >& masses );
    void write_results( vector< double >& rs, vector< vector< double > >& rv );

private:
    hid_t simFile = -1, anaResFile = -1;  // file id: simulation file and analysis result file
    int parttype_offset = 0;  // the offset of the particle type, if there is 0 number gas component
};

}  // namespace post_ana

#endif  // h5io_HPP
