#include <stdlib.h>
#include <errno.h>
#include <math.h>
#include <complex.h>
#include <unistd.h>
#include <gtk/gtk.h>
#include <pthread.h>
#include <stdio.h>
#include <semaphore.h>

#include "../include/sched.h"

#define ITERATIONS 1000

static double scale;
static int dx, dy;

int nthreads;

struct param {
    double x1;
    double x2;
    double y1;
    double y2;
    unsigned *data;
    int stride;
};

struct init_param {
    unsigned *data;
    double x1, x2, y1, y2;
    int stride;
};
static int mandel(double complex c) {
    double complex z = 0.0;
    int i = 0;
    while(i < ITERATIONS && creal(z) * creal(z) + cimag(z) * cimag(z) <= 4.0) {
        z = z * z + c;
        i++;
    }
    return i;
}
static double complex toc(int x, int y) {
    return ((x - dx) + I * (y - dy)) / scale;
}
static unsigned int torgb(int n) {
    unsigned char r, g, b;

    if(n < 128) {
        int v = 2 * n;
        r = v;
        g = 0;
        b = 255 - v;
    } else if(n < 256) {
        int v = 2 * (n - 128);
        r = 0;
        g = v;
        b = 255 - v;
    } else if(n < 512) {
        int v = n - 256;
        r = 255 - v;
        g = v;
        b = 0;
    } else if(n < 1024) {
        int v = (n - 512) / 2;
        g = 255;
        r = b = v;
    } else {
        r = g = b = 255;
    }

    return r << 16 | g << 8 | b;
}
static void calculate(double x1, double x2, double y1, double y2, unsigned *data, int stride) {
    for(int j = 0; j < y2 - y1; j++) {
        for(int i = 0; i < x2 - x1; i++) {
            unsigned rgb = torgb(mandel(toc(x1 + i, y1 + j)));
            data[(stride / 4) * j + i] = rgb;
        }
    }
}

void calculate_thread_bis(void *closure, struct scheduler *) {
    struct param *p = (struct param *) closure;
    calculate(p -> x1, p -> x2, p -> y1, p -> y2, p -> data, p -> stride);
}
void calculate_sched(void *closure, struct scheduler *s) {
    struct init_param *info = (struct init_param *) closure;
    int increment = (info->y2 - info->y1) / (double) get_nbthreads(s);
    for (int i = 0; i < get_nbthreads(s); i++) {
        struct param *args = malloc(sizeof (struct param));
        args -> x1 = info->x1;
        args -> x2 = info->x2;
        args -> y1 = info->y1 + (increment * i);
        args -> y2 = info->y1 + (increment * (i + 1));
        args -> data = &info->data[(i * increment) * info->stride / 4];
        args -> stride = info->stride;
        sched_spawn(calculate_thread_bis, args, s);
    }
}
static void draw_cb(
        GtkDrawingArea *drawing_area,
        cairo_t *cr,
        int width,
        int height,
        gpointer closure)
{
    printf("drawing\n");
    double x1, y1, x2, y2;
    struct timespec t0, t1;

    cairo_clip_extents(cr, &x1, &y1, &x2, &y2);

    clock_gettime(CLOCK_MONOTONIC, &t0);

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, x2 - x1, y2 - y1);
    unsigned *data = (unsigned*)cairo_image_surface_get_data(surface);
    int stride = cairo_image_surface_get_stride(surface);

    struct init_param *info = malloc(sizeof(struct init_param));
    if (info == NULL) {
        perror("Failed to malloc init_param");
        exit(1);
    }
    info->x1 = x1;
    info->y1 = y1;
    info->x2 = x2;
    info->y2 = y2;
    info->data = data;
    info->stride = stride;

    if (sched_init(nthreads, 50, calculate_sched, info) == -1) {
        perror("Failed to init sched");
        exit(1);
    }
    free(info);

    cairo_surface_mark_dirty(surface);
    cairo_set_source_surface(cr, surface, x1, y1);
    cairo_paint(cr);
    cairo_surface_destroy(surface);

    clock_gettime(CLOCK_MONOTONIC, &t1);
    printf("Repaint done in %.2lfs\n",
           ((double)t1.tv_sec - t0.tv_sec) +
           ((double)t1.tv_nsec - t0.tv_nsec) / 1.0E9);
}

static void pressed(
        GtkGestureClick *gesture,
        int n_press,
        double x,
        double y,
        GtkWidget *area)
{
    int button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
    int width = gtk_widget_get_width(area);
    int height = gtk_widget_get_height(area);
    double dscale = 1.0;

    if(button == 1)
        dscale = sqrt(2.0);
    else if(button == 3)
        dscale = 1.0 / sqrt(2.0);
    scale *= dscale;

    dx = width / 2 + (dx - x) * dscale;
    dy = height / 2 + (dy - y) * dscale;

    gtk_widget_queue_draw(area);
}

static void
close_window(void)
{
}

static void
activate(GtkApplication* app, gpointer user_data)
{
    GtkWidget *window;
    GtkWidget *frame;
    GtkWidget *drawing_area;
    GtkGesture *press;

    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Mandelbrot");

    g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);

    frame = gtk_frame_new(NULL);
    gtk_window_set_child(GTK_WINDOW(window), frame);

    drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 1024, 512);

    gtk_frame_set_child(GTK_FRAME(frame), drawing_area);

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(drawing_area), draw_cb, NULL, NULL);

    press = gtk_gesture_click_new();
    gtk_gesture_single_set_button(GTK_GESTURE_SINGLE(press), 0);
    gtk_widget_add_controller(drawing_area, GTK_EVENT_CONTROLLER(press));

    g_signal_connect(press, "pressed", G_CALLBACK(pressed), drawing_area);

    dx = 512;
    dy = 256;
    scale = 1024 / 4.0;

    gtk_widget_show(window);
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        nthreads = atoi(argv[1]);
        if (nthreads <= 0) {
            nthreads = sched_default_threads();
        }
    } else {
        nthreads = sched_default_threads();
    }
    printf("nthreads set to: %d\n", nthreads);


    GtkApplication *app;
    int status;

    app = gtk_application_new("fr.irif.jch.mandelbrot", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    status = g_application_run(G_APPLICATION(app), 1, &argv[0]);
    g_object_unref(app);

    return status;
}
