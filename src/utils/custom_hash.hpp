#pragma once

#include <ankerl/unordered_dense.h>
#include <glm/glm.hpp>

template<>
struct ankerl::unordered_dense::hash<glm::ivec3> {
    using is_avalanching = void;

    [[nodiscard]] auto operator()(glm::ivec3 const& x) const noexcept -> uint64_t {
        size_t seed = 0;
        seed = detail::wyhash::mix(seed, detail::wyhash::hash(x.x));
        seed = detail::wyhash::mix(seed, detail::wyhash::hash(x.y));
        seed = detail::wyhash::mix(seed, detail::wyhash::hash(x.z));
        return seed;        
    }
};