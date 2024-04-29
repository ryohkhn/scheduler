#include "raylib.h"

int main(void) {
    const int width = 800, height = 450;

    InitWindow(width, height, "Salut");

    SetTargetFPS(60);

    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(BLACK);

        DrawText("Ouais ouais ouais", 190, 200, 20, WHITE);

        EndDrawing();
    }
    return 0;
}
