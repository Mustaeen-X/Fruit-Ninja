#include "raylib.h"
#include "raymath.h"
#include <stdio.h>

#define DEBUG 0
#define ScreenWidth 1450
#define ScreenHeight 800
// here is the 3rd new comment. 
void LoadFruits();
void LoadSounds();
void DrawFruit(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth, float rotation);
void UnloadFruits();
void InitializeFruitThrow();
void UpdateFruitThrow(int sliced, float dt);
void LoadGame();
void ShowMenu();
void PlayGame(float dt);
void CustomCursor();

int fruit_index, sliced, splatter_index, splatterX=0, splatterY=0;
float rotation = 0;
Vector2 fruitposition, fruitspeed, initialfruitposition, initialfruitspeed;
Vector2 gravity={0,1200};

Texture2D fruit[20], deadfruit[20], splatter[8], KNIFE;
Sound Throw[4], Slash[3], Splat[3];

int main()
{
LoadGame();
while(!WindowShouldClose())
    {
        float dt = GetFrameTime();
        BeginDrawing();
        ClearBackground((Color){89, 50, 27, 255}); //fruit ninja bgc
        

        PlayGame(dt);
            
        CustomCursor();
        
        EndDrawing(); 
        
    }
    UnloadFruits();

CloseWindow();

return 0;
}

void LoadGame(){
    InitWindow(ScreenWidth, ScreenHeight,"Fruit Ninja");
    InitAudioDevice();
    SetTargetFPS(60);
    LoadSounds();
    LoadFruits();
    InitializeFruitThrow();
}

void LoadFruits(){
    char path[150];
    Image temp;
    int maxdim;
    temp = LoadImage("assets/KNIFE.png");
    maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    KNIFE = LoadTextureFromImage(temp);

    for( int i = 0 ; i < 20 ; i++ )
    {
    sprintf(path, "assets/sprites/fruits/%d.png", i+1);
    temp = LoadImage(path);
    maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    fruit[i] = LoadTextureFromImage(temp);

    sprintf(path, "assets/sprites/fruits/%da.png", i+1);
    temp = LoadImage(path);
    maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    deadfruit[i] = LoadTextureFromImage(temp);
    UnloadImage(temp);
    }

    for( int i = 0 ; i < 8 ; i++ )
    {
    sprintf(path, "assets/sprites/Splatter/%d.png", i+1);
    temp = LoadImage(path);
    maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    splatter[i] = LoadTextureFromImage(temp);
    UnloadImage(temp);
    }
    }                   // loaded 20 fruit assets, 20 dead fruit assets, 8 splatter effects

void LoadSounds(){
    for( int i = 0 ; i < 3 ; i++ )
    {
    char path[150];
    sprintf(path, "assets/audio/throw%d.mp3", i+1);
    Throw[i] = LoadSound(path);
    sprintf(path, "assets/audio/Slash%d.mp3", i+1);
    Slash[i] = LoadSound(path);
    sprintf(path, "assets/audio/fruitsplat%d.mp3", i+1);
    Splat[i] = LoadSound(path);
    }
}

void CustomCursor(){
    int X = GetMouseX(), Y = GetMouseY();
    HideCursor();
    DrawFruit(KNIFE, X, Y, 100, 0);
}

void InitializeFruitThrow(){
    int fruitrise = GetRandomValue(100,1450);
    int fruitfall = GetRandomValue(100,1450);
    float fruitHspeed = (fruitfall - fruitrise)/5;
    initialfruitposition = (Vector2){fruitrise, ScreenHeight};
    initialfruitspeed=(Vector2){fruitHspeed, GetRandomValue(-1300,-900)};
    fruit_index = GetRandomValue (0,19);
    if (fruit_index == 0 || fruit_index == 6 || fruit_index == 7 || fruit_index == 8) splatter_index = 0;
    if (fruit_index == 10) splatter_index = 1;
    if (fruit_index == 4 || fruit_index == 14) splatter_index = 2;
    if (fruit_index == 2 || fruit_index == 11 || fruit_index == 15 || fruit_index == 13) splatter_index = 3;
    if (fruit_index == 1 || fruit_index == 3 || fruit_index == 5 || fruit_index == 9) splatter_index = 4;
    if (fruit_index == 17 || fruit_index == 18) splatter_index = 5;
    if (fruit_index == 16) splatter_index = 6;
    if (fruit_index == 19) splatter_index = 7;
    fruitposition = initialfruitposition;
    fruitspeed = initialfruitspeed;
    PlaySound(Throw[GetRandomValue(0,2)]);
    sliced = 0;
    rotation = 0;
}                   // before each throw, the position and speed parameters are initialized, with the throwing audio and splatter color

void PlayGame(float dt){
    UpdateFruitThrow(sliced, dt);

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) PlaySound(Slash[GetRandomValue(0,2)]);
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !sliced){
        splatterX = GetMouseX();
        splatterY = GetMouseY(); 
        if(( GetMouseX() > fruitposition.x-100 && GetMouseX() < fruitposition.x+100 ) && ( GetMouseY() > fruitposition.y-100 && GetMouseY() < fruitposition.y+100 ))
        {
            PlaySound(Splat[GetRandomValue(0,2)]);
            sliced = 1;
        }
    }  
}

void UpdateFruitThrow(int sliced, float dt){
    rotation += 70*dt;
    fruitspeed = Vector2Add(fruitspeed, Vector2Scale(gravity,dt));
    fruitposition = Vector2Add(fruitposition, Vector2Scale(fruitspeed,dt));
    if(!sliced) DrawFruit(fruit[fruit_index], fruitposition.x, fruitposition.y, 200, rotation);
    else {
        DrawFruit(splatter[splatter_index], splatterX, splatterY, 500, 0);
        DrawFruit(deadfruit[fruit_index], fruitposition.x, fruitposition.y, 200, rotation);
    }
    if(fruitposition.y > ScreenHeight+1000 && fruitspeed.y > 0) InitializeFruitThrow();           
}               // speed and position of fruits are updated while the fruits are in the screen

void DrawFruit(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth, float rotation){
    Vector2 center = {fruitwidth/2, fruitwidth/2};
    float temp = (fruit.width - fruit.height)/2;
    DrawTexturePro(fruit,
        (Rectangle){0 , 0, fruit.width, fruit.height}, 
        (Rectangle){TopleftX, TopleftY + temp, fruitwidth, fruitwidth - temp}, 
        center, rotation , WHITE);
    
}               // draws each frame of fruit onscreen

void UnloadFruits(){
    for (int i = 0 ; i < 20 ; i++) UnloadTexture(fruit[i]);
}           // unloads all fruits after window is closed
