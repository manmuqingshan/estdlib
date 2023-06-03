#pragma once

#include "cstddef.h"

namespace estd {

namespace internal {

struct in_place_tag {};

}

struct in_place_t : internal::in_place_tag
{

};

template <class T>
struct in_place_type_t : internal::in_place_tag
{

};

template <size_t>
struct in_place_index_t : internal::in_place_tag
{};

}
