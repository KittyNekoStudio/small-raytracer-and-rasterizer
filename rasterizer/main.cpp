#include "raylib.h"
#include "vec3.h"
#include <algorithm>
#include <cmath>
#include <vector>
// #include <iostream>
#include <utility>

using namespace std;
const unsigned int CANVAS_WIDTH{600};
const unsigned int CANVAS_HEIGHT{600};
const float VIEWPORT_WIDTH{1.0};
const float VIEWPORT_HEIGHT{1.0};
const float DISTANCE_FROM_CAMERA_TO_VIEWPORT{1.0};

struct Point {
  float x;
  float y;
  float h;
  Point() {}

  Point(float xVal, float yVal, float hVal) {
    x = xVal;
    y = yVal;
    h = hVal;
  }
};

Color MultiplyColor(double intensity, Color color) {
  unsigned char r = color.r * intensity > 255 ? 255 : color.r * intensity;
  unsigned char g = color.g * intensity > 255 ? 255 : color.g * intensity;
  unsigned char b = color.b * intensity > 255 ? 255 : color.b * intensity;

  return Color{r, g, b, color.a};
}

void PutPixel(int x, int y, Color color) {
  auto screen_width{CANVAS_WIDTH / 2 + x};
  auto screen_height{CANVAS_HEIGHT / 2 - y - 1};

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

void DrawShadedTriangle(Point p0, Point p1, Point p2, Color color) {
  if (p1.y < p0.y)
    swap(p1, p0);
  if (p2.y < p0.y)
    swap(p2, p0);
  if (p2.y < p1.y)
    swap(p2, p1);

  vector<float> x01{Interpolate(p0.y, p0.x, p1.y, p1.x)};
  vector<float> h01{Interpolate(p0.y, p0.h, p1.y, p1.h)};

  vector<float> x12{Interpolate(p1.y, p1.x, p2.y, p2.x)};
  vector<float> h12{Interpolate(p1.y, p1.h, p2.y, p2.h)};

  vector<float> x02{Interpolate(p0.y, p0.x, p2.y, p2.x)};
  vector<float> h02{Interpolate(p0.y, p0.h, p2.y, p2.h)};

  vector<float> x012{};
  x01.pop_back();
  x012.insert(x012.begin(), x01.begin(), x01.end());
  x012.insert(x012.end(), x12.begin(), x12.end());

  vector<float> h012{};
  h01.pop_back();
  h012.insert(h012.begin(), h01.begin(), h01.end());
  h012.insert(h012.end(), h12.begin(), h12.end());

  vector<float> xLeft{};
  vector<float> xRight{};
  vector<float> hLeft{};
  vector<float> hRight{};

  auto m{x012.size() / 2};
  if (x02[m] < x012[m]) {
    xLeft = x02;
    hLeft = h02;

    xRight = x012;
    hRight = h012;
  } else {
    xLeft = x012;
    hLeft = h012;

    xRight = x02;
    hRight = h02;
  }

  for (float y{p0.y}; y < p2.y; y++) {
    float xL{xLeft[y - p0.y]};
    float xR{xRight[y - p0.y]};

    vector<float> hSegment{
        Interpolate(xL, hLeft[y - p0.y], xR, hRight[y - p0.y])};
    for (float x{xL}; x < xR; x++) {
      Color shadedColor{MultiplyColor(hSegment[x - xL], color)};
      PutPixel(x, y, shadedColor);
    }
  }
}

Point ViewPortToCanvas(float x, float y) {
  return Point{x * (float)CANVAS_WIDTH / VIEWPORT_WIDTH,
               y * (float)CANVAS_HEIGHT / VIEWPORT_HEIGHT,
               // TODO! make a way to change this variable
               1};
}

Point ProjectVertex(vec3 v) {
  return ViewPortToCanvas(
      (float)v.x() * DISTANCE_FROM_CAMERA_TO_VIEWPORT / (float)v.z(),
      (float)v.y() * DISTANCE_FROM_CAMERA_TO_VIEWPORT / (float)v.z());
}

int main() {
  // The four "front" faces
  vec3 vAf{-2.0, -0.5, 5.0};
  vec3 vBf{-2.0, 0.5, 5.0};
  vec3 vCf{-1.0, 0.5, 5.0};
  vec3 vDf{-1.0, -0.5, 5.0};

  // The four "back" faces
  vec3 vAb{-2.0, -0.5, 6.0};
  vec3 vBb{-2.0, 0.5, 6.0};
  vec3 vCb{-1.0, 0.5, 6.0};
  vec3 vDb{-1.0, -0.5, 6.0};

  SetConfigFlags(FLAG_WINDOW_RESIZABLE);
  InitWindow(CANVAS_WIDTH, CANVAS_HEIGHT, "Rasterizer");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    // std::cout << GetFPS() << '\n';
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // The "front" face
    myDrawLine(ProjectVertex(vAf), ProjectVertex(vBf), BLUE);
    myDrawLine(ProjectVertex(vBf), ProjectVertex(vCf), BLUE);
    myDrawLine(ProjectVertex(vCf), ProjectVertex(vDf), BLUE);
    myDrawLine(ProjectVertex(vDf), ProjectVertex(vAf), BLUE);

    // The "back" face
    myDrawLine(ProjectVertex(vAb), ProjectVertex(vBb), RED);
    myDrawLine(ProjectVertex(vBb), ProjectVertex(vCb), RED);
    myDrawLine(ProjectVertex(vCb), ProjectVertex(vDb), RED);
    myDrawLine(ProjectVertex(vDb), ProjectVertex(vAb), RED);
    // The front-to-back edges
    myDrawLine(ProjectVertex(vAf), ProjectVertex(vAb), GREEN);
    myDrawLine(ProjectVertex(vBf), ProjectVertex(vBb), GREEN);
    myDrawLine(ProjectVertex(vCf), ProjectVertex(vCb), GREEN);
    myDrawLine(ProjectVertex(vDf), ProjectVertex(vDb), GREEN);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
