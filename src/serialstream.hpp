#if defined(_MSC_VER) && _MSC_VER > 1200
#   pragma once
#endif
#ifndef ORG_SQG_SERIALSTREAM_H_INCLUDED
#define ORG_SQG_SERIALSTREAM_H_INCLUDED

#include <cstring>
#ifdef HAVE_CONFIG_H
#   include <config.h>
#   ifdef HAVE_STDINT_H
#       include <cstdint>
#   endif
#   ifdef HAVE_SYS_SOCKET_H
#       include <sys/socket.h>
#   endif
#   ifdef HAVE_NETINET_IN_H
#       include <netinet/in.h>
#   endif
#   ifdef HAVE_NETDB_H
#       include <netdb.h>
#   endif
#   ifdef HAVE_UNISTD_H
#       include <unistd.h>
#   endif
#   ifdef __MINGW32__
#       include <winsock2.h>
#       include <windows.h>
#       include <ws2tcpip.h>
#   endif
#else
#   ifdef _MSC_VER
#       include <winsock2.h>
#       include <windows.h>
#       include <ws2tcpip.h>
#   endif
#endif

#include <streambuf>
#include <istream>
#include <ostream>
#include <vector>
#include <algorithm>

namespace org {
    namespace sqg {

        std::ostream& operator << (std::ostream &os, ::DCB const &dcb) {
            return os
                << "{ DCBlength = "  << static_cast<int>(dcb.DCBlength)
                << ", BaudRate = "  << static_cast<int>(dcb.BaudRate)
                << ", fBinary = "  << static_cast<int>(dcb.fBinary)
                << ", fParity = "  << static_cast<int>(dcb.fParity)
                << ", fOutxCtsFlow = "  << static_cast<int>(dcb.fOutxCtsFlow)
                << ", fOutxDsrFlow = "  << static_cast<int>(dcb.fOutxDsrFlow)
                << ", fDtrControl = "  << static_cast<int>(dcb.fDtrControl)
                << ", fDsrSensitivity = "  << static_cast<int>(dcb.fDsrSensitivity)
                << ", fTXContinueOnXoff = "  << static_cast<int>(dcb.fTXContinueOnXoff)
                << ", fOutX = "  << static_cast<int>(dcb.fOutX)
                << ", fInX = "  << static_cast<int>(dcb.fInX)
                << ", fErrorChar = "  << static_cast<int>(dcb.fErrorChar)
                << ", fNull = "  << static_cast<int>(dcb.fNull)
                << ", fAbortOnError = "  << static_cast<int>(dcb.fAbortOnError)
                << ", Parity = "  << static_cast<int>(dcb.Parity)
                << ", ByteSize = "  << static_cast<int>(dcb.ByteSize)
                << ", StopBits = "  << static_cast<int>(dcb.StopBits)
                << ", XonChar = " << static_cast<int>(dcb.XonChar)
                << ", XoffChar = " << static_cast<int>(dcb.XoffChar)
                << ", ErrorChar = " << static_cast<int>(dcb.ErrorChar)
                << ", EofChar = " << static_cast<int>(dcb.EofChar)
                << ", EvtChar = " << static_cast<int>(dcb.EvtChar)
                << " }"
                ;
        }

        std::string win32_error_msg(DWORD code) {
            using namespace std;
            string msg = "<UNKNOWN ERROR>";
            LPSTR text = 0;
            DWORD ret = ::FormatMessageA(
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    0,
                    code,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    reinterpret_cast<LPSTR>(&text),
                    0,
                    0);
            if(ret < 0 || !text)
                return msg;
            msg.assign(text, ret);
            ::LocalFree(text);
            return msg;
        }

        template <typename CharT, typename Traits = std::char_traits<CharT> >
        class basic_serialbuf : public std::basic_streambuf<CharT, Traits> {
        public:
            typedef std::basic_streambuf<CharT, Traits> super;
            typedef basic_serialbuf<CharT, Traits> self;

            typedef typename super::char_type char_type;
            typedef typename super::traits_type traits_type;
            typedef typename super::int_type int_type;
            typedef typename super::pos_type pos_type;
            typedef typename super::off_type off_type;

        protected:
            static const int CHAR_SIZE = sizeof(char_type);
#ifdef BUFSIZ
            static const int SIZE = BUFSIZ;
#else
            static const int SIZE = 1024;
#endif
            char_type _M_obuf[SIZE];
            std::vector<char_type> _M_ibuf;

            ::HANDLE _M_handle;

        public:
            basic_serialbuf() : _M_handle(INVALID_HANDLE_VALUE) {
                super::setp(_M_obuf, _M_obuf + (SIZE - 1));
                super::setg(&_M_ibuf[0], &_M_ibuf[0], &_M_ibuf[0]);
            }

            virtual ~basic_serialbuf() { sync(); }

            void set_serial(::HANDLE serialport) { this->_M_handle = serialport; }
            ::HANDLE get_serial() { return this->_M_handle; }

        protected:
            int_type output_buffer() {
                int_type num = super::pptr() - super::pbase();
                if (num == 0)
                    return num;
                //BOOL WINAPI WriteFile(
                //  _In_        HANDLE       hFile,
                //  _In_        LPCVOID      lpBuffer,
                //  _In_        DWORD        nNumberOfBytesToWrite,
                //  _Out_opt_   LPDWORD      lpNumberOfBytesWritten,
                //  _Inout_opt_ LPOVERLAPPED lpOverlapped
                //);

                ::HANDLE hFile = _M_handle;
                ::LPCVOID lpBuffer = _M_obuf;
                ::DWORD nNumberOfBytesToWrite = num * CHAR_SIZE;
                ::DWORD nNumberOfBytesWritten = 0;
                ::LPOVERLAPPED lpOverlapped = NULL;

                if (::WriteFile(hFile,
                            lpBuffer,
                            nNumberOfBytesToWrite,
                            &nNumberOfBytesWritten,
                            lpOverlapped) == FALSE) {
                    std::cerr << "WriteFile failed: " << win32_error_msg(::GetLastError()) << std::endl;
                    return traits_type::eof();
                }
                if (nNumberOfBytesWritten != nNumberOfBytesToWrite)
                    return traits_type::eof();
                super::pbump(-num);
                return num;
            }

            virtual int_type overflow(int_type c) {
                if (c != traits_type::eof()) {
                    *super::pptr() = c;
                    super::pbump(1);
                }

                if (output_buffer() == traits_type::eof())
                    return traits_type::eof();
                return c;
            }

            virtual int sync() {
                if (output_buffer() == traits_type::eof())
                    return traits_type::eof();
                return 0;
            }

            virtual int_type underflow() {
                if (super::gptr() < super::egptr())
                    return *super::gptr();

                //BOOL WINAPI ReadFile(
                //  _In_        HANDLE       hFile,
                //  _Out_       LPVOID       lpBuffer,
                //  _In_        DWORD        nNumberOfBytesToRead,
                //  _Out_opt_   LPDWORD      lpNumberOfBytesRead,
                //  _Inout_opt_ LPOVERLAPPED lpOverlapped
                //);

                char_type buffer[0xff] = {0};
                HANDLE hFile = _M_handle;
                LPVOID lpBuffer = reinterpret_cast<LPVOID>(&buffer[0]);
                DWORD nNumberOfBytesToRead = sizeof(buffer);
                DWORD nNumberOfBytesRead = 0;
                LPOVERLAPPED lpOverlapped = NULL;

                _M_ibuf.clear();
                super::setg(&_M_ibuf[0], &_M_ibuf[0], &_M_ibuf[0] + _M_ibuf.size());
                do {
                    if (::ReadFile(hFile,
                                lpBuffer,
                                nNumberOfBytesToRead,
                                &nNumberOfBytesRead,
                                lpOverlapped) == FALSE) {
                        std::cerr << "ReadFile failed: " << win32_error_msg(::GetLastError()) << std::endl;
                        return traits_type::eof();
                    }
                    if (nNumberOfBytesRead == 0) {
                        std::cerr << "encounter EOF" << std::endl;
                        return traits_type::eof();
                    }
                    if (nNumberOfBytesRead > 0) {
                        int n = _M_ibuf.size();
                        _M_ibuf.resize(n + nNumberOfBytesRead);
                        // std::clog << "copy " << nNumberOfBytesRead << " bytes to _M_ibuf at position " << n << std::endl;
                        std::copy(&buffer[0], &buffer[0] + nNumberOfBytesRead, &_M_ibuf[0] + n);
                        super::setg(&_M_ibuf[0], &_M_ibuf[0], &_M_ibuf[0] + _M_ibuf.size());
                    }
                } while (nNumberOfBytesRead == nNumberOfBytesToRead);
                return *super::gptr();
            }
        };

        typedef basic_serialbuf<char> serialbuf;
        // FIXME The char_type and CHAR_SIZE may lead to crash!!!
        typedef basic_serialbuf<wchar_t> wserialbuf;

        template <typename CharT, typename Traits = std::char_traits<CharT> >
        class basic_serialstream : public std::basic_iostream<CharT, Traits> {
        public:
            typedef std::basic_iostream<CharT, Traits> super;
            typedef basic_serialbuf<CharT, Traits> streambuffer;
            typedef typename super::char_type char_type;
            typedef typename super::traits_type traits_type;
            typedef typename super::int_type    int_type;
            typedef typename super::pos_type    pos_type;
            typedef typename super::off_type    off_type;

        protected:
            streambuffer _M_buffer;

        public:
            basic_serialstream() : super(&_M_buffer) { }

            explicit basic_serialstream(::HANDLE handle) : super(&_M_buffer) {
                _M_buffer.set_serial(handle);
            }

            virtual ~basic_serialstream() {
                this->close();
            }

            void close() {
                if (_M_buffer.get_serial() != INVALID_HANDLE_VALUE) {
                    std::clog << "Closing serial port handle..." << std::endl;
                    ::CloseHandle(_M_buffer.get_serial());
                    _M_buffer.set_serial(INVALID_HANDLE_VALUE);
                }
                super::clear();
            }

            bool open(std::string const &serialport, int baudrate) {
                this->close();

                //HANDLE WINAPI CreateFile(
                //  _In_     LPCTSTR               lpFileName,
                //  _In_     DWORD                 dwDesiredAccess,
                //  _In_     DWORD                 dwShareMode,
                //  _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
                //  _In_     DWORD                 dwCreationDisposition,
                //  _In_     DWORD                 dwFlagsAndAttributes,
                //  _In_opt_ HANDLE                hTemplateFile
                //);

                LPCSTR  lpFileName = serialport.c_str(); // serial port name.
                DWORD   dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
                DWORD   dwShareMode = 0; // serial port MUST be no share mode.
                LPSECURITY_ATTRIBUTES lpSecurityAttributes = NULL;
                DWORD dwCreationDisposition = OPEN_EXISTING;
                DWORD dwFlagsAndAttributes = 0;
                HANDLE hTemplateFile = NULL;

                ::HANDLE handle = ::CreateFile(
                        lpFileName,
                        dwDesiredAccess,
                        dwShareMode,
                        lpSecurityAttributes,
                        dwCreationDisposition,
                        dwFlagsAndAttributes,
                        hTemplateFile);
                if (handle == INVALID_HANDLE_VALUE) {
                    std::cerr << "CreateFile failed: " << win32_error_msg(::GetLastError()) << std::endl;
                    super::setstate(std::ios::failbit);
                    return false;
                }

                //BOOL WINAPI GetCommState(
                //  _In_    HANDLE hFile,
                //  _Inout_ LPDCB  lpDCB
                //);

                ::DCB dcb;
                memset(&dcb, 0, sizeof(dcb));
                dcb.DCBlength = sizeof(dcb);
                if (!::GetCommState(handle, &dcb)) {
                    std::cerr << "GetCommState failed: " << win32_error_msg(::GetLastError()) << std::endl;
                    super::setstate(std::ios::failbit);
                    return false;
                }
                std::clog << "Port = " << serialport << std::endl;
                std::clog << dcb << std::endl;

                // manually configure the serial port.
                dcb.DCBlength = sizeof(dcb);
                dcb.BaudRate = 921600;
                dcb.fBinary = 1;
                dcb.fParity = 0;
                dcb.fOutxCtsFlow = 1;
                dcb.fOutxDsrFlow = 0;
                dcb.fDtrControl = 1;
                dcb.fDsrSensitivity = 0;
                dcb.fTXContinueOnXoff = 1;
                dcb.fOutX = 0;
                dcb.fInX = 0;
                dcb.fErrorChar = 0;
                dcb.fNull = 0;
                dcb.fAbortOnError = 1;
                dcb.Parity = NOPARITY;
                dcb.ByteSize = 8;
                dcb.StopBits = ONESTOPBIT;
                dcb.XonChar = static_cast<char>(17);
                dcb.XoffChar = static_cast<char>(19);
                dcb.ErrorChar = static_cast<char>(0);
                dcb.EofChar = static_cast<char>(0);
                dcb.EvtChar = static_cast<char>(0);

                std::clog << dcb << std::endl;
                if (!::SetCommState(handle, &dcb)) {
                    super::setstate(std::ios::failbit);
                    std::cerr << "SetCommState failed: " << win32_error_msg(::GetLastError()) << std::endl;
                    return false;
                }
                if (!::GetCommState(handle, &dcb)) {
                    std::cerr << "GetCommState failed: " << win32_error_msg(::GetLastError()) << std::endl;
                    super::setstate(std::ios::failbit);
                    return false;
                }
                std::clog << dcb << std::endl;
                _M_buffer.set_serial(handle);
                return !!(*this);
            }
        };

        typedef basic_serialstream<char> serialstream;
        // FIXME The char_type and CHAR_SIZE may lead to crash!!!
        typedef basic_serialstream<wchar_t> wserialstream;
    }
}

#endif // ORG_SQG_SERIALSTREAM_H_INCLUDED

