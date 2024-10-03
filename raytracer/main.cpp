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
    int specular;

    Sphere() {}

    Sphere(vec3 cen, double rad, Color col, int spec) {
        center = cen;
        radius = rad;
        color = col;
        specular = spec;
    }
};

enum LightType { ambient, point, directional };

class Light {
  public:
    // TODO! find a way to separate the light type
    // so that I don't need the direction for the ambient light
    LightType type;
    float intensity;
    vec3 direction;
    vec3 position;

    Light() {}

    Light(LightType light, float intens, vec3 dir) {
        switch (light) {
        case ambient:
            type = light;
            intensity = intens;
            break;
        case point:
            type = light;
            intensity = intens;
            position = dir;
            break;
        case directional:
            type = light;
            intensity = intens;
            direction = dir;
            break;
        }
    }
};

class Scene {
  public:
    Color background_color;
    std::vector<Light> lights;
    std::vector<Sphere> spheres;

    Scene() {}
};

Color MultiplyColor(double intensity, Color color) {
    unsigned char r = color.r * intensity > 255 ? 255 : color.r * intensity;
    unsigned char g = color.g * intensity > 255 ? 255 : color.g * intensity;
    unsigned char b = color.b * intensity > 255 ? 255 : color.b * intensity;

    return Color{r, g, b, color.a};
}

vec3 CanvasToViewport(double x, double y) {
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

float ComputeLighting(vec3 position, vec3 normal, vec3 toward_camera,
                      int specular, Scene scene) {
    float intensity{0.0};

    for (Light light : scene.lights) {
        if (light.type == LightType::ambient) {
            intensity += light.intensity;
        } else {
            vec3 light_vector{vec3(0.0, 0.0, 0.0)};
            float light_intensity{0.0};
            if (light.type == LightType::point) {
                light_vector = light.position - position;
                light_intensity = light.intensity;
            } else {
                light_vector = light.direction;
                light_intensity = light.intensity;
            }
            auto normal_light_dot{dot(normal, light_vector)};

            // Diffuse light
            if (normal_light_dot > 0.0) {
                intensity += light_intensity * normal_light_dot /
                             (normal.length() * light_vector.length());
            }

            // Specular light
            if (specular != -1) {
                auto reflection{2 * normal * dot(normal, light_vector) -
                                light_vector};
                auto reflection_dot_to_camera{dot(reflection, toward_camera)};
                if (reflection_dot_to_camera > 0) {
                    intensity +=
                        light.intensity * std::pow(reflection_dot_to_camera /
                                                       (reflection.length() *
                                                        toward_camera.length()),
                                                   specular);
                }
            }
        }
    }
    return intensity;
}

Color TraceRay(vec3 origin, vec3 direction, double t_min, double t_max,
               Scene scene) {
    auto closest_t{std::numeric_limits<double>::infinity()};
    std::optional<Sphere> closest_shpere{};

    for (Sphere entity : scene.spheres) {
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
        auto position{origin + (direction * closest_t)};
        auto normal{position - closest_shpere->center};
        normal = normal / normal.length();

        return MultiplyColor(ComputeLighting(position, normal, -direction,
                                             closest_shpere->specular, scene),
                             closest_shpere->color);
    }
}

Scene CreateScene(Color background, std::vector<Sphere> spheres,
                  std::vector<Light> lights) {
    Scene scene{};
    scene.background_color = background;
    scene.spheres = spheres;
    scene.lights = lights;
    return scene;
}

void PutPixel(int x, int y, Color color) {
    auto screen_width{canvas_width / 2 + x};
    auto screen_height{canvas_height / 2 - y - 1};

    DrawPixel(screen_width, screen_height, color);
}

int main() {
    const std::vector<Sphere> spheres{
        Sphere(vec3(0, -1, 3), 1, Color{255, 0, 0, 255}, 500),
        Sphere(vec3(2, 0, 4), 1, Color{0, 0, 255, 255}, 500),
        Sphere(vec3(-2, 0, 4), 1, Color{0, 255, 0, 255}, 10),
        Sphere(vec3(0, -5001, 0), 5000, Color{255, 255, 0, 255}, 1000)};
    const std::vector<Light> lights{
        Light(LightType::ambient, 0.2, vec3(0, 0, 0)),
        Light(LightType::point, 0.6, vec3(2, 1, 0)),
        Light(LightType::directional, 0.2, vec3(1, 4, 4))};

    Scene scene{CreateScene(RAYWHITE, spheres, lights)};

    InitWindow(canvas_width, canvas_height, "Ray tracer");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (auto x{-canvas_width / 2}; x <= canvas_width / 2; x++) {
            for (auto y{-canvas_height / 2}; y <= canvas_height / 2; y++) {
                auto d{CanvasToViewport(x, y)};
                auto color{TraceRay(camera_position, d, 1,
                                    std::numeric_limits<double>::infinity(),
                                    scene)};
                PutPixel(x, y, color);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
