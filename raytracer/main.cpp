#include "raylib.h"
#include "vec3.h"
#include <array>
#include <cmath>
#include <limits>
#include <optional>
#include <vector>
#include <tuple>

using namespace std;

const auto canvas_width{600};
const auto canvas_height{600};
const auto viewport_width{1.0};
const auto viewport_height{1.0};
const auto distance_from_camera{1.0};
vec3 camera_position{0, 0, 0};

class Sphere {
  public:
    vec3 center;
    double radius;
    Color color;
    int specular;
    float reflective;

    Sphere() {}

    Sphere(vec3 cen, double rad, Color col, int spec, float ref) {
        center = cen;
        radius = rad;
        color = col;
        specular = spec;
        reflective = ref;
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
    vector<Light> lights;
    vector<Sphere> spheres;

    Scene() {}
};

Color MultiplyColor(double intensity, Color color) {
    unsigned char r = color.r * intensity > 255 ? 255 : color.r * intensity;
    unsigned char g = color.g * intensity > 255 ? 255 : color.g * intensity;
    unsigned char b = color.b * intensity > 255 ? 255 : color.b * intensity;

    return Color{r, g, b, color.a};
}

Color AddColor(Color color1, Color color2) {
    color1.r += color2.r;
    color1.g += color2.g;
    color1.b += color2.b;

    return color1;
}

vec3 CanvasToViewport(double x, double y) {
    return {x * viewport_width / canvas_width,
            y * viewport_height / canvas_height, distance_from_camera};
}

array<double, 2> IntersectRaySphere(vec3 origin, vec3 direction,
                                    Sphere sphere) {
    auto r{sphere.radius};
    auto co{origin - sphere.center};

    auto a{dot(direction, direction)};

    auto b{2.0 * dot(co, direction)};
    auto c{dot(co, co) - r * r};

    auto discriminant{b * b - 4.0 * a * c};
    if (discriminant < 0.0) {
        array<double, 2> infinity{numeric_limits<double>::infinity(),
                                  numeric_limits<double>::infinity()};
        return infinity;
    }

    auto t1{(-b + sqrt(discriminant)) / (2.0 * a)};
    auto t2{(-b - sqrt(discriminant)) / (2.0 * a)};
    array<double, 2> t{t1, t2};
    return t;
}

tuple<optional<Sphere>, double> ClosestIntersection(vec3 origin, vec3 direction,
                                                    double t_min, double t_max,
                                                    Scene scene) {
    auto closest_t{numeric_limits<double>::infinity()};
    optional<Sphere> closest_shpere{};

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
    return tuple<optional<Sphere>, double>{
        make_tuple(closest_shpere, closest_t)};
}

vec3 ReflectRay(vec3 ray, vec3 normal) {
    auto normal_dot_ray{dot(normal, ray)};
    return normal * (2.0 * normal_dot_ray) - ray;
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
            auto t_max{0.0};
            if (light.type == LightType::point) {
                light_vector = light.position - position;
                light_intensity = light.intensity;
                t_max = 1.0;
            } else {
                light_vector = light.direction;
                light_intensity = light.intensity;
                t_max = numeric_limits<double>::infinity();
            }

            // Shadow check
            auto [shadow_sphere, shadow_t]{ClosestIntersection(
                position, light_vector, 0.001, t_max, scene)};
            if (shadow_sphere != nullopt) {
                continue;
            }

            auto normal_light_dot{dot(normal, light_vector)};

            // Diffuse light
            if (normal_light_dot > 0.0) {
                intensity += light_intensity * normal_light_dot /
                             (normal.length() * light_vector.length());
            }

            // Specular light
            if (specular != -1) {
                auto reflection{ReflectRay(light_vector, normal)};
                auto reflection_dot_to_camera{dot(reflection, toward_camera)};
                if (reflection_dot_to_camera > 0.0) {
                    intensity +=
                        light_intensity *
                        pow(reflection_dot_to_camera /
                                (reflection.length() * toward_camera.length()),
                            specular);
                }
            }
        }
    }
    return intensity;
}

Color TraceRay(vec3 origin, vec3 direction, double t_min, double t_max,
               int recursion_depth, Scene scene) {
    auto [closest_shpere, closest_t]{
        ClosestIntersection(origin, direction, t_min, t_max, scene)};

    if (closest_shpere == nullopt) {
        return scene.background_color;
    } else {

        auto position{origin + (direction * closest_t)};
        auto normal{position - closest_shpere->center};
        auto normal_norm = normal / normal.length();

        Color local_color{
            MultiplyColor(ComputeLighting(position, normal_norm, -direction,
                                          closest_shpere->specular, scene),
                          closest_shpere->color)};

        if (recursion_depth <= 0 | closest_shpere->reflective <= 0) {
            return local_color;
        }

        auto reflected_ray{ReflectRay(-direction, normal_norm)};
        Color reflected_color{TraceRay(position, reflected_ray, 0.001,
                                       numeric_limits<double>::infinity(),
                                       recursion_depth - 1, scene)};

        auto color1{MultiplyColor(1 - closest_shpere->reflective, local_color)};
        auto color2{MultiplyColor(closest_shpere->reflective, reflected_color)};

        return AddColor(color1, color2);
    }
}
Scene CreateScene(Color background, vector<Sphere> spheres,
                  vector<Light> lights) {
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

void UpdateCamera() {
    if (IsKeyPressed(KEY_W)) {
        vec3 cp{vec3(0, 0, camera_position.z() + 1)};
        camera_position = cp;
    }
    if (IsKeyPressed(KEY_A)) {
        vec3 cp{vec3(camera_position.x() + -1, 0, 0)};
        camera_position = cp;
    }
    if (IsKeyPressed(KEY_S)) {
        vec3 cp{vec3(0, 0, camera_position.z() - 1)};
        camera_position = cp;
    }
    if (IsKeyPressed(KEY_D)) {
        vec3 cp{vec3(camera_position.x() + 1, 0, 0)};
        camera_position = cp;
    }
}

int main() {
    const vector<Sphere> spheres{
        Sphere(vec3(0, -1, 3), 1, Color{255, 0, 0, 255}, 500, 0.2),
        Sphere(vec3(2, 0, 4), 1, Color{0, 0, 255, 255}, 500, 0.3),
        Sphere(vec3(-2, 0, 4), 1, Color{0, 255, 0, 255}, 10, 0.4),
        Sphere(vec3(0, -5001, 0), 5000, Color{255, 255, 0, 255}, 1000, 0.5),
    };
    const vector<Light> lights{
        Light(LightType::ambient, 0.2, vec3(0, 0, 0)),
        Light(LightType::point, 0.6, vec3(2, 1, 0)),
        Light(LightType::directional, 0.2, vec3(1, 4, 4)),
    };

    Scene scene{CreateScene(BLACK, spheres, lights)};

    InitWindow(canvas_width, canvas_height, "Ray tracer");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {

        BeginDrawing();
        ClearBackground(RAYWHITE);
        UpdateCamera();

        for (auto x{-canvas_width / 2}; x <= canvas_width / 2; x++) {
            for (auto y{-canvas_height / 2}; y <= canvas_height / 2; y++) {
                auto d{CanvasToViewport(x, y)};
                auto color{TraceRay(camera_position, d, 1,
                                    numeric_limits<double>::infinity(), 3,
                                    scene)};
                PutPixel(x, y, color);
            }
        }
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
