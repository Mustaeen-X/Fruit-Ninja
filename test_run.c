#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define DEBUG 0

#define ScreenWidth 1450
#define ScreenHeight 800

void LoadFruits();
void DrawFruitpng(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth);
void UnloadFruits();
void InitializeFruitThrow();
void UpdateFruitThrow(float dt);

int flag1;
Vector2 fruitposition, fruitspeed;
Vector2 initialfruitposition = {.8 * ScreenWidth, ScreenHeight+1500};
Vector2 initialfruitspeed={-200,-1800};
Vector2 gravity={0,750};

Texture2D fruit[100];

int main()
{
InitWindow(ScreenWidth, ScreenHeight,"Fruit Ninja");
SetTargetFPS(60);
LoadFruits();
InitializeFruitThrow();
while(!WindowShouldClose())
    {
    float dt = GetFrameTime();
    BeginDrawing();
    ClearBackground((Color){89, 50, 27, 255}); //fruit ninja bgc
            
    UpdateFruitThrow(dt);
    
    EndDrawing();   
    }
    UnloadFruits();

CloseWindow();

return 0;
}

void LoadFruits(){
    for( int i = 0 ; i < 20 ; i++ )
    {
    char path[150];
    sprintf(path, "assets/sprites/fruits/%d.png", i+1);
    Image temp;
    temp = LoadImage(path);
    int maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    fruit[i] = LoadTextureFromImage(temp);
    UnloadImage(temp);
    }
}

void InitializeFruitThrow(){
    flag1 = GetRandomValue (0,19);
    fruitposition = initialfruitposition;
    fruitspeed = initialfruitspeed;

}

void UpdateFruitThrow(float dt){

            fruitspeed = Vector2Add(fruitspeed, Vector2Scale(gravity,dt));
            fruitposition = Vector2Add(fruitposition, Vector2Scale(fruitspeed,dt));

            
            DrawFruitpng(fruit[flag1], fruitposition.x, fruitposition.y, 200);

            if(fruitposition.y > ScreenHeight+300 && fruitspeed.y > 0) InitializeFruitThrow();
            
}

void DrawFruitpng(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth){
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

void UnloadFruits(){
    for (int i = 0 ; i < 20 ; i++) UnloadTexture(fruit[i]);
}