#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <stdexcept>
#include <typeinfo>

#include <winsock2.h>
#include <windows.h>

#include "../src/serialstream.hpp"

int main(int argc, char* argv[]) try {
    using namespace std;
    using namespace org::sqg;

    string const &cfg = "baud=921600 parity=N data=8 stop=1 to=on xon=off odsr=off octs=on dtr=on rts=on idsr=off";

    ::DCB dcb = {0};
    dcb.DCBlength = sizeof(dcb);

    // BOOL WINAPI BuildCommDCB(
    //   _In_  LPCTSTR lpDef,
    //   _Out_ LPDCB   lpDCB
    // );

    LPCTSTR lpDef = cfg.c_str();
    if (!::BuildCommDCB(lpDef, &dcb))
        throw std::runtime_error(win32_error_msg(::GetLastError()));
    clog << dcb << endl;
    return EXIT_SUCCESS;
} catch (std::exception const &e) {
    std::cerr << "[C++ Exception] " << e.what() << std::endl;
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "[C++ Exception] <UNKNOWN CAUSE>" << std::endl;
    return EXIT_FAILURE;
}
