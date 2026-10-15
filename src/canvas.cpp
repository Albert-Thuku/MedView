
// Canvas storage, pixel access and BMP export.

#include "graphics.h"
#include <fstream>
#include <stdexcept>

namespace mv {

Canvas::Canvas(int w, int h, Color bg) : w_(w), h_(h) {
    if (w <= 0 || h <= 0)
        throw std::invalid_argument("Invalid canvas dimensions");

    pixels_.assign(static_cast<std::size_t>(w) * h, bg);
}

void Canvas::pixel(int x, int y, Color c) {
    if (x >= 0 && x < w_ && y >= 0 && y < h_)
        pixels_[static_cast<std::size_t>(y) * w_ + x] = c;
}

Color Canvas::get(int x, int y) const {
    if (x < 0 || y < 0 || x >= w_ || y >= h_)
        throw std::out_of_range("Pixel outside canvas");

    return pixels_[static_cast<std::size_t>(y) * w_ + x];
}

void Canvas::saveBMP(const std::string& path) const {
    std::ofstream out(path.c_str(), std::ios::binary);

    if (!out)
        throw std::runtime_error("Cannot write " + path);

    const std::uint32_t stride =
        (static_cast<std::uint32_t>(w_) * 3 + 3) & ~3u;

    const std::uint32_t size =
        stride * static_cast<std::uint32_t>(h_);

    const auto u16 = [&out](std::uint16_t v) {
        out.put(char(v & 255));
        out.put(char((v >> 8) & 255));
    };

    const auto u32 = [&out](std::uint32_t v) {
        for (int i = 0; i < 4; ++i)
            out.put(char((v >> (8 * i)) & 255));
    };

    // BMP file header.
    out.put('B');
    out.put('M');
    u32(54 + size);
    u16(0);
    u16(0);
    u32(54);

    // BITMAPINFOHEADER.
    u32(40);
    u32(w_);
    u32(h_);
    u16(1);
    u16(24);
    u32(0);
    u32(size);
    u32(2835);
    u32(2835);
    u32(0);
    u32(0);

    // BMP stores rows bottom-up, with BGR colour ordering.
    for (int y = h_ - 1; y >= 0; --y) {
        for (int x = 0; x < w_; ++x) {
            Color c = get(x, y);
            out.put(char(c.b));
            out.put(char(c.g));
            out.put(char(c.r));
        }

        for (
            std::uint32_t p = static_cast<std::uint32_t>(w_) * 3;
            p < stride;
            ++p
        ) {
            out.put(0);
        }
    }

    if (!out)
        throw std::runtime_error("Failed writing " + path);
}

}