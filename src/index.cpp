#include <sdsl/suffix_arrays.hpp>
#include <fstream>

using namespace sdsl;

int main() {
    csa_wt<> fm_index;
    construct_im(fm_index, "mississippi!", 1);
    std::cout << "'si' occurs " << count(fm_index,"si") << " times.\n";
}
