#include "raylib.h"
#include "vec3.h"

#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>

const auto canvas_width{600};
const auto canvas_height{600};
const auto viewport_width{1.0};
const auto viewport_height{1.0};
const auto distance_from_camera{1.0};
const vec3 camera_position{0, 0, 0};

class Sphere {
  public:
    vec3 center;
    double radius;
    Color color;

    Sphere() {}

    Sphere(vec3 cen, double rad, Color col) {
        center = cen;
        radius = rad;
        color = col;
    }
};

class Scene {
  public:
    Color background_color;
    std::vector<Sphere> entities;

    Scene() {}
};

vec3 canvas_to_viewport(double x, double y) {
    return {x * viewport_width / canvas_width,
            y * viewport_height / canvas_height, distance_from_camera};
}

std::array<double, 2> IntersectRaySphere(vec3 origin, vec3 direction,
                                         Sphere sphere) {
    auto r{sphere.radius};
    auto co{origin - sphere.center};

    auto a{dot(direction, direction)};

    auto b{2.0 * dot(co, direction)};
    auto c{dot(co, co) - r * r};

    auto discriminant{b * b - 4.0 * a * c};
    if (discriminant < 0.0) {
        std::array<double, 2> infinity{std::numeric_limits<double>::infinity(),
                                       std::numeric_limits<double>::infinity()};
        return infinity;
    }

    auto t1{(-b + sqrt(discriminant)) / (2.0 * a)};
    auto t2{(-b - sqrt(discriminant)) / (2.0 * a)};
    std::array<double, 2> t{t1, t2};
    return t;
}

Color trace_ray(vec3 origin, vec3 direction, double t_min, double t_max,
                Scene scene) {
    auto closest_t{std::numeric_limits<double>::infinity()};
    std::optional<Sphere> closest_shpere{};

    for (auto entity : scene.entities) {
        auto t{(IntersectRaySphere(origin, direction, entity))};
        if (t_min < t[0] && t[0] < t_max && t[0] < closest_t) {
            closest_t = t[0];
            closest_shpere = entity;
        }
        if (t_min < t[1] && t[1] < t_max && t[1] < closest_t) {
            closest_t = t[1];
            closest_shpere = entity;
        }
    }

    if (closest_shpere == std::nullopt) {
        return scene.background_color;
    } else {
        return closest_shpere->color;
    }
}

Scene create_scene(Color background, std::vector<Sphere> entites) {
    Scene scene{};
    scene.background_color = background;
    scene.entities = entites;
    return scene;
}

void put_pixel(int x, int y, Color color) {
    auto screen_width{canvas_width / 2 + x};
    auto screen_height{canvas_height / 2 - y - 1};

    DrawPixel(screen_width, screen_height, color);
}
int main() {
    const auto red{Color{255, 0, 0, 255}};
    const auto blue{Color{0, 255, 0, 255}};
    const auto green{Color{0, 255, 0, 255}};
    const std::vector<Sphere> spheres{
        Sphere(vec3(0, -1, 3), 1, Color{255, 0, 0, 255}),
        Sphere(vec3(2, 0, 4), 1, Color{0, 0, 255, 255}),
        Sphere(vec3(-2, 0, 4), 1, Color{0, 255, 0, 255})};

    Scene scene{create_scene(RAYWHITE, spheres)};

    InitWindow(canvas_width, canvas_height, "Ray tracer");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (auto x{-canvas_width / 2}; x <= canvas_width / 2; x++) {
            for (auto y{-canvas_height / 2}; y <= canvas_height / 2; y++) {
                auto d{canvas_to_viewport(x, y)};
                auto color{trace_ray(camera_position, d, 1,
                                     std::numeric_limits<double>::infinity(),
                                     scene)};
                put_pixel(x, y, color);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
