#ifndef ONB_H
#define ONB_H

#include "utility.h"

class onb {
  public:
    onb() {}

    vec3 operator[](int i) const { return axis[i]; }
    vec3& operator[](int i) { return axis[i]; }

    vec3 u() const { return axis[0]; }
    vec3 v() const { return axis[1]; }
    vec3 w() const { return axis[2]; }

    vec3 local(double a, double b, double c) const {
        return a*u() + b*v() + c*w();
    }

    vec3 local(const vec3& a) const {
        return a.x()*u() + a.y()*v() + a.z()*w();
    }

    void build_from_w(const vec3& w) {
        
        // naive approach
        
        // vec3 unit_w = unit_vector(w);
        // vec3 a = (fabs(unit_w.x()) > 0.9) ? vec3(0,1,0) : vec3(1,0,0);
        // vec3 v = unit_vector(cross(unit_w, a));
        // vec3 u = cross(unit_w, v);
        // axis[0] = u;
        // axis[1] = v;
        // axis[2] = unit_w;

        // Moller and Hughes approach (failed)

        // vec3 unit_w = unit_vector(w);
        // auto x_abs = std::fabs(unit_w.x());
        // auto y_abs = std::fabs(unit_w.y());
        // auto z_abs = std::fabs(unit_w.z());
        // vec3 v = x_abs < y_abs && x_abs < z_abs ? vec3(0, -unit_w.z(), unit_w.y()) : 
        //          y_abs < x_abs && y_abs < z_abs ? vec3(-unit_w.z(), 0, unit_w.x()) :
        //                                           vec3(-unit_w.y(), unit_w.x(), 0);
        // v = unit_vector(v);
        // vec3 u = cross(v, unit_w);
        // axis[0] = u;
        // axis[1] = v;
        // axis[2] = unit_w;

        // pixar approach (revised from Frisvad)
        
        vec3 unit_w = unit_vector(w);
        float sign = copysignf(1.0f, unit_w.z());
        const float a = -1.0f / (sign + unit_w.z());
        const float b = unit_w.x() * unit_w.y() * a;
        auto u = vec3(1.0f + sign * unit_w.x() * unit_w.x() * a, sign * b, -sign * unit_w.x());
        auto v = vec3(b, sign + unit_w.y() * w.y() * a, -unit_w.y());
        // u = unit_vector(u);
        // v = unit_vector(v);
        axis[0] = u;
        axis[1] = v;
        axis[2] = unit_w;
    }

  public:
    vec3 axis[3];
};


#endif