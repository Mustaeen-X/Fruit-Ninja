#include "raylib.h"
#define width 1450
#define height 800

int main(){
InitWindow(width,height,"fruitninja");

while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground((Color){90, 45, 30, 255}); //fruit ninja bgc
        EndDrawing();   
    }

CloseWindow();

return 0;
}