#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

#include "../include/sched.h"

#define LENGTH 200

#define WIDTH 1920
#define HEIGHT 1080

int nthreads = -1000;

struct point {
    float x;
    float y;
};
struct color {
    int r;
    int g;
    int b;
};

struct param {
    struct point *points;
    int length;
    struct color *pixelColorMap;
    int x;
    int y;
    int width;
    int height;
};



struct point getRandomPoint(int x, int y, int width, int height) {
    int i = rand() % ((width + 1) - x) + x;
    int j = rand() % ((height + 1) - y) + y;
    struct point res = {i, j};
    return res;
}

struct color getCustomColor(int i) {
    int red, green, blue;

    if (i % 5 == 0) {
        red = 125;
        green = (int) (i * 4 * M_1_PI) % 255;
        blue = (i * 3) % 255;
    } else if (i % 5 == 1) {
        green = 125;
        red = (i * 5) % 255;
        blue = (int) (i * 3 * M_1_PI) % 255;
    } else if (i % 5 == 2) {
        blue = 125;
        red = (i * 9) % 255;
        green = (int) (i * 2 * M_1_PI) % 255;
    } else if (i % 5 == 3) {
        blue = 125;
        red = (int) (i * 6 * M_1_PI) % 255;
        green = 165;
    } else if (i % 5 == 4) {
        blue = 125;
        red = 75;
        green = (int) (i * 8 * M_1_PI) % 255;
    } else {
        blue = (125 - i) % 255;
        red = 75;
        green = (int) (i * 8 * M_1_PI) % 255;
    }
    struct color res = {red, green, blue};
    return res;
}

void generatePixelColorsSubArea(void *closure, struct scheduler *) {
    struct param *args = (struct param *) closure;
    struct point *points = args->points;
    int length = args->length;
    struct color *pixelColorMap = args->pixelColorMap;
    int x = args->x;
    int y = args->y;
    int width = args->width;
    int height = args->height;

    for (int i = x; i < width; i++) {
        for (int j = y; j < height; j++) {
            double distMin = 10000;
            int index = 0;
            for (int k = 0; k < length; k++) {
                double tmp = pow(points[k].x - i, 2) + pow(points[k].y - j, 2);
                double dist = sqrt(tmp);
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
    srand(time(NULL));

    struct color *pixelColorMap = malloc((WIDTH * HEIGHT) * sizeof(struct color));
    if (pixelColorMap == NULL) {
        perror("Failed to allocate pixelColor array");
        exit(1);
    }

    struct point *points = malloc(sizeof(struct point ) * LENGTH);
    if (points == NULL) {
        perror("Failed to allocate points array");
        exit(1);
    }
    int sum = 0;
    while (sum < LENGTH) {
        points[sum] = getRandomPoint(0, 0, WIDTH, HEIGHT);
        sum++;
    }

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

    int fd = open("out/pixelInfo.txt", O_CREAT | O_WRONLY | O_TRUNC, 0666);
    char buff[100];
    if (fd != -1) {
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                memset(buff, 0, 100);
                int r = sprintf(buff, "%d;%d;%d\n", pixelColorMap[x * HEIGHT + y].r, pixelColorMap[x * HEIGHT + y].g, pixelColorMap[x * HEIGHT + y].b);
                if (write(fd, buff, r) == -1) {
                    perror("Failed to write");
                    exit(1);
                }
            }
        }
    }

    free(pixelColorMap);
    free(points);
    return 0;
}
