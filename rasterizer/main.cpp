#include "raylib.h"
#include <algorithm>
#include <cmath>
#include <iostream>
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
    if (i0 == i1) {
        vector<float> d;
        d.push_back(d0);
        return d;
    }
    vector<float> values;
    float a{(d1 - d0) / (i1 - i0)};
    float d{d0};

    for (float i{i0}; i <= i1; i++) {
        values.push_back(d);
        d = d + a;
    }

    return values;
}

void myDrawLine(Point point0, Point point1, Color color) {
    if (fabs(point1.x - point0.x) > fabs(point1.y - point0.y)) {
        if (point0.x > point1.x) {
            swap(point0, point1);
        }

        vector<float> ys{Interpolate(point0.x, point0.y, point1.x, point1.y)};
        for (float x{point0.x}; x <= point1.x; x++) {
            PutPixel(x, ys[int(x) - (int)point0.x], color);
        }
    } else {

        if (point0.y > point1.y) {
            swap(point0, point1);
        }

        vector<float> xs{Interpolate(point0.y, point0.x, point1.y, point1.x)};
        for (float y{point0.y}; y <= point1.y; y++) {
            PutPixel(xs[int(y) - (int)point0.y], y, color);
        }
    }
}

int main() {
    Point point0{Point(-50, -200)};
    Point point1{Point(60, 240)};

    Point point2{Point(-200, -100)};
    Point point3{Point(240, 120)};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(canvas_width, canvas_height, "Rasterizer");
    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        // std::cout << GetFPS() << '\n';
        BeginDrawing();
        ClearBackground(RAYWHITE);

        myDrawLine(point0, point1, BLACK);
        myDrawLine(point2, point3, BLACK);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
