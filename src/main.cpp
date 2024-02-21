#include <iostream>

#include "utility.h"
#include "vector.h"
#include "color.h"
#include "camera.h"
#include "sphere.h"
#include "scene.h"
#include "material.h"
#include "bvh.h"
#include "bbox.h"
#include "texture.h"
#include "quad.h"
#include "constant_medium.h"
#include "pdf.h"


void materials();
void random_spheres();
void two_spheres();
void earth();
void two_perlin_spheres();
void quads();
void simple_light();
void cornell_box();
void cornell_smoke();
void rayTracingtheNextWeek_final_scene(int image_width, int samples_per_pixel, int max_depth);

int main()
{
    int n = 7;
    switch (n)
    {
    case 0:
        materials();
        break;
    case 1:
        random_spheres();
        break;
    case 2:
        two_spheres();
        break;
    case 3:
        earth();
        break;
    case 4:
        two_perlin_spheres();
        break;
    case 5:
        quads();
        break;
    case 6:
        simple_light();
        break;
    case 7:
        cornell_box();
        break;
    case 8:
        cornell_smoke();
        break;
    case 9:
        rayTracingtheNextWeek_final_scene(800, 200, 40);
        break;
    default:
        rayTracingtheNextWeek_final_scene(400, 100,  4);
        break;
    }

    return 0;
}

void materials()
{
    camera cam;
    scene world;
    scene lights;
    
    // Objects

    auto material_ground = make_shared<lambertian>(color(0.8, 0.8, 0.0));
    auto material_center = make_shared<lambertian>(color(0.1, 0.2, 0.5));
    auto material_left   = make_shared<dielectric>(1.5);
    auto material_right  = make_shared<metal>(color(0.8, 0.6, 0.2), 0.3);

    world.add(make_shared<sphere>(point3( 0.0, -100.5, -1.0),  100.0, material_ground));
    world.add(make_shared<sphere>(point3( 0.0,    0.0, -1.0),    0.5, material_center));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),    0.5, material_left));
    world.add(make_shared<sphere>(point3(-1.0,    0.0, -1.0),   -0.4, material_left));
    world.add(make_shared<sphere>(point3( 1.0,    0.0, -1.0),    0.5, material_right));


    // Camera

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.img_width         = 800;
    cam.samples_per_pixel = 100;
    cam.sample_max_depth  = 50;

    cam.vfov              = 20;
    cam.lookfrom          = point3(-2,2,1);
    cam.lookat            = point3(0,0,-1);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 10.0;
    cam.focus_distance    = 3.4;

    cam.background        = color(0.70, 0.80, 1.00);
    
    // Render

    cam.render(world, lights);
}


void random_spheres()
{
    camera cam;
    scene world;
    scene lights;
    
    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));

    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = random_double();
            point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

            if ((center - point3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = color::random() * color::random();
                    sphere_material = make_shared<lambertian>(albedo);
                    auto center2 = center + vec3(0, random_double(0,.5), 0);                   // motion blur: random destination
                    world.add(make_shared<sphere>(center, center2, 0.2, sphere_material));     // motion blur: add new sphere with start and end location
                    // world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = color::random(0.5, 1);
                    auto fuzz = random_double(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world.add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }

    auto material1 = make_shared<dielectric>(1.5);
    world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

    world = scene(make_shared<bvh_node>(world));  // build BVH

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.img_width         = 400;
    cam.samples_per_pixel = 40;
    cam.sample_max_depth  = 40;

    cam.vfov              = 20;
    cam.lookfrom          = point3(13,2,3);
    cam.lookat            = point3(0,0,0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0.6;
    cam.focus_distance    = 10.0;

    cam.background        = color(0.70, 0.80, 1.00);

    cam.render(world, lights);
}


void two_spheres()
{
    scene world;
    scene lights;

    auto checker = make_shared<checker_board>(0.8, color(.2, .3, .1), color(.9, .9, .9));

    world.add(make_shared<sphere>(point3(0,-10, 0), 10, make_shared<lambertian>(checker)));
    world.add(make_shared<sphere>(point3(0, 10, 0), 10, make_shared<lambertian>(checker)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.img_width         = 400;
    cam.samples_per_pixel = 100;
    cam.sample_max_depth  = 50;

    cam.vfov              = 20;
    cam.lookfrom          = point3(13,2,3);
    cam.lookat            = point3(0,0,0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0;

    cam.background        = color(0.70, 0.80, 1.00);

    cam.render(world, lights);
}

void earth() {
    scene lights;

    auto earth_texture = make_shared<image_texture>("../image/earthmap.jpg");
    auto earth_surface = make_shared<lambertian>(earth_texture);
    auto globe = make_shared<sphere>(point3(0,0,0), 2, earth_surface);

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.img_width         = 400;
    cam.samples_per_pixel = 100;
    cam.sample_max_depth  = 50;

    cam.vfov              = 20;
    cam.lookfrom          = point3(0,0,12);
    cam.lookat            = point3(0,0,0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0;

    cam.background        = color(0.70, 0.80, 1.00);

    cam.render(scene(globe), lights);
}

void two_perlin_spheres() {
    scene world;
    scene lights;

    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.img_width         = 400;
    cam.samples_per_pixel = 100;
    cam.sample_max_depth  = 50;

    cam.vfov              = 20;
    cam.lookfrom          = point3(13,2,3);
    cam.lookat            = point3(0,0,0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0;

    cam.background        = color(0.70, 0.80, 1.00);

    cam.render(world, lights);
}


void quads() {
    scene world;
    scene lights;

    // Materials
    auto left_red     = make_shared<lambertian>(color(1.0, 0.2, 0.2));
    auto back_green   = make_shared<lambertian>(color(0.2, 1.0, 0.2));
    auto right_blue   = make_shared<lambertian>(color(0.2, 0.2, 1.0));
    auto upper_orange = make_shared<lambertian>(color(1.0, 0.5, 0.0));
    auto lower_teal   = make_shared<lambertian>(color(0.2, 0.8, 0.8));

    // Quads
    world.add(make_shared<quad>(point3(-3,-2, 5), vec3(0, 0,-4), vec3(0, 4, 0), left_red));
    world.add(make_shared<quad>(point3(-2,-2, 0), vec3(4, 0, 0), vec3(0, 4, 0), back_green));
    world.add(make_shared<quad>(point3( 3,-2, 1), vec3(0, 0, 4), vec3(0, 4, 0), right_blue));
    world.add(make_shared<quad>(point3(-2, 3, 1), vec3(4, 0, 0), vec3(0, 0, 4), upper_orange));
    world.add(make_shared<quad>(point3(-2,-3, 5), vec3(4, 0, 0), vec3(0, 0,-4), lower_teal));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.img_width         = 400;
    cam.samples_per_pixel = 100;
    cam.sample_max_depth  = 50;

    cam.vfov              = 80;
    cam.lookfrom          = point3(0,0,9);
    cam.lookat            = point3(0,0,0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0;

    cam.background        = color(0.70, 0.80, 1.00);

    cam.render(world, lights);
}

void simple_light() {
    scene world;
    scene lights;

    // world objects
    auto pertext = make_shared<noise_texture>(4);
    world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(pertext)));
    world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(pertext)));

    // light sources
    auto difflight = make_shared<diffuse_light>(color(4,4,4));
    world.add(make_shared<sphere>(point3(0,7,0), 2, difflight));
    world.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));
    lights.add(make_shared<sphere>(point3(0,7,0), 2, difflight));
    lights.add(make_shared<quad>(point3(3,1,-2), vec3(2,0,0), vec3(0,2,0), difflight));

    camera cam;

    cam.aspect_ratio      = 16.0 / 9.0;
    cam.img_width         = 400;
    cam.samples_per_pixel = 100;
    cam.sample_max_depth  = 50;

    cam.vfov              = 20;
    cam.lookfrom          = point3(26,3,6);
    cam.lookat            = point3(0,2,0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0;

    cam.background        = color(0,0,0);

    cam.render(world, lights);
}

void cornell_box() {
    scene world;
    scene lights;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(30, 30, 30));
    // auto light = make_shared<diffuse_light>(8.0f * color(0.747f+0.058f, 0.747f+0.258f, 0.747f) + 15.6f * color(0.740f+0.287f,0.740f+0.160f,0.740f) + 18.4f * color(0.737f+0.642f,0.737f+0.159f,0.737f));
    

    // world objects

    // cornell box's slabs
    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), red));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(555,555,555), vec3(-555,0,0), vec3(0,0,-555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));
    world.add(make_shared<quad>(point3(213,554,227), vec3(130,0,0), vec3(0,0,105), light));

    // box inside
    auto box1 = box(point3(0,0,0), point3(165,330,165), white);
    auto box2 = box(point3(0,0,0), point3(165,165,165), white);

    // world space rotation and translation
    for (auto quad : box1->objects)
    {
        quad->rotate(15.0, 1);  // x = 0, y = 1, z = 2
        quad->translate(vec3(265,0,295));
    }
    for (auto quad : box2->objects)
    {
        quad->rotate(-18.0, 1); // x = 0, y = 1, z = 2
        quad->translate(vec3(130,0,65));
    }

    world.add(box1);
    world.add(box2);

    // light sources
    auto m = std::shared_ptr<material>();
    lights.add(std::make_shared<quad>(point3(343,554,332), vec3(-130,0,0), vec3(0,0,-105), m));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.img_width         = 400;
    cam.samples_per_pixel = 200;
    cam.sample_max_depth  = 50;
    cam.background        = color(0,0,0);

    cam.vfov              = 40;
    cam.lookfrom          = point3(278, 278, -800);
    cam.lookat            = point3(278, 278, 0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0;

    cam.render(world, lights);
}

void cornell_smoke() {
    scene world;
    scene lights;

    auto red   = make_shared<lambertian>(color(.65, .05, .05));
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    auto green = make_shared<lambertian>(color(.12, .45, .15));
    auto light = make_shared<diffuse_light>(color(7, 7, 7));

    // light soureces
    world.add(make_shared<quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light));
    lights.add(make_shared<quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light));

    world.add(make_shared<quad>(point3(555,0,0), vec3(0,555,0), vec3(0,0,555), green));
    world.add(make_shared<quad>(point3(0,0,0), vec3(0,555,0), vec3(0,0,555), red));
    // world.add(make_shared<quad>(point3(113,554,127), vec3(330,0,0), vec3(0,0,305), light));
    world.add(make_shared<quad>(point3(0,555,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,0), vec3(555,0,0), vec3(0,0,555), white));
    world.add(make_shared<quad>(point3(0,0,555), vec3(555,0,0), vec3(0,555,0), white));

    shared_ptr<scene> box1 = box(point3(0,0,0), point3(165,330,165), white);
    shared_ptr<scene> box2 = box(point3(0,0,0), point3(165,165,165), white);
    for (auto quad : box1->objects)
    {
        quad->rotate(15.0, 1);
        quad->translate(vec3(265,0,295));
    }
    for (auto quad : box2->objects)
    {
        quad->rotate(-18.0, 1);
        quad->translate(vec3(130,0,65));
    }

    world.add(make_shared<constant_medium>(box1, 0.01, color(0,0,0)));
    world.add(make_shared<constant_medium>(box2, 0.01, color(1,1,1)));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.img_width         = 400;
    cam.samples_per_pixel = 100;
    cam.sample_max_depth  = 50;
    cam.background        = color(0,0,0);

    cam.vfov              = 40;
    cam.lookfrom          = point3(278, 278, -800);
    cam.lookat            = point3(278, 278, 0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0;

    cam.render(world, lights);
}

void rayTracingtheNextWeek_final_scene(int image_width, int samples_per_pixel, int max_depth) {
    scene boxes1;
    auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

    int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; i++) {
        for (int j = 0; j < boxes_per_side; j++) {
            auto w = 100.0;
            auto x0 = -1000.0 + i*w;
            auto z0 = -1000.0 + j*w;
            auto y0 = 0.0;
            auto x1 = x0 + w;
            auto y1 = random_double(1,101);
            auto z1 = z0 + w;

            boxes1.add(box(point3(x0,y0,z0), point3(x1,y1,z1), ground));
        }
    }

    scene world;
    scene lights;

    // light sources
    auto light = make_shared<diffuse_light>(color(7, 7, 7));
    world.add(make_shared<quad>(point3(123,554,147), vec3(300,0,0), vec3(0,0,265), light));
    lights.add(make_shared<quad>(point3(123,554,147), vec3(300,0,0), vec3(0,0,265), light));


    // world objects
    world.add(make_shared<bvh_node>(boxes1));

    auto center1 = point3(400, 400, 200);
    auto center2 = center1 + vec3(30,0,0);
    auto sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
    world.add(make_shared<sphere>(center1, center2, 50, sphere_material));

    world.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
    world.add(make_shared<sphere>(
        point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
    ));

    auto boundary = make_shared<sphere>(point3(360,150,145), 70, make_shared<dielectric>(1.5));
    world.add(boundary);
    world.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
    boundary = make_shared<sphere>(point3(0,0,0), 5000, make_shared<dielectric>(1.5));
    world.add(make_shared<constant_medium>(boundary, .0001, color(1,1,1)));

    auto emat = make_shared<lambertian>(make_shared<image_texture>("../../../../image/earthmap.jpg"));
    world.add(make_shared<sphere>(point3(400,200,400), 100, emat));
    auto pertext = make_shared<noise_texture>(0.1);
    world.add(make_shared<sphere>(point3(220,280,300), 80, make_shared<lambertian>(pertext)));

    scene boxes2;
    auto white = make_shared<lambertian>(color(.73, .73, .73));
    int ns = 1000;
    for (int j = 0; j < ns; j++) {
        boxes2.add(make_shared<sphere>(point3::random(0,165), 10, white));
    }
    for (auto sphere : boxes2.objects)
    {
        sphere->rotate(15.0, 1);
        sphere->translate(vec3(-100, 270, 395));
    }
    world.add(std::make_shared<bvh_node>(boxes2));

    camera cam;

    cam.aspect_ratio      = 1.0;
    cam.img_width         = image_width;
    cam.samples_per_pixel = samples_per_pixel;
    cam.sample_max_depth  = max_depth;
    cam.background        = color(0,0,0);

    cam.vfov              = 40;
    cam.lookfrom          = point3(478, 278, -600);
    cam.lookat            = point3(278, 278, 0);
    cam.vup               = vec3(0,1,0);

    cam.defocus_angle     = 0;

    cam.render(world, lights);
}