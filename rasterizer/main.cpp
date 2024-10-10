#include "raylib.h"
#include <algorithm>
#include <cmath>
// #include <iostream>
#include <utility>

using namespace std;
const auto canvas_width{600};
const auto canvas_height{600};

struct Point {
    float x;
    float y;
    Point() {}

    Point(float xPos, float yPos) {
        x = xPos;
        y = yPos;
    }
};

void PutPixel(int x, int y, Color color) {
    auto screen_width{canvas_width / 2 + x};
    auto screen_height{canvas_height / 2 - y - 1};

    DrawPixel(screen_width, screen_height, color);
}

vector<float> Interpolate(float i0, float d0, float i1, float d1) {
    vector<float> values;
    if (i0 == i1) {
        values.push_back(i0);
        return values;
    }

    float a{(d1 - d0) / (i1 - i0)};
    float d{d0};

    for (float i{i0}; i < i1; i++) {
        values.push_back(d);
        d = d + a;
    }

    return values;
}

void myDrawLine(Point point1, Point point2, Color color) {
    if (fabs(point2.x - point1.x) > fabs(point2.y - point1.y)) {
        if (point1.x > point2.x) {
            swap(point1, point2);
        }

        vector<float> ys{Interpolate(point1.x, point1.y, point2.x, point2.y)};
        for (float x{point1.x}; x <= point2.x; x++) {
            PutPixel(x, ys[int(x) - (int)point1.x], color);
        }
    } else {

        if (point1.y > point2.y) {
            swap(point1, point2);
        }

        vector<float> xs{Interpolate(point1.y, point1.x, point2.y, point2.x)};
        for (float y{point1.y}; y <= point2.y; y++) {
            PutPixel(xs[int(y) - (int)point1.y], y, color);
        }
    }
}

void DrawWireframeTriangle(Point p0, Point p1, Point p2, Color color) {
    myDrawLine(p0, p1, color);
    myDrawLine(p1, p2, color);
    myDrawLine(p2, p0, color);
}

void DrawFilledTriangle(Point p0, Point p1, Point p2, Color color) {
    auto corner0{p0};
    auto corner1{p1};
    auto corner2{p2};

    if (corner1.y < corner0.y)
        swap(corner1, corner0);
    if (corner2.y < corner0.y)
        swap(corner2, corner0);
    if (corner2.y < corner1.y)
        swap(corner2, corner1);

    vector<float> x01{Interpolate(p0.y, p0.x, p1.y, p1.x)};
    vector<float> x12{Interpolate(p1.y, p1.x, p2.y, p2.x)};
    vector<float> x02{Interpolate(p0.y, p0.x, p2.y, p2.x)};

    vector<float> x012{};
    x01.pop_back();
    x012.insert(x012.begin(), x01.begin(), x01.end());
    x012.insert(x012.end(), x12.begin(), x12.end());
    auto m{x012.size() / 2};

    vector<float> xLeft{};
    vector<float> xRight{};

    if (x02[m] < x012[m]) {
        xLeft = x02;
        xRight = x012;
    } else {
        xLeft = x012;
        xRight = x02;
    }

    for (auto y{p0.y}; y < p2.y; y++) {
        for (auto x{xLeft[y - p0.y]}; x < xRight[y - p0.y]; x++) {
            PutPixel(x, y, color);
        }
    }
}
int main() {
    Point point1{Point(-200, -250)};
    Point point2{Point(200, 50)};
    Point point3{Point(20, 250)};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(canvas_width, canvas_height, "Rasterizer");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // std::cout << GetFPS() << '\n';
        BeginDrawing();
        ClearBackground(RAYWHITE);

        DrawFilledTriangle(point1, point2, point3, GREEN);
        DrawWireframeTriangle(point1, point2, point3, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
