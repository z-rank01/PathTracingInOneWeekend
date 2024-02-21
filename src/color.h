#ifndef COLOR_H
#define COLOR_H

#include "vector.h"
#include <iostream>

using color = vec3;

inline double linear_to_gamma(double linear_component)
{
    return sqrt(linear_component);
}

void pixel_to_image(std::ostream &os, color pixel_color, int sample_per_pixel)
{
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    auto scale = 1.0 / sample_per_pixel;
    r *= scale;
    g *= scale;
    b *= scale;

    // Apply the linear to gamma transform.
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    static const interval intensity(0.000, 0.999);  // make sure rgb value is not exceeding 0 to 1.
    os << static_cast<int>(256 * intensity.clamp(r)) << ' '
       << static_cast<int>(256 * intensity.clamp(g)) << ' '
       << static_cast<int>(256 * intensity.clamp(b)) << '\n';
}


#endif //COLOR_H