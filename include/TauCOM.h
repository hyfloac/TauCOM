#pragma once

#include <vector>
#include <string>


#ifdef _WIN32
  #define TAUCOM_EXPORT __declspec(dllexport)
#else
  #define TAUCOM_EXPORT
#endif

TAUCOM_EXPORT void TauCOM();
TAUCOM_EXPORT void TauCOM_print_vector(const std::vector<std::string> &strings);
