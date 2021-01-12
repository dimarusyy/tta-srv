#pragma once
#include <type_traits>

namespace traits
{
    template <typename Tp, typename... List>
    struct contains : std::true_type
    {
    };

    template <typename Tp, typename Head, typename... Rest>
    struct contains<Tp, Head, Rest...>
        : std::conditional<std::is_same<Tp, Head>::value,
        std::true_type,
        contains<Tp, Rest...>>::type
    {
    };

    template <typename Tp>
    struct contains<Tp> : std::false_type
    {
    };
} // namespace traits
