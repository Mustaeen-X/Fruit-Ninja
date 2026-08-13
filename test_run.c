#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define DEBUG 0

#define ScreenWidth 1450
#define ScreenHeight 800

//int DrawWatermelon(float TopleftX, float TopleftY, float WMwidth, float WMheight, Color color1, Color color2);        //tried custom watermelon
int DrawFruitpng(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth);



int main()
{
int flag1;
flag1 = GetRandomValue (0,19);
InitWindow(ScreenWidth, ScreenHeight,"Fruit Ninja");

SetTargetFPS(60);
Vector2 initialfruitposition = {.8 * ScreenWidth, ScreenHeight+1500};
Vector2 initialfruitspeed={-200,-1800};
Vector2 gravity={0,750};

Vector2 fruitposition = initialfruitposition;
Vector2 fruitspeed = initialfruitspeed;
Image temp;
Texture2D fruit[20];

for( int i = 0 ; i < 20 ; i++ )
    {
    char path[150];
    sprintf(path, "assets/sprites/fruits/%d.png", i+1);
    temp = LoadImage(path);
    int maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    fruit[i] = LoadTextureFromImage(temp);
    }

while(!WindowShouldClose())
    {
        
        float dt = GetFrameTime();
        
        BeginDrawing();
        ClearBackground((Color){89, 50, 27, 255}); //fruit ninja bgc
            
            fruitspeed = Vector2Add(fruitspeed, Vector2Scale(gravity,dt));
            fruitposition = Vector2Add(fruitposition, Vector2Scale(fruitspeed,dt));
            
            /*if ( IsKeyDown(KEY_A) ){
                fruitspeed = Vector2Add(fruitspeed, Vector2Scale(gravity,dt));
                fruitposition = Vector2Add(fruitposition, Vector2Scale(fruitspeed,dt));
                }
            else
                {
                fruitposition = initialfruitposition;
                fruitspeed = initialfruitspeed; 
                }*/                                                             //debugging fruitthrow

            /*if(fruitposition.y > ScreenHeight - 80  && fruitspeed.y > 0)
                {
                fruitposition.y = ScreenHeight - 80;
                fruitspeed.y *= -1;
                fruitspeed.x /= 1.1;
                fruitspeed.y /= 1.1;
                }
            if(fruitposition.y < 0  && fruitspeed.y < 0)
                {
                fruitposition.y = 0;
                fruitspeed.y *= -1;
                fruitspeed.x /= 1.1;
                fruitspeed.y /= 1.1;
                } 
            if(fruitposition.x > ScreenWidth - 80  && fruitspeed.x > 0)
                {
                fruitposition.x = ScreenWidth - 80;
                fruitspeed.x *= -1;
                fruitspeed.x /= 1.1;
                fruitspeed.y /= 1.1;
                }
            if(fruitposition.x < 0  && fruitspeed.x < 0)
                {
                fruitposition.x = 0;
                fruitspeed.x *= -1;
                fruitspeed.x /= 1.1;
                fruitspeed.y /= 1.1;
                }*/                                                             //for bouncing lol
            
            /*const Color WATERMELONGREEN = {17, 110, 35, 255 };
            const Color WATERMELONSTRIPE = {8, 53, 17, 255};
            DrawWatermelon(fruitposition.x, fruitposition.y, 300, 200, WATERMELONGREEN, WATERMELONSTRIPE);*/  

            
            DrawFruitpng(fruit[flag1], fruitposition.x, fruitposition.y, 200);

            if(fruitposition.y > ScreenHeight+300 && fruitspeed.y > 0) 
                {
                flag1 = GetRandomValue( 0 , 19 );
                fruitposition = initialfruitposition;
                fruitspeed = initialfruitspeed;
                }
            


        EndDrawing();   
    }
    for (int i = 0 ; i < 20 ; i++)
    UnloadTexture(fruit[i]);

CloseWindow();

return 0;
}

/*int DrawWatermelon(float TopleftX, float TopleftY, float WMwidth, float WMheight, Color color1, Color color2){
    float centreX = TopleftX + WMwidth/2;
    float centreY = TopleftY + WMheight/2;
    float radiusX = WMwidth/2;
    float radiusY = WMheight/2;
    Vector2 linestart[5];
    Vector2 lineend[5];
    DrawEllipse (centreX, centreY, radiusX, radiusY, color1);
    for (int i = 1 ; i < 6 ; i++)
    {
        TopleftY += 33;
        linestart[i].x = TopleftX;
        linestart[i].y = TopleftY;
        lineend[i].x = TopleftX + WMwidth;
        lineend[i].y = TopleftY;
        DrawLineEx(linestart[i], lineend[i] , 10, color2);
    }
}*/

int DrawFruitpng(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth){
    Vector2 center = {fruitwidth/2, fruitwidth/2};
    float dt = GetFrameTime();
    static float rotation = 10;
    rotation += 70*dt; 
    float temp = (fruit.width - fruit.height)/2;
    DrawTexturePro(fruit,
                (Rectangle){0 , 0, fruit.width, fruit.height}, 
                (Rectangle){TopleftX, TopleftY + temp, fruitwidth, fruitwidth - temp}, 
                center, rotation+10 , WHITE);
}