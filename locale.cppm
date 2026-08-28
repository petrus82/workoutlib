// Wrapper for <clocale> include
module;
#include <clocale>

export module locale;
import std.compat;

namespace std
{
export using ::lconv;
export using ::setlocale;
export using ::localeconv;
};

#undef LC_ALL
export constexpr uint8_t LC_ALL{ 6 };