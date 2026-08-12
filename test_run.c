#include "raylib.h"
#include "raymath.h"

#define ScreenWidth 1450
#define ScreenHeight 800

int DrawWatermelon(float TopleftX, float TopleftY, float WMwidth, float WMheight, Color color1, Color color2);

int main(void){
int flag1=0; //flag1=0 means watermelon throw
InitWindow(ScreenWidth, ScreenHeight,"Fruit Ninja");

SetTargetFPS(60);
Vector2 fruitposition = {.66 * ScreenWidth, ScreenHeight};
Vector2 fruitspeed={-200,-1000};
Vector2 gravity={0,750};

while(!WindowShouldClose())
    {
        
        float dt = GetFrameTime();
        
        fruitspeed = Vector2Add(fruitspeed, Vector2Scale(gravity,dt));

        BeginDrawing();
        ClearBackground((Color){89, 50, 27, 255}); //fruit ninja bgc
        
        if (flag1==0) //watermelon throw
            {
            const Color WATERMELONGREEN = {17, 110, 35, 255 };
            const Color WATERMELONSTRIPE = {8, 53, 17, 255};
            
            fruitposition = Vector2Add(fruitposition, Vector2Scale(fruitspeed,dt));
            /*if(fruitposition.y > ScreenHeight - 200 && fruitspeed.y > 0)
                {
                fruitposition.y = ScreenHeight - 200;
                fruitspeed.y *= -1;
                fruitspeed.x /= 1.5;
                fruitspeed.y /= 1.5;
                }*/                            //for bouncing lol 
            DrawWatermelon(fruitposition.x, fruitposition.y, 300, 200, WATERMELONGREEN, WATERMELONSTRIPE);
            }


        EndDrawing();   
    }

CloseWindow();

return 0;
}

int DrawWatermelon(float TopleftX, float TopleftY, float WMwidth, float WMheight, Color color1, Color color2){
    float centreX = TopleftX + WMwidth/2;
    float centreY = TopleftY + WMheight/2;
    float radiusX = WMwidth/2;
    float radiusY = WMheight/2;
    Vector2 linestart[5];
    Vector2 lineend[5];
    DrawEllipse (centreX, centreY, radiusX, radiusY, color1);
    for (int i = 1 ; i < 6 ; i++)
    {
        TopleftX += 50;
        linestart[i].x = TopleftX;
        linestart[i].y = TopleftY;
        lineend[i].x = TopleftX;
        lineend[i].y = TopleftY + WMheight;
        DrawLineEx(linestart[i], lineend[i] , 15, color2);
    }
}