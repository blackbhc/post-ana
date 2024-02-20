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

private:
    void  create_datasets();
    hid_t simFile = -1, anaResFile = -1;       // file id: simulation file and analysis result file
    hid_t rc_id = -1, rs_id = -1;              // dataset id: RC and RS
    hid_t rc_space_id = -1, rs_space_id = -1;  // dataspace id: RC and RS
};

}  // namespace post_ana

#endif  // h5io_HPP
