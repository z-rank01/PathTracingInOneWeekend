#ifndef TEXTURE_H
#define TEXTURE_H

#include "utility.h"
#include "rtw_stb_image.h"
#include "perlin.h"

class texture
{
public:
    virtual ~texture() = default;
    virtual color get_value(double u, double v, const point3& p) const = 0 {};
};


class solid_color : public texture
{
public:
    solid_color(color c) : color_value(c) {} 
    solid_color(double red, double green, double blue) : color_value(color(red, green, blue)) {}

    // Method
    color get_value(double u, double v, const point3& p) const override
    {
        return color_value;
    }

private:
    color color_value;
};


class checker_board : public texture
{
public:
    checker_board(double _scale, std::shared_ptr<texture> _even, std::shared_ptr<texture> _odd) : inv_scale(1.0 / _scale), even(_even), odd(_odd) {}
    checker_board(double _scale, color c1, color c2) : inv_scale(1.0f / _scale), 
                                                       even(std::make_shared<solid_color>(c1)), 
                                                       odd(std::make_shared<solid_color>(c2)) {}

    // Method
    color get_value(double u, double v, const point3& p) const override
    {
        auto x = static_cast<int>(std::floor(p.x() * inv_scale));
        auto y = static_cast<int>(std::floor(p.y() * inv_scale));
        auto z = static_cast<int>(std::floor(p.z() * inv_scale));

        // if the sum of a point is even, return one color
        // if the sum of a point is odd,  return another color
        bool isEven = (x + y + z) % 2 == 0;
        
        return isEven ? even->get_value(u, v, p) : odd->get_value(u, v, p);
    }

private:
    std::shared_ptr<texture> even;
    std::shared_ptr<texture> odd;
    double inv_scale;
};


class image_texture : public texture {
  public:
    image_texture(const char* filename) : image(filename) {}

    color get_value(double u, double v, const point3& p) const override {
        // If we have no texture data, then return solid cyan as a debugging aid.
        if (image.height() <= 0) return color(0,1,1);

        // Clamp input texture coordinates to [0,1] x [1,0]
        u = interval(0,1).clamp(u);
        v = 1.0 - interval(0,1).clamp(v);  // Flip V to image coordinates

        auto i = static_cast<int>(u * image.width());
        auto j = static_cast<int>(v * image.height());
        auto pixel = image.pixel_data(i,j);

        auto color_scale = 1.0 / 255.0;
        return color(color_scale*pixel[0], color_scale*pixel[1], color_scale*pixel[2]);
    }

  private:
    rtw_image image;
};


class noise_texture : public texture {
  public:
    noise_texture() {}
    noise_texture(double sc) : scale(sc) {}


    color get_value(double u, double v, const point3& p) const override {
        auto s = scale * p;
        return color(1,1,1) * 0.5 * (1 + sin(s.z() + 10*noise.turb(s)));
    }

  private:
    perlin noise;
    double scale;
};


#endif //TEXTURE_H