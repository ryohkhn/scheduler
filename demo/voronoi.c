#include "raylib.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#include "../include/sched.h"

#define LENGTH 50

#define WIDTH 1200
#define HEIGHT 800

int nthreads = -1000;

struct param {
    Vector2 *points;
    int length;
    Color *pixelColorMap;
    int x;
    int y;
    int width;
    int height;
};

Vector2 getRandomPoint(int x, int y, int width, int height) {
    int i = rand() % ((width + 1) - x) + x;
    int j = rand() % ((height + 1) - y) + y;
    Vector2 res = {i, j};
    return res;
}

Color getCustomColor(int i) {
    double hue = (i + (25 - i) * 2) % 360;
    double saturation = (50 + i) % 100;
    double value = (50 + i) % 100;
    return ColorFromHSV(hue, saturation, value);
}

void generatePixelColorsSubArea(void *closure, struct scheduler *) {
    struct param *args = (struct param *) closure;
    Vector2 *points = args->points;
    int length = args->length;
    Color *pixelColorMap = args->pixelColorMap;
    int x = args->x;
    int y = args->y;
    int width = args->width;
    int height = args->height;

    for (int i = x; i < width; i++) {
        for (int j = y; j < height; j++) {
            double distMin = 10000;
            int index = 0;
            for (int k = 0; k < length; k++) {
                double dist = sqrt(pow(points[k].x - i, 2) + pow(points[k].y - j, 2));
                if (dist < distMin) {
                    distMin = dist;
                    index = k;
                }
            }
            pixelColorMap[i * HEIGHT + j] = getCustomColor(index);
        }
    }
}
void generatePixelColors(void *closure, struct scheduler *s) {
    struct param *args = (struct param *) closure;
    int increment = HEIGHT / get_nbthreads(s);
    for (int i = 0; i < get_nbthreads(s); i++) {
        struct param *sub_args = malloc(sizeof(struct param));
        if (sub_args == NULL) {
            perror("Failed to allocate sub_args");
            exit(2);
        }
        sub_args->points = args->points;
        sub_args->length = args->length;
        sub_args->pixelColorMap = args->pixelColorMap;
        sub_args->x = 0;
        sub_args->y = increment * i;
        sub_args->width = WIDTH;
        sub_args->height = increment * (i + 1);
        sched_spawn(generatePixelColorsSubArea, sub_args, s);
    }
}

int main(void) {
    InitWindow(WIDTH, HEIGHT, "Salut");

    SetTargetFPS(60);
    srand(time(NULL));

    Color *pixelColorMap = malloc((WIDTH * HEIGHT) * sizeof(Color));
    if (pixelColorMap == NULL) {
        perror("Failed to allocate pixelColor array");
        exit(1);
    }

    Vector2 *points = malloc(sizeof(Vector2) * LENGTH);
    if (points == NULL) {
        perror("Failed to allocate points array");
        exit(1);
    }
    int sum = 0;
    while (sum < LENGTH) {
        points[sum] = getRandomPoint(0, 0, WIDTH, HEIGHT);
        sum++;
    }
    Vector2 velocity[LENGTH] = {0};
    int counter = 0;
    while (counter < LENGTH) {
        velocity[counter].x = 0;
        velocity[counter].y = 0;
        counter++;
    }
    int cycles = 0;


    while (!WindowShouldClose()) {
        BeginDrawing();
        cycles--;

        ClearBackground(BLACK);

        struct param *args = malloc(sizeof(struct param));
        args->points = points;
        args->length = LENGTH;
        args->pixelColorMap = pixelColorMap;
        args->x = 0;
        args->y = 0;
        args->width = 0;
        args->height = 0;

        if (sched_init(-1, 50, generatePixelColors, args) == -1) {
            perror("Failed to init sched");
            exit(1);
        }
        free(args);
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                DrawPixel(x, y, pixelColorMap[x * HEIGHT + y]);
            }
        }


        if (cycles < 0) {
            cycles = 150;
            for (int i = 0; i < LENGTH; i++) {
                velocity[i].x = (rand() % ((WIDTH + 1) - 0) + 0);
                velocity[i].y = (rand() % ((HEIGHT + 1) - 0) + 0);
            }
        }

        for (int i = 0; i < LENGTH; i++) {
            DrawCircle(points[i].x, points[i].y, 7, WHITE);
            double newX = velocity[i].x - points[i].x;
            double newY = velocity[i].y - points[i].y;
            points[i].x = points[i].x +(GetFrameTime() * newX);
            points[i].y = points[i].y + (GetFrameTime() * newY);
        }
        char fps[10];
        sprintf(fps, "%d", GetFPS());
        DrawText(fps, 10, 10, 24, BLACK);

        EndDrawing();
    }
    free(pixelColorMap);
    free(points);
    return 0;
}
