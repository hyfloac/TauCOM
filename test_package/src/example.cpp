#include "TauCOM.h"
#include <vector>
#include <string>

int main() {
    TauCOM();

    std::vector<std::string> vec;
    vec.push_back("test_package");

    TauCOM_print_vector(vec);
}
