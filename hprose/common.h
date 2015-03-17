/**********************************************************\
|                                                          |
|                          hprose                          |
|                                                          |
| Official WebSite: http://www.hprose.com/                 |
|                   http://www.hprose.org/                 |
|                                                          |
\**********************************************************/

/**********************************************************\
 *                                                        *
 * hprose/common.h                                        *
 *                                                        *
 * hprose common library for php-cpp.                     *
 *                                                        *
 * LastModified: Jul 3, 2014                              *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_COMMON_H_
#define HPROSE_COMMON_H_

#include <phpcpp.h>

namespace Hprose {

    std::string &trim(std::string &s) {
        if (s.empty()) return s;
        s.erase(0, s.find_first_not_of(" "));
        s.erase(s.find_last_not_of(" ") + 1);
        return s;
    }

    class Bytes : public Php::Base {
    public:
        std::string value;
        Bytes() {}
        Bytes(const std::string str) : value(str) {}
        virtual ~Bytes() {}
        void __construct(Php::Parameters &params) {
            value = params[0].stringValue();
        }
        std::string __toString() const {
            return value;
        }
    };

    Php::Value bytes(Php::Parameters &params) {
        return Php::Object("HproseBytes", params[0]);
    }

    class Map : public Php::Base {
    public:
        Php::Value value;
        Map() {}
        Map(Php::Value &map) : value(map) {}
        virtual ~Map() {}
        void __construct(Php::Parameters &params) {
            value = params[0];
        }
        std::string __toString() const {
            return "Map";
        }
    };

    Php::Value map(Php::Parameters &params) {
        return Php::Object("HproseMap", params[0]);
    }

    Php::Value is_list(Php::Parameters &params) {
        return params[0].isList();
    }

    Php::Value contains(Php::Parameters &params) {
        return params[0].contains(params[1]);
    }

    inline bool is_utf8(const unsigned char *str, int32_t len) {
        for (int32_t i = 0; i < len; ++i) {
            const unsigned char &c = str[i];
            switch (c >> 4) {
                case 0:
                case 1:
                case 2:
                case 3:
                case 4:
                case 5:
                case 6:
                case 7:
                    break;
                case 12:
                case 13:
                    if ((str[++i] >> 6) != 0x2) return false;
                    break;
                case 14:
                    if ((str[++i] >> 6) != 0x2) return false;
                    if ((str[++i] >> 6) != 0x2) return false;
                    break;
                case 15: {
                    const unsigned char &b = str[++i];
                    if ((str[++i] >> 6) != 0x2) return false;
                    if ((str[++i] >> 6) != 0x2) return false;
                    if ((((c & 0xf) << 2) | ((b >> 4) & 0x3)) > 0x10) return false;
                    break;
                }
                default:
                    return false;
            }
        }
        return true;
    }

    inline bool is_utf8(const std::string &str) {
        return is_utf8((const unsigned char *)str.c_str(), (int32_t)str.size());
    }

    inline bool is_utf8(const Php::Value &value) {
        return is_utf8((const unsigned char *)value.rawValue(), value.size());
    }

    Php::Value is_utf8(Php::Parameters &params) {
        return is_utf8(params[0]);
    }

    inline int32_t ustrlen(const unsigned char *str, const int32_t length) {
        int32_t len = length, pos = 0;
        while (pos < length) {
            const unsigned char &a = str[pos++];
            if (a < 0x80) {
                continue;
            }
            else if ((a & 0xE0) == 0xC0) {
                ++pos;
                --len;
            }
            else if ((a & 0xF0) == 0xE0) {
                pos += 2;
                len -= 2;
            }
            else if ((a & 0xF8) == 0xF0) {
                pos += 3;
                len -= 2;
            }
            else {
                return -1;
            }
        }
        return len;
    }

    inline int32_t ustrlen(const std::string &str) {
        return ustrlen((const unsigned char *)str.c_str(), (int32_t)str.size());
    }

    inline int32_t ustrlen(const Php::Value &value) {
        return ustrlen((const unsigned char *)value.rawValue(), value.size());
    }

    Php::Value ustrlen(Php::Parameters &params) {
        return ustrlen(params[0]);
    }

    Php::Value array_ref_search(Php::Parameters &params) {
        Php::Value &value = params[0];
        Php::Value &array = params[1];
        if (value.isArray()) {
            for (int32_t i = 0, n = array.size(); i < n; ++i) {
                Php::Value v = array.get(i);
                if (value.refequals(v)) {
                    return i;
                }
            }
        }
        else {
            for (int32_t i = 0, n = array.size(); i < n; ++i) {
                Php::Value v = array.get(i);
                if (value.type() == v.type() && value == v) {
                    return i;
                }
            }
        }
        return false;
    }

    inline void publish_common(Php::Extension &ext) {
        Php::Class<Bytes> b("HproseBytes");
        b.method("__construct",
                 &Hprose::Bytes::__construct,
                 {
                     Php::ByVal("bytes", Php::Type::String)
                 });
        ext.add(std::move(b));
        ext.add("bytes",
                &bytes,
                { Php::ByVal("bytes", Php::Type::String) });
        Php::Class<Map> m("HproseMap");
        m.method("__construct",
                 &Hprose::Map::__construct,
                 {
                     Php::ByVal("map", Php::Type::Array)
                 });
        ext.add(std::move(m));
        ext.add("map",
                &map,
                { Php::ByVal("map", Php::Type::Array) });
        ext.add("is_list",
                &is_list,
                {
                    Php::ByVal("a", Php::Type::Array)
                });
        ext.add("contains",
                &contains,
                {
                    Php::ByVal("a", Php::Type::Array),
                    Php::ByVal("index", Php::Type::Null)
                });
        ext.add("is_utf8",
                &is_utf8,
                {
                    Php::ByVal("s", Php::Type::String)
                });
        ext.add("ustrlen",
                &ustrlen,
                {
                    Php::ByVal("s", Php::Type::String)
                });
        ext.add("array_ref_search",
                &array_ref_search,
                {
                    Php::ByRef("value", Php::Type::Null),
                    Php::ByVal("array", Php::Type::Array)
                });
    }
}
#endif /* HPROSE_COMMON_H_ */
