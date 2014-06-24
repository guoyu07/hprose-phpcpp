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
 * hprose/date.h                                          *
 *                                                        *
 * hprose date class for php-cpp.                         *
 *                                                        *
 * LastModified: Jun 24, 2014                             *
 * Author: Ma Bingyao <andot@hprose.com>                  *
 *                                                        *
\**********************************************************/

#ifndef HPROSE_DATE_H_
#define HPROSE_DATE_H_

#include <phpcpp.h>
#include <time.h>

namespace Hprose {

    class Date: public Php::Base {
    private:
        static const int32_t days_to_month_365[13];
        static const int32_t days_to_month_366[13];
        struct tm timebuf;
        inline void init(const time_t timer) {
            localtime_r(&timer, &timebuf);
            utc = false;
        }
        inline void init(int32_t year, int32_t month, int32_t day, bool utc) {
            if (is_valid_date(year, month, day)) {
                set_year(year);
                set_month(month);
                set_day(day);
                this->utc = utc;
            }
            else {
                throw Php::Exception("Unexpected arguments");
            }
        }
        inline static bool add_days_to_year(int32_t &days,
                                            int32_t &year,
                                            const int32_t period,
                                            const int32_t times) {
            if ((days >= period) || (days <= -period)) {
                int32_t remainder = days % period;
                year += times * ((days - remainder) / period);
                if (year < 1 || year > 9999) return false;
                days = remainder;
            }
            return true;
        }
    public:
        bool utc;
        inline static bool is_leap_year(int32_t year) {
            return (year % 4) == 0 ? ((year % 100) == 0 ? (year % 400) == 0
                                                        : true) : false;
        }
        inline static int32_t days_in_year(int32_t year) {
            return is_leap_year(year) ? 366 : 365;
        }
        inline static int32_t days_in_month(int32_t year, int32_t month) {
            if ((month < 1) || (month > 12)) {
                return 0;
            }
            const int *days = is_leap_year(year) ? days_to_month_366
                                                 : days_to_month_365;
            return days[month] - days[month - 1];
        }
        inline static bool is_valid_date(int32_t year,
                                         int32_t month,
                                         int32_t day) {
            if ((year > 0) && (year < 10000)) {
                int days = days_in_month(year, month);
                return (days > 0) && (day > 0) && (day <= days);
            }
            return false;
        }
        Date() {}
        Date(const time_t *timer) {
            init(*timer);
        }
        Date(int32_t year, int32_t month, int32_t day) :
            Date(year, month, day, false) {
        }
        Date(int32_t year, int32_t month, int32_t day, bool utc) {
            init(year, month, day, utc);
        }
        virtual ~Date() {
        }
        inline int32_t year() const {
            return timebuf.tm_year + 1900;
        }
        inline int32_t month() const {
            return timebuf.tm_mon + 1;
        }
        inline int32_t day() const {
            return timebuf.tm_mday;
        }
        inline void set_year(const int32_t year) {
            timebuf.tm_year = year - 1900;
        }
        inline void set_month(const int32_t month) {
            timebuf.tm_mon = month - 1;
        }
        inline void set_day(const int32_t day) {
            timebuf.tm_mday = day;
        }
        inline bool add_days(int32_t days) {
            if (days == 0) return true;
            int32_t y = year();
            if (!add_days_to_year(days, y, 146097, 400)) return false;
            if (!add_days_to_year(days, y, 36524, 100)) return false;
            if (!add_days_to_year(days, y, 1461, 4)) return false;
            int32_t m = month();
            while (days >= 365) {
                if (y >= 9999) return false;
                days -= days_in_year(m <= 2 ? y++ : ++y);
            }
            while (days < 0) {
                if (y <= 1) return false;
                days += days_in_year(m <= 2 ? --y : y--);
            }
            int32_t d_in_m = days_in_month(y, m);
            int32_t d = day();
            while (d + days > d_in_m) {
                days -= d_in_m - d + 1;
                if (++m > 12) {
                    if (++y > 9999) return false;
                    m = 1;
                }
                d = 1;
                d_in_m = days_in_month(y, m);
            }
            d += days;
            set_year(y);
            set_month(m);
            set_day(d);
            return true;
        }
        inline bool add_months(int32_t months) {
            if (months == 0) return true;
            int32_t m = month() + months - 1;
            int32_t years = 0;
            if (m >= 0) {
                years = m / 12;
                m = m % 12 + 1;
            }
            else {
                years = m / 12 - 1;
                m = m % 12 + 13;
            }
            if (add_years(years)) {
                int32_t d = day() - days_in_month(year(), m);
                if (d > 0) {
                    ++m;
                    set_day(d);
                }
                set_month(m);
                return true;
            }
            else {
                return false;
            }
        }
        inline bool add_years(int32_t years) {
            if (years == 0) return true;
            int32_t y = year() + years;
            if (y < 1 || y > 9999) return false;
            set_year(y);
            return true;
        }
        virtual inline int format(char *str, bool fullformat = true) const {
            const char *format = (fullformat ? "%04d-%02d-%02d"
                                             : "%04d%02d%02d");
            int n = sprintf(str, format, year(), month(), day());
            if (utc) {
                str[n++] = 'Z';
                str[n] = 0;
            }
            return n;
        }
        inline std::string to_string(bool fullformat = true) const {
            char buffer[32];
            int n = format(buffer, fullformat);
            return std::string(buffer, n);
        }
        virtual inline time_t time() {
            timebuf.tm_hour = 0;
            timebuf.tm_min = 0;
            timebuf.tm_sec = 0;
            return utc ? timegm(&timebuf) : mktime(&timebuf);
        }
        inline int32_t day_of_week() const {
            return day_of_week(year(), month(), day());
        }
        static inline int32_t day_of_week(int32_t year, int32_t month, int32_t day) {
            day += month < 3 ? year-- : year - 2;
            return ((23 * month / 9) + day + 4 + (year / 4) - (year / 100) + (year / 400)) % 7;
        }
        inline int32_t day_of_year() const {
            return day_of_year(year(), month(), day());
        }
        static inline int32_t day_of_year(int32_t year, int32_t month, int32_t day) {
            const int *days = is_leap_year(year) ? days_to_month_366
                                                 : days_to_month_365;
            return days[month - 1] + day;
        }
        // -----------------------------------------------------------
        // for PHP
        void __construct(Php::Parameters &params) {
            size_t n = params.size();
            switch (n) {
                case 0: {
                    init(::time(0));
                    break;
                }
                case 1: {
                    Php::Value val = params[0];
                    if (val.isNumeric()) {
                        init(val.numericValue());
                    }
                    else if (val.isString()) {
                        init(Php::call("strtotime", val).numericValue());
                        utc = false;
                    }
                    else if (val.isArray()) {
                        init(val.get("year", 4),
                             val.get("mon", 3),
                             val.get("mday", 4),
                             false);
                    }
                    else if (val.isObject()) {
                        if (Php::call("is_a", val, "Hprose\\Date")) {
                            timebuf = ((Hprose::Date *) val.implementation())->timebuf;
                        }
                        else if (Php::call("is_a", val, "HproseDate")) {
                            init(val.get("year", 4),
                                 val.get("month", 5),
                                 val.get("day", 3),
                                 val.get("utc", 3));
                        }
                        else {
                            throw Php::Exception("Unexpected arguments");
                        }
                    }
                    else {
                        throw Php::Exception("Unexpected arguments");
                    }
                    break;
                }
                case 3: {
                    init(params[0], params[1], params[2], false);
                    break;
                }
                case 4: {
                    init(params[0], params[1], params[2], params[3]);
                    break;
                }
            }
        }
        Php::Value getYear() const {
            return year();
        }
        void setYear(const Php::Value &year) {
            set_year(year);
        }
        Php::Value getMonth() const {
            return month();
        }
        void setMonth(const Php::Value &month) {
            set_month(month);
        }
        Php::Value getDay() const {
            return day();
        }
        void setDay(const Php::Value &day) {
            set_day(day);
        }
        Php::Value getUtc() const {
            return utc;
        }
        void setUtc(const Php::Value &utc) {
            this->utc = utc;
        }
        Php::Value addDays(Php::Parameters &params) {
            if (params[0].isNumeric()) {
                return add_days(params[0]);
            }
            return false;
        }
        Php::Value addMonths(Php::Parameters &params) {
            if (params[0].isNumeric()) {
                return add_months(params[0]);
            }
            return false;
        }
        Php::Value addYears(Php::Parameters &params) {
            if (params[0].isNumeric()) {
                return add_years(params[0]);
            }
            return false;
        }
        Php::Value toString(Php::Parameters &params) const {
            bool fullformat = params.size() > 0 ? params[0].boolValue() : true;
            Php::Value result;
            char *buffer = result.reserve(16);
            int n = format(buffer, fullformat);
            result.reserve(n);
            return result;
        }
        Php::Value timestamp() {
            return (int64_t)time();
        }
        Php::Value __toString() const {
            Php::Value result;
            char *buffer = result.reserve(16);
            int n = format(buffer);
            result.reserve(n);
            return result;
        }
        static Php::Value isLeapYear(Php::Parameters &params) {
            if (params[0].isNumeric()) {
                return is_leap_year(params[0]);
            }
            return false;
        }
        static Php::Value daysInMonth(Php::Parameters &params) {
            return days_in_month(params[0], params[1]);
        }
        static Php::Value daysInYear(Php::Parameters &params) {
            return days_in_year(params[0]);
        }
        static Php::Value isValidDate(Php::Parameters &params) {
            return is_valid_date(params[0], params[1], params[2]);
        }
        Php::Value dayOfWeek(Php::Parameters &params) const {
            if (params.size() == 3) {
                return day_of_week(params[0], params[1], params[2]);
            }
            else {
                return day_of_week();
            }
        }
        Php::Value dayOfYear(Php::Parameters &params) const {
            if (params.size() == 3) {
                return day_of_year(params[0], params[1], params[2]);
            }
            else {
                return day_of_year();
            }
        }
    };
    inline void publish_date(Php::Namespace &ns) {
        Php::Class<Hprose::Date> date("Date");
        date.method("__construct",
                    &Hprose::Date::__construct,
                     { Php::ByVal("arg1", Php::Type::Null, false),
                       Php::ByVal("arg2", Php::Type::Numeric, false),
                       Php::ByVal("arg3", Php::Type::Numeric, false),
                       Php::ByVal("arg4", Php::Type::Bool, false) });
        date.property("year", &Hprose::Date::getYear, &Hprose::Date::setYear);
        date.property("month", &Hprose::Date::getMonth, &Hprose::Date::setMonth);
        date.property("day", &Hprose::Date::getDay, &Hprose::Date::setDay);
        date.property("utc", &Hprose::Date::getUtc, &Hprose::Date::setUtc);
        date.method("addDays",
                    &Hprose::Date::addDays,
                     { Php::ByVal("days", Php::Type::Numeric) });
        date.method("addMonths",
                    &Hprose::Date::addMonths,
                     { Php::ByVal("months", Php::Type::Numeric) });
        date.method("addYears",
                    &Hprose::Date::addYears,
                     { Php::ByVal("years", Php::Type::Numeric) });
        date.method("timestamp", &Hprose::Date::timestamp);
        date.method("toString",
                    &Hprose::Date::toString,
                     { Php::ByVal("fullformat", Php::Type::Bool, false) });
        date.method("__toString", &Hprose::Date::__toString);
        date.method("isLeapYear",
                    &Hprose::Date::isLeapYear,
                    Php::Public | Php::Static,
                    { Php::ByVal("days", Php::Type::Numeric) });
        date.method("daysInMonth",
                    &Hprose::Date::daysInMonth,
                    Php::Public | Php::Static,
                    { Php::ByVal("year", Php::Type::Numeric),
                      Php::ByVal("month", Php::Type::Numeric) });
        date.method("daysInYear",
                    &Hprose::Date::daysInYear,
                    Php::Public | Php::Static,
                    { Php::ByVal("year", Php::Type::Numeric) });
        date.method("isValidDate",
                    &Hprose::Date::isLeapYear,
                    Php::Public | Php::Static,
                    { Php::ByVal("year", Php::Type::Numeric),
                      Php::ByVal("month", Php::Type::Numeric),
                      Php::ByVal("day", Php::Type::Numeric) });
        date.method("dayOfWeek",
                    &Hprose::Date::dayOfWeek,
                    { Php::ByVal("year", Php::Type::Numeric, false),
                      Php::ByVal("month", Php::Type::Numeric, false),
                      Php::ByVal("day", Php::Type::Numeric, false) });
        date.method("dayOfYear",
                    &Hprose::Date::dayOfYear,
                    { Php::ByVal("year", Php::Type::Numeric, false),
                      Php::ByVal("month", Php::Type::Numeric, false),
                      Php::ByVal("day", Php::Type::Numeric, false) });
        ns.add(std::move(date));
    }
}

#endif /* HPROSE_DATE_H_ */
