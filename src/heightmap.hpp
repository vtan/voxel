#pragma once

class Heightmap
{
public:
    Heightmap(size_t x_size, size_t z_size)
        : x_size_(x_size)
        , z_size_(z_size)
        , data_(x_size * z_size) {}

    size_t x_size() const { return x_size_; }
    size_t z_size() const { return z_size_; }

    uint8_t& at(size_t x, size_t z);
    uint8_t at(size_t, size_t) const;
private:
    const size_t x_size_;
    const size_t z_size_;

    std::vector<uint8_t> data_;
};

uint8_t& Heightmap::at(size_t x, size_t z)
{
    assert(x < x_size_);
    assert(z < z_size_);

    return data_[z * z_size_ + x];
}

uint8_t Heightmap::at(size_t x, size_t z) const
{
    assert(x < x_size_);
    assert(z < z_size_);

    return data_[z * z_size_ + x];
}
