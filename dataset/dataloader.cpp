#include "dataloader.h"

int main()
{
    std::vector<uint64_t> data = load_data<uint64_t>("fb_16M_uint64");
    write_data_txt<uint64_t>(data, "fb_16M_uint64.txt");

    data = load_data<uint64_t>("osm_cellids_16M_uint64");
    write_data_txt<uint64_t>(data, "osm_cellids_16M_uint64.txt");

    return 0;
}