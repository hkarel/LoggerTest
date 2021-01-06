
#pragma once

#include "logger/logger.h"

#include <utility>
#include <tuple>
#include <iostream>
#include <boost/hana.hpp>
#include <boost/hana/ext/std/tuple.hpp>

namespace distortneo {

template<typename... Args>
class Format
{
    const char* _descript;
    std::tuple<Args...> _args;

public:
    Format(const char* descript, Args&&... args)
        : _descript(descript), _args(std::forward<Args>(args)...)
    { }

    template <typename TLine>
    void WriteTo(TLine &line);
};

template <typename TLine>
class FormatWriter
{
    const char* _descript;
    TLine &line;

public:
    FormatWriter(const char* descript, TLine &line)
        : _descript(descript), line(line) {}

    template <typename Arg>
    void operator()(const Arg& arg)
    {
        while (*_descript != '\0')
        {
            if (_descript[0] != '%' || _descript[1] != '?')
            {
                line << *_descript++;
            }
            else
            {
                _descript += 2;
                line << arg;
                break;
                // line << arg;
            }
        }
    }

    void Finish()
    {
        while (*_descript != '\0')
            line << *_descript++;
    }
};

template <typename... Args>
template <typename TLine>
void Format<Args...>::WriteTo(TLine &line)
{
    FormatWriter<TLine> formatWriter(_descript, line);
    boost::hana::for_each(_args, formatWriter);
    formatWriter.Finish();
}

template <typename... Args>
Format<Args...> format(const char *fmt, Args... args)
{
    return Format<Args...>(fmt, std::forward<Args>(args)...);
}

} // namespace distortneo

namespace alog {

template<typename... Args>
Line& operator<< (Line& line, const distortneo::Format<Args...>& fmt)
{
    if (line.toLogger())
    {
        const_cast<distortneo::Format<Args...>&>(fmt).WriteTo(line);
    }
    return line;
}

} // namespace alog

//int main(int argc, char **argv)
//{
//    format("Hello, %?-%?%?! You are awesome!\n", "Meow", 4, 5).WriteTo(std::cout);
//    format("Too few arguments %?, %?, %?.\n", 1).WriteTo(std::cout);
//    format("Too much arguments %?\n", 1, 2, 3).WriteTo(std::cout);
//    return 0;
//}
