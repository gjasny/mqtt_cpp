// Copyright Takatoshi Kondo 2020
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if !defined(MQTT_LOG_HPP)
#define MQTT_LOG_HPP

#include <tuple>

#include <boost/log/core.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/attributes/scoped_attribute.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/expressions/keyword.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/utility/manipulators/add_value.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>


namespace MQTT_NS {

namespace log = boost::log;

enum class severity_level {
    trace,
    debug,
    info,
    warning,
    error,
    fatal
};

inline std::ostream& operator<<(std::ostream& o, severity_level sev) {
    constexpr char const* const str[] {
        "trace",
        "debug",
        "info",
        "warning",
        "error",
        "fatal"
    };
    o << str[static_cast<std::size_t>(sev)];
    return o;
}

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(logger, log::sources::severity_channel_logger_mt<severity_level>);

// Scoped attributes
BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "MqttSeverity", severity_level)
BOOST_LOG_ATTRIBUTE_KEYWORD(channel, "MqttChannel", std::string)

// Normal attributes
BOOST_LOG_ATTRIBUTE_KEYWORD(file, "MqttFile", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(line, "MqttLine", unsigned int)
BOOST_LOG_ATTRIBUTE_KEYWORD(function, "MqttFunction", std::string)
BOOST_LOG_ATTRIBUTE_KEYWORD(address, "MqttAddress", void const*)


template <typename Logger, typename Name, typename Attr, typename... Ts>
struct scoped_attrs_cond : scoped_attrs_cond<Logger, Ts...> {
    scoped_attrs_cond(Logger& l, Name&& n, Attr&& a, Ts&&... ts)
        : scoped_attrs_cond<Logger, Ts...> { l, std::forward<Ts>(ts)... }
        , sla { l, std::forward<Name>(n), log::attributes::make_constant(std::forward<Attr>(a)) }
    {
    }

    log::aux::scoped_logger_attribute<Logger> sla;
};

template <typename Logger, typename Name, typename Attr>
struct scoped_attrs_cond<Logger, Name, Attr> {
    scoped_attrs_cond(Logger& l, Name&& n, Attr&& a)
        : sla { l, std::forward<Name>(n), log::attributes::make_constant(std::forward<Attr>(a)) }
    {
    }
    operator bool() const {
        return false;
    }

    log::aux::scoped_logger_attribute<Logger> sla;
};

template <typename Logger, typename... Ts>
auto make_scoped_attrs_cond(Logger& l, Ts&&... ts) {
    return scoped_attrs_cond<Logger, Ts...> { l, std::forward<Ts>(ts)... };
}

// Take any filterable parameters (FP)
#define MQTT_LOG_FP(...) \
    if (auto mqtt_scoped_attrs = make_scoped_attrs_cond(logger::get(), __VA_ARGS__) \
    ) { \
        (void)mqtt_scoped_attrs; \
    } \
    else \
        BOOST_LOG(logger::get()) \
            << log::add_value(file, __FILE__) \
            << log::add_value(line, __LINE__) \
            << log::add_value(function, BOOST_CURRENT_FUNCTION)

#define MQTT_LOG(chan, sev) MQTT_LOG_FP("MqttChannel", chan, "MqttSeverity", severity_level::sev)

} // namespace MQTT_NS

#endif // MQTT_LOG_HPP
