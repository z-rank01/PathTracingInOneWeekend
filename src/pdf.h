#ifndef PDF_H
#define PDF_H

#include "utility.h"
#include "object.h"
#include "onb.h"
#include "scene.h"

class pdf
{
public:
    virtual ~pdf() {}
    virtual double get_value(const vec3& direction) const = 0;
    virtual vec3 generate_randomDir() const = 0;
};


class uniform_sphere_pdf : public pdf
{
public:
    uniform_sphere_pdf() {};
    
    double get_value(const vec3& direction) const override
    {
        return 1 / (4 * pi);
    }

    vec3 generate_randomDir() const override
    {
        return randomSample_unit_sphere();
    }
    
};

class cosine_pdf : public pdf
{
public:
    cosine_pdf(const vec3& normal)
    {
        uvw.build_from_w(normal);
    }
    
    double get_value(const vec3& direction) const override
    {
        auto cos_theta = dot(uvw.w(), unit_vector(direction));
        return fmax(0, cos_theta / pi);
    }

    vec3 generate_randomDir() const override
    {
        return uvw.local(randomSample_cosine_direction());
    }

private:
    onb uvw;  // coordinate
};


class object_pdf : public pdf
{
public:
    object_pdf(const object& _objects, const point3& _origin) : objects(_objects), origin(_origin) {}

    double get_value(const vec3& direction) const override
    {
        return objects.get_pdf(origin, direction);
    }

    vec3 generate_randomDir() const override
    {
        return objects.randomDir(origin);
    }


private:
    const object& objects;
    point3 origin;
};

class mixture_pdf : public pdf {
  public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }

    double get_value(const vec3& direction) const override {
        return 0.5 * p[0]->get_value(direction) + 0.5 *p[1]->get_value(direction);
    }

    vec3 generate_randomDir() const override {
        if (random_double() < 0.5)
            return p[0]->generate_randomDir();
        else
            return p[1]->generate_randomDir();
    }

  private:
    shared_ptr<pdf> p[2];
};

#endif //PDF_H