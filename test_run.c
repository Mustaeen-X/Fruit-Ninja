#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define DEBUG 0

#define ScreenWidth 1450
#define ScreenHeight 800

// dhor eita ekta commment korlam. 
// this is a second comment in 1234 branch. 
void LoadFruits();
void DrawFruitpng(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth);
void UnloadFruits();
void InitializeFruitThrow();
void UpdateFruitThrow(float dt);

int fruit_index;
Vector2 fruitposition, fruitspeed, initialfruitposition, initialfruitspeed;
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

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)){
        if((GetMouseX()>fruitposition.x-100 && GetMouseX()<fruitposition.x+100) && (GetMouseY()>fruitposition.y-100 && GetMouseY()<fruitposition.y+100))
        InitializeFruitThrow();
    }           // for BROHAN (remove after reading) leftclick dhore fruit e dhore rakhle new fruit throw initialize korsi, ur part is to make the fruit sliced and do InitializeFruitThrow() after the sliced pieces fall down
                // run the code once before changing anything to check
                // suggestion: either fruits gula 2 separate sliced fruits er parameters niye DrawTexturePro() use kor, or sliced fruits er png banaye assets folder e dhukaye LoadFruits() er for-loop e add kore DrawFruitpng() use kor
                // ******pura code reading diye kisu na bujhle AI mar to summarize, age AI ke ei pura code copy paste marbi then ekbare last e boro ekta prompt lekhe disi, oitao last e copypaste marish

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
    }                   // loaded 20 fruit assets
}

void InitializeFruitThrow(){
    int fruitrise = GetRandomValue(100,1450);
    int fruitfall = GetRandomValue(100,1450);
    float fruitHspeed = (fruitfall - fruitrise)/5;
    initialfruitposition = (Vector2){fruitrise, ScreenHeight+1500};
    initialfruitspeed=(Vector2){fruitHspeed,-1800};
    fruit_index = GetRandomValue (0,19);
    fruitposition = initialfruitposition;
    fruitspeed = initialfruitspeed;

}                   // before each throw, the position and speed parameters are initialized

void UpdateFruitThrow(float dt){
            fruitspeed = Vector2Add(fruitspeed, Vector2Scale(gravity,dt));
            fruitposition = Vector2Add(fruitposition, Vector2Scale(fruitspeed,dt));
            DrawFruitpng(fruit[fruit_index], fruitposition.x, fruitposition.y, 200);
            if(fruitposition.y > ScreenHeight+300 && fruitspeed.y > 0) InitializeFruitThrow();           
}               // speed and position of fruits are updated while the fruits are onscreen

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
}               // draws each frame of fruit onscreen

void UnloadFruits(){
    for (int i = 0 ; i < 20 ; i++) UnloadTexture(fruit[i]);
}           // unloads all fruits after window is closed


/*The prompt to give to make AI explain the code:

"Here is our current Raylib project code written by my partner. Please act as a senior game developer doing a hand-off summary for me:

Give a high-level overview of what this code currently accomplishes.

Explain the main loop structure, physics math, texture handling, and collision logic step-by-step.

Highlight visual quirks (like origin points in DrawTexturePro) that I need to be aware of before modifying it.

Break down this code for me function-by-function (main, LoadFruits, InitializeFruitThrow, UpdateFruitThrow, DrawFruitpng, and UnloadFruits). 

Explain what each function receives, what state variables it modifies, and how the math/rendering logic inside it works.

Explain this Raylib codebase by creating a text-based flowchart/data-flow diagram showing how fruitposition, fruitspeed, mouse inputs, and drawing update every frame.
Then list the key variables and what they control.

Summarize what is ready to be implemented next."*/