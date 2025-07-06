#pragma once

#include "fwd.h"

namespace estd { namespace internal {

template <class TFacet, class TLocale>
struct use_facet_helper;

}

template <class Facet, class Locale>
constexpr typename internal::use_facet_helper<Facet, Locale>::facet_type use_facet(Locale l)
{
    return internal::use_facet_helper<Facet, Locale>::use_facet(l);
}


} // namespace estd { namespace experimental {