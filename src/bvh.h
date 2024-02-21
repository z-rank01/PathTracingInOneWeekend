#ifndef BVH_H
#define BVH_H


#include "utility.h"
#include "object.h"
#include "scene.h"
#include <algorithm>

class bvh_node : public object
{
public:
    bvh_node(const scene& world) : bvh_node(world.objects, 0, world.objects.size()) {};

    bvh_node(const std::vector<shared_ptr<object>>& object_list, size_t start, size_t end) {
        
        // this method allow overlapping between bounding boxes
        // this is correct, not fastest though.

        auto objects = object_list;

        int axis = random_int(0, 2);
        auto comparator = (axis == 0) ? b_compare_x : 
                                        (axis == 1) ? b_compare_y : b_compare_z;

        size_t list_length = end - start;

        switch (list_length)
        {
        case 1:
            left = right = objects[start];
            break;

        case 2:
            if (comparator(objects[start], objects[start + 1]))
            {
                left = objects[start];
                right = objects[start + 1];
            }
            else
            {
                left = objects[start + 1];
                right = objects[start];
            }
            break;

        default:
            // sort bounding box according to specified axis
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
            
            // recursively construct smaller nodes of BVH
            auto mid = start + list_length / 2;
            left = std::make_shared<bvh_node>(objects, start, mid);
            right = std::make_shared<bvh_node>(objects, mid, end);
            break;
        }


        boundingBox = bbox(left->get_bbox(), right->get_bbox());
        
    }

    // Method    
    bbox get_bbox() const override { return boundingBox; }

    // double get_pdf(const point3& origin, const vec3& direction) const override
    // {
    //     return 0;
    // }

    // vec3 randomDir(const point3& origin) const override
    // {
    //     return vec3(0, 0, 0);
    // }

    bool intersect(const ray& r, interval t, intersect_record& rec) const override
    {
        if (!boundingBox.intersect(r, t))
        {
            return false;
        }

        bool intersect1 = left->intersect(r, t, rec);
        bool intersect2 = right->intersect(r, interval(t.min, intersect1 ? rec.t : t.max), rec);
        
        return intersect1 || intersect2;
    }


private:
    std::shared_ptr<object> left;
    std::shared_ptr<object> right;
    bbox boundingBox;

    static bool b_compare(const std::shared_ptr<object> a, const std::shared_ptr<object> b, int axis)
    {
        return a->get_bbox().axis(axis).min < b->get_bbox().axis(axis).min;
    }

    static bool b_compare_x(const std::shared_ptr<object> a, const std::shared_ptr<object> b)
    {
        return b_compare(a, b, 0);
    }

    static bool b_compare_y(const std::shared_ptr<object> a, const std::shared_ptr<object> b)
    {
        return b_compare(a, b, 1);
    }
    
    static bool b_compare_z(const std::shared_ptr<object> a, const std::shared_ptr<object> b)
    {
        return b_compare(a, b, 2);
    }
};


#endif //BVH_H