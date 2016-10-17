#pragma once

#define GLM_FORCE_RADIANS

#include <vector>

#include <glm/glm.hpp>

template <typename T>
class Volume
{
public:
    Volume(size_t x, size_t y, size_t z, T init)
        : data(x * y * z, init)
        , s_x(x)
        , s_y(y)
        , s_z(z)
        , s_xy(x * y)
    {}

    size_t size_x() const { return s_x; }
    size_t size_y() const { return s_y; }
    size_t size_z() const { return s_z; }

    T& at(size_t x, size_t y, size_t z);
    T& at(glm::ivec3 v) { return at(v.x, v.y, v.z); }
    const T& at(size_t x, size_t y, size_t z) const;
    const T& at(glm::ivec3 v) const { return at(v.x, v.y, v.z); }

    template <typename F>
    void for_each_in_border(size_t x, size_t y, size_t z, F functor) const;
private:
    std::vector<T> data;
    size_t s_x;
    size_t s_y;
    size_t s_z;
    size_t s_xy;
};

template <typename T>
T& Volume<T>::at(const size_t x, const size_t y, const size_t z)
{
    assert(x >= 0);
    assert(x < s_x);
    assert(y >= 0);
    assert(y < s_y);
    assert(z >= 0);
    assert(z < s_z);

    return data[z * s_xy + y * s_x + x];
}

template <typename T>
const T& Volume<T>::at(const size_t x, const size_t y, const size_t z) const
{
    assert(x >= 0);
    assert(x < s_x);
    assert(y >= 0);
    assert(y < s_y);
    assert(z >= 0);
    assert(z < s_z);

    return data[z * s_xy + y * s_x + x];
}

template <typename T>
template <typename F>
void Volume<T>::for_each_in_border(
        const size_t border_x,
        const size_t border_y,
        const size_t border_z,
        F f)
    const
{
    for (size_t z = border_z; z < s_z - border_z; ++z) {
        for (size_t y = border_y; y < s_y - border_y; ++y) {
            for (size_t x = border_x; x < s_x - border_x; ++x) {
                f(x, y, z);
            }
        }
    }
}
