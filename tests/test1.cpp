#include <cstdlib>
#include <iomanip>
#include <sstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <limits>
#include <vector>
#include <algorithm>

#include "../src/serialstream.hpp"

#define PHONE_NUMBER "+86123456789AB"

namespace org {
    namespace sqg {

        template <typename U, typename V> U sstream_cast(V const&);

        template <typename U, typename V>
        U sstream_cast(V const &v) {
            U u;
            std::stringstream ss;
            ss << v;
            ss >> u;
            return u;
        }

        class url {
            public:
                url(std::string const &scheme,
                        std::string const &user, std::string const &password,
                        std::string const &host, std::size_t port,
                        std::string const &path,
                        std::string const &query,
                        std::string const &hash)
                    :_M_scheme(scheme)
                     , _M_user(user), _M_password(password)
                     , _M_host(host), _M_port(port)
                     , _M_path(path)
                     , _M_query(query)
                     , _M_hash(hash)
                {
                }
                virtual ~url() {}

            private:
                static void parse_authority(std::string const &authority,
                        std::string &user, std::string &password) {
                    using namespace std;
                    string::size_type pos(0);
                    string::size_type loc(string::npos);

                    if ((loc = authority.find(":", pos)) == string::npos) {
                        user = authority;
                        password = "";
                    } else {
                        user = authority.substr(pos, loc - pos);
                        password = authority.substr(loc + 1);
                    }
                }

                static void parse_service(std::string const &service,
                        std::string &host, std::size_t &port) {
                    using namespace std;
                    string::size_type pos(0);
                    string::size_type loc(string::npos);

                    if ((loc = service.find(":", pos)) == string::npos) {
                        host = service;
                        port = 0;
                    } else {
                        host = service.substr(pos, loc - pos);
                        port = sstream_cast<std::size_t>(service.substr(loc + 1));
                    }
                }

                static void parse_remote(std::string const &remote,
                        std::string &user, std::string &password,
                        std::string &host, std::size_t &port) {
                    using namespace std;
                    string::size_type pos(0);
                    string::size_type loc(string::npos);

                    string service;
                    string authority;

                    if ((loc = remote.find("@", pos)) == string::npos) {
                        service = remote.substr(loc + 1);
                        user = "";
                        password = "";
                        parse_service(service, host, port);
                    } else {
                        authority = remote.substr(pos, loc - pos);
                        service = remote.substr(loc + 1);
                        parse_authority(authority, user, password);
                        parse_service(service, host, port);
                    }
                }
            public:
                std::string scheme() const { return _M_scheme; }
                std::string user() const { return _M_user; }
                std::string password() const { return _M_password; }
                std::string host() const { return _M_host; }
                std::size_t port() const { return _M_port; }
                std::string path() const { return _M_path; }
                std::string query() const { return _M_query; }
                std::string hash() const { return _M_hash; }

                static url parse(std::string const &s) {
                    using namespace std;
                    string::size_type pos(0);
                    string::size_type loc(string::npos);

                    string scheme;
                    string remote, user, password, host;
                    std::size_t port;
                    string path, query, hash;

                    // URL
                    //   [scheme] :// [user]:[password]@[host]:[port] [path] ? [query] # [hash]
                    //   scheme:[//[user[:password]@]host[:port]][/path][?query][#fragment]
                    if ((loc = s.find("://", pos)) == std::string::npos) {
                        if ((loc = s.find(":/")) == std::string::npos)
                            throw std::runtime_error("no scheme found!");
                        scheme = s.substr(pos, loc - pos);
                        pos = loc + 2;
                        user = "";
                        password = "";
                        host = "";
                        port = 0;
                        if ((loc = s.find("?", pos) == string::npos)) {
                            if ((loc = s.find("#", pos)) == string::npos) {
                                path = s.substr(pos);
                                query = "";
                                hash = "";
                            } else {
                                path = s.substr(pos, loc - pos);
                                query = "";
                                hash = s.substr(loc + 1);
                            }
                        } else {
                            path = s.substr(pos, loc - pos);
                            pos = loc + 1;
                            if ((loc = s.find("#", pos)) == string::npos) {
                                query = s.substr(pos);
                                hash = "";
                            } else {
                                query = s.substr(pos, loc - pos);
                                hash = s.substr(loc + 1);
                            }
                        }
                    } else {
                        scheme = s.substr(pos, loc - pos);
                        pos = loc + 3;
                        if ((loc = s.find("/", pos)) == std::string::npos) {
                            remote = s.substr(pos);
                            parse_remote(remote, user, password, host, port);
                        } else {
                            remote = s.substr(pos, loc - pos);
                            parse_remote(remote, user, password, host, port);
                            pos = loc + 1;
                            if ((loc = s.find("?", pos)) == string::npos) {
                                if ((loc = s.find("#", pos)) == string::npos) {
                                    path = s.substr(pos);
                                    query = "";
                                    hash = "";
                                } else {
                                    path = s.substr(pos, loc - pos);
                                    query = "";
                                    hash = s.substr(loc + 1);
                                }
                            } else {
                                path = s.substr(pos, loc - pos);
                                pos = loc + 1;
                                if ((loc = s.find("#", pos)) == string::npos) {
                                    query = s.substr(pos);
                                    hash = "";
                                } else {
                                    query = s.substr(pos, loc - pos);
                                    hash = s.substr(loc + 1);
                                }
                            }
                        }
                    }
                    return url(scheme, user, password, host, port, path, query, hash);
                }
            private:
                std::string _M_scheme;
                std::string _M_user;
                std::string _M_password;
                std::string _M_host;
                std::size_t _M_port;
                std::string _M_path;
                std::string _M_query;
                std::string _M_hash;
        };

        std::ostream& operator << (std::ostream &os, url const &url) {
            os << url.scheme() << ":";
            if (url.host().length() > 0) {
                os << "//";
                if (url.user().length() > 0) {
                    os << url.user();
                    if (url.password().length() > 0)
                        os << ":" << url.password();
                    os << "@";
                }
                os << url.host();
                if (url.port() != 0)
                    os << ":" << url.port();
            }
            if (url.path().length() > 0 || url.query().length() > 0 || url.hash().length() > 0)
                os << "/" << url.path();
            if (url.query().length() > 0)
                os << "?" << url.query();
            if (url.hash().length() > 0)
                os << "#" << url.hash();
            return os;
        }

        template <typename CharT, typename Traits, typename Allocator>
        std::basic_istream<CharT, Traits>&
        ignore(std::basic_istream<CharT, Traits> &is, std::streamsize n,
                std::basic_string<CharT, Traits, Allocator> const& delim) {
            typedef std::basic_istream<CharT, Traits> istream;
            typedef std::basic_string<CharT, Traits, Allocator> string;
            typedef typename istream::char_type     char_type;
            typedef typename istream::traits_type   traits_type;

            if (delim.length() < 1)
                return is.ignore(n);

            char_type ch;
            std::vector<char_type> chars;
            do {
                is.ignore(n, delim[0]);
                chars.push_back(delim[0]);
                for (typename string::size_type i = 1, n = delim.size(); i < n && is.get(ch); ++i)
                    chars.push_back(ch);
                if (chars.size() != delim.length())
                    return is;
                if (traits_type::compare(&chars[0], &delim[0], delim.length()) == 0)
                    return is;
                if (!is)
                    return is;
                for (typename string::size_type i = 1, n = chars.size(); i < n && is.unget(); ++i)
                chars.clear();
            } while (true);
            return is;
        }
    }
}

bool ssend(org::sqg::serialstream &ss, std::string const &msg) {
    using namespace std;
    using namespace org::sqg;
    if (!!ss) {
        ss << msg << std::flush;
        std::clog << "[REQ] " << msg << std::endl;
    } else {
        std::clog << "IGNORE ssend " << msg << std::endl;
    }
    return !!ss;
}

bool srecv(org::sqg::serialstream &ss, std::string &msg) {
    using namespace std;
    using namespace org::sqg;
    char buffer[0xff] = {0};
    if (!ss)
        ss.clear();
    streamsize n = 0;
    ss.peek();
    do {
        n = ss.readsome(&buffer[0], sizeof(buffer));
        if (n == 0) {
            clog << "[RSP] <no response>" << endl;
            break;
        }
        *(&buffer[0] + n) = '\0';
        clog << "[RSP] " << &buffer[0] << endl;
    } while (n >= sizeof(buffer));
    return !!ss;
}

bool test_serial_sms(std::string const &name) {
    using namespace org::sqg;
    serialstream port;
    if (!port.open(name, 57600))
        throw std::runtime_error(name + " failed to be open!");
    std::string request, response;
    time_t epoch = time(NULL);
    ssend(port, "ATQ0V1E0\r"); srecv(port, response);
    ssend(port, "AT+CGMR\r"); srecv(port, response);
    ssend(port, "AT+GMI\r"); srecv(port, response);
    ssend(port, "AT+GMM\r"); srecv(port, response);
    ssend(port, "AT+GSN\r"); srecv(port, response);
    ssend(port, "AT+CIMI\r"); srecv(port, response);
    ssend(port, "AT+FCLASS=?\r"); srecv(port, response);
    ssend(port, "ATI1\r"); srecv(port, response);
    ssend(port, "ATI2\r"); srecv(port, response);
    ssend(port, "ATI3\r"); srecv(port, response);
    ssend(port, "ATI4\r"); srecv(port, response);
    ssend(port, "ATI5\r"); srecv(port, response);
    ssend(port, "ATI6\r"); srecv(port, response);
    ssend(port, "ATI7\r"); srecv(port, response);
    ssend(port, "AT\r"); srecv(port, response);
    ssend(port, "AT+CGREG?\r"); srecv(port, response);
    ssend(port, "AT+COPS?\r"); srecv(port, response);
    ssend(port, "AT+CMGF=1\r"); srecv(port, response);
    ssend(port, "AT+CMGS=\"" PHONE_NUMBER "\"\r"); srecv(port, response);
    ssend(port, "Hello,I'm robot!\x1a\r"); srecv(port, response);
    return true;
}

bool test_serial_telephone(std::string const &name) {
    using namespace org::sqg;
    serialstream port;
    if (!port.open(name, 57600))
        throw std::runtime_error(name + " failed to be open!");
    std::string request, response;
    time_t epoch = time(NULL);
    ssend(port, "AT\r");
    srecv(port, response);
    ssend(port, "AT+CGREG?\r");
    srecv(port, response);
    ssend(port, "AT+CGMR\r");
    srecv(port, response);
    ssend(port, "ATD" PHONE_NUMBER ";\r");
    srecv(port, response);
    return true;
}

bool test_serial_wappush(std::string const &name) {
    using namespace org::sqg;
    serialstream port;
    if (!port.open(name, 57600))
        throw std::runtime_error(name + " failed to be open!");
    std::string request, response;
    time_t epoch = time(NULL);
    ssend(port, "AT\r");
    srecv(port, response);
    ssend(port, "AT+CGREG?\r");
    srecv(port, response);
    ssend(port, "AT+CMGF=1\r");
    srecv(port, response);
    ssend(port, "AT+CMGS=\"" PHONE_NUMBER "\"\r");
    srecv(port, response);
    ssend(port, "Hello,I'm robot!\x1a\r");
    srecv(port, response);
    return true;
}

int main(int argc, char* argv[]) try {
    using namespace std;
    using namespace org::sqg;
    if (argc < 2)
        throw std::runtime_error("No COM port name specified!");
    test_serial_sms(argv[1]);
    // test_serial_telephone(argv[1]);
    return EXIT_SUCCESS;
} catch (std::exception const &e) {
    std::cerr << "[C++ Exception]: " << e.what() << std::endl;
    return EXIT_FAILURE;
} catch (...) {
    std::cerr << "[C++ Exception]: <UNKNOWN CAUSE>" << std::endl;
    return EXIT_FAILURE;
}
