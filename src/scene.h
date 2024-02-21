#ifndef SCENE_H
#define SCENE_H

#include "utility.h"
#include "object.h"
#include "bbox.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class scene : public object {
public:
    std::vector<shared_ptr<object>> objects;

    scene() {}
    scene(shared_ptr<object> object) { add(object); }
    

    // Method

    void clear() { objects.clear(); }

    bbox get_bbox() const override { return boundingBox; }

    void add(shared_ptr<object> object) {
        objects.push_back(object);
        boundingBox = bbox(boundingBox, object->get_bbox());   // get bounding box including all objects 
    }

    bool intersect(const ray& r, interval ray_t, intersect_record& rec) const override {
        intersect_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->intersect(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    double get_pdf(const point3& o, const vec3& v) const override {
        auto weight = 1.0/objects.size();
        auto sum = 0.0;

        for (const auto& object : objects)
            sum += weight * object->get_pdf(o, v);

        return sum;
    }

    vec3 randomDir(const vec3& o) const override 
    {
        auto int_size = static_cast<int>(objects.size());
        return objects[random_int(0, int_size-1)]->randomDir(o);
    }


private:
    bbox boundingBox;
};

#endif