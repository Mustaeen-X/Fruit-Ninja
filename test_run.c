#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define DEBUG 0
#define ScreenWidth 1450
#define ScreenHeight 1000
#define Red_BG 97
#define Green_BG 50
#define Blue_BG 24
#define FruitWidth 150
#define BombWidth 200
#define SplatterWidth 500

typedef enum GameStatus {
    STATE_MENU,
    STATE_GAMEPLAY,
    STATE_GAMEOVER
} GameStatus;

GameStatus currentState = STATE_MENU;

int score = 0;
int highScore = 0;
int missedFruits = 0;
bool isNewHighScore = false;
float gameOverTimer = 0;

Texture2D *allocate_TextureMatrix(Texture2D *array, int elements);
Vector2 *allocate_VectorMatrix(Vector2 *array, int elements);
void InitSplatterMatrices();
void ClearSplatterMatrices();
void LoadGame();
void LoadFruits();
void LoadSounds();
void GameState();
void DrawFruit(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth, float rotation);
void UnloadFruits();
void InitializeFruitThrow();
void UpdateFruitThrow(int sliced, float dt);
void ShowMenu();
int Background();
void PlayGame(float dt);
void CustomCursor();
void StorePreviousSplatters(int Splatter_Index, Vector2 Splatter_Position);
void PreviousSplatters();
void DrawScoreAndStrikes();
void DrawGameOverScreen();
void ResetGame();

int fruit_index, sliced, storeSplatter=0, splatter_index, splatterX=0, splatterY=0, R=Red_BG, G=Green_BG, B=Blue_BG;
float rotation = 0;
Vector2 fruitposition, fruitspeed, initialfruitposition, initialfruitspeed, *PreviousSplatterPosition;
Vector2 gravity={0,2500};

Texture2D fruit[20], deadfruit[20], splatter[9], bomb[4], bombfuse[4], KNIFE, EMPTY, *PreviousSplatter;
Sound Throw[4], Slash[3], Splat[3], EXPLODE, fuse;

int main(){
    LoadGame();
    while(!WindowShouldClose()){

        float dt = GetFrameTime();
        BeginDrawing();
        Background();

        GameState();
            
        CustomCursor();
        EndDrawing(); 
    }

    UnloadFruits();

    CloseAudioDevice();
    CloseWindow();

    return 0;
}

Texture2D *allocate_TextureMatrix(Texture2D *array, int elements){
    array = (Texture2D *)malloc(elements*sizeof(Texture2D));
    return array;
}

Vector2 *allocate_VectorMatrix(Vector2 *array, int elements){
    array = (Vector2 *)malloc(elements*sizeof(Vector2));
    return array;
}

void LoadGame(){
    InitSplatterMatrices();
    InitWindow(ScreenWidth, ScreenHeight,"Fruit Ninja");
    InitAudioDevice();
    SetTargetFPS(60);
    LoadSounds();
    LoadFruits();
    InitializeFruitThrow();
}

void InitSplatterMatrices(){
    PreviousSplatter = allocate_TextureMatrix(PreviousSplatter, 10);
    PreviousSplatterPosition = allocate_VectorMatrix(PreviousSplatterPosition, 10);
    ClearSplatterMatrices();
}

void ClearSplatterMatrices(){
    for(int i = 0; i < 10; i++) {
        PreviousSplatter[i].id = 0;
        PreviousSplatterPosition[i] = (Vector2){ 0, 0 };
    }
}

void LoadFruits(){
    char path[150];
    Image temp;
    int maxdim;
    
    temp = LoadImage("assets/KNIFE.png");
    maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    KNIFE = LoadTextureFromImage(temp);
    UnloadImage(temp);

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

    for( int i = 0 ; i < 9 ; i++ )
    {
        sprintf(path, "assets/sprites/Splatter/%d.png", i+1);
        temp = LoadImage(path);
        maxdim = temp.width >= temp.height? temp.width : temp.height;
        ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
        splatter[i] = LoadTextureFromImage(temp);
        UnloadImage(temp);
    }

    sprintf(path, "assets/sprites/fruits/Empty.png");
    temp = LoadImage(path);
    maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    EMPTY = LoadTextureFromImage(temp);
    UnloadImage(temp);

    for( int i = 0 ; i < 4 ; i++ )
    {
        sprintf(path, "assets/sprites/Bomb/Bomb%d.png", i+1);
        temp = LoadImage(path);
        maxdim = temp.width >= temp.height? temp.width : temp.height;
        ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
        bomb[i] = LoadTextureFromImage(temp);

        sprintf(path, "assets/sprites/Bomb/Bomb%da.png", i+1);
        temp = LoadImage(path);
        maxdim = temp.width >= temp.height? temp.width : temp.height;
        ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
        bombfuse[i] = LoadTextureFromImage(temp);
        UnloadImage(temp);
    }
    }                   // loaded 20 fruit assets, 20 dead fruit assets, 8 splatter effects, with KNIFE and an empty image

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
    EXPLODE = LoadSound("assets/audio/explosion.mp3");
    fuse = LoadSound("assets/audio/bombfuse.mp3");
}

void GameState(){
    float dt=GetFrameTime();
    if (currentState == STATE_GAMEPLAY || currentState == STATE_GAMEOVER) {
            PreviousSplatters();
        }

        switch (currentState) {
            case STATE_MENU:
                ShowMenu();
                break;
            case STATE_GAMEPLAY:
                PlayGame(dt);
                DrawScoreAndStrikes();
                break;
            case STATE_GAMEOVER:
                DrawGameOverScreen();
                break;
        }
}

int Background(){
    ClearBackground((Color){R, G, B, 255});
    R = R > Red_BG ? R-6 : Red_BG;
    G = G > Green_BG ? G-7 : Green_BG;
    B = B > Blue_BG ? B-7 : Blue_BG;
    if (R == Red_BG && G == Green_BG && B == Blue_BG) return 1;
    else return 0;
}

void CustomCursor(){
    int X = GetMouseX(), Y = GetMouseY();
    HideCursor();
    DrawFruit(KNIFE, X, Y, 100, 0);
}

void ResetGame() {
    score = 0;
    missedFruits = 0;
    isNewHighScore = false;
    ClearSplatterMatrices();
    InitializeFruitThrow();
}

void ShowMenu() {
    DrawText("FRUIT NINJA", ScreenWidth/2 - MeasureText("FRUIT NINJA", 70)/2, 200, 70, DARKGRAY);
    char hightext[50];
    sprintf(hightext, "HIGH SCORE: %d", highScore);
    DrawText(hightext, ScreenWidth/2 - MeasureText(hightext, 30)/2, 320, 30, GRAY);

    Rectangle startButton = {ScreenWidth/2 - 120, 450, 240, 70};
    Vector2 mousePoint = GetMousePosition();
    bool hovering = CheckCollisionPointRec(mousePoint, startButton);

    DrawRectangleRec(startButton, hovering ? DARKGREEN : GREEN);
    DrawRectangleLinesEx(startButton, 5, DARKGRAY);
    DrawText("PLAY", startButton.x + startButton.width/2 - MeasureText("PLAY", 35)/2, startButton.y + 18, 35, WHITE);

    if (hovering && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ResetGame();
        currentState = STATE_GAMEPLAY;
    }
}

void DrawScoreAndStrikes() {
    Color scoreColor = (score > highScore && highScore > 0) ? GOLD : DARKGRAY;
    DrawText(TextFormat("SCORE: %d", score), ScreenWidth - 250, 30, 35, scoreColor);
    
    if (score > highScore && highScore > 0) {
        DrawText("NEW HIGH!", ScreenWidth - 250, 70, 20, ORANGE);
    }

    DrawText(TextFormat("STRIKES: %d / 3", missedFruits), 30, 30, 35, (missedFruits >= 2) ? RED : MAROON);
}

void TriggerGameOver() {
    if (score > highScore) {
        highScore = score;
        isNewHighScore = true;
    }
    gameOverTimer = 3;
    currentState = STATE_GAMEOVER;
}

void DrawGameOverScreen() {
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, (Color){ 0, 0, 0, 200 });
    
    DrawText("GAME OVER", ScreenWidth/2 - MeasureText("GAME OVER", 80)/2, 250, 80, RED);
    
    if (isNewHighScore) {
        DrawText("NEW HIGHEST SCORE!!", ScreenWidth/2 - MeasureText("NEW HIGHEST SCORE!!", 40)/2, 360, 40, GOLD);
    }

    DrawText(TextFormat("FINAL SCORE: %d", score), ScreenWidth/2 - MeasureText(TextFormat("FINAL SCORE: %d", score), 35)/2, 430, 35, WHITE);

    gameOverTimer -= GetFrameTime();
    DrawText(TextFormat("Returning to menu in %d...", (int)ceil(gameOverTimer)), ScreenWidth/2 - MeasureText("Returning to menu in X...", 20)/2, 530, 20, LIGHTGRAY);

    if (gameOverTimer <= 0) {
        ClearSplatterMatrices();
        currentState = STATE_MENU;
    }
}

void InitializeFruitThrow(){
    int fruitrise = GetRandomValue(100,ScreenWidth);
    int fruitfall = GetRandomValue(100,ScreenWidth);
    float fruitHspeed = (float)(fruitfall - fruitrise)/5;
    int fruitheight = GetRandomValue((int)(ScreenHeight/3), ScreenHeight-100);
    int fruitVspeed = -sqrt(2*gravity.y*fruitheight);
    initialfruitposition = (Vector2){fruitrise, ScreenHeight};
    initialfruitspeed = (Vector2){fruitHspeed, fruitVspeed};
    fruit_index = GetRandomValue (0,25);
    if (fruit_index == 0 || fruit_index == 6 || fruit_index == 7 || fruit_index == 8) splatter_index = 0;
    else if (fruit_index == 10) splatter_index = 1;
    else if (fruit_index == 4 || fruit_index == 14) splatter_index = 2;
    else if (fruit_index == 2 || fruit_index == 11 || fruit_index == 15 || fruit_index == 13) splatter_index = 3;
    else if (fruit_index == 1 || fruit_index == 3 || fruit_index == 5 || fruit_index == 9) splatter_index = 4;
    else if (fruit_index == 17 || fruit_index == 18 || fruit_index == 12) splatter_index = 5;
    else if (fruit_index == 16) splatter_index = 6;
    else if (fruit_index == 19) splatter_index = 7;
    else splatter_index = 8;
    fruitposition = initialfruitposition;
    fruitspeed = initialfruitspeed;
    if (fruit_index >= 20 && fruit_index <= 25) PlaySound(fuse);
    else PlaySound(Throw[GetRandomValue(0,2)]);
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
            if (fruit_index >= 20 && fruit_index <= 25){
                PlaySound(EXPLODE);
                StopSound(fuse);
                R = 255;
                G = 255;
                B = 255;
                score -= 10;
                if (score < 0) {
                    TriggerGameOver();
                }
            }
            else {
                PlaySound(Splat[GetRandomValue(0,2)]);
                score += 10;
            }
            sliced = 1;
            storeSplatter = 1;
        }
    }  
}

void UpdateFruitThrow(int sliced, float dt){
    rotation += 70*dt;
    fruitspeed = Vector2Add(fruitspeed, Vector2Scale(gravity,dt));
    fruitposition = Vector2Add(fruitposition, Vector2Scale(fruitspeed,dt));
    if(fruit_index >= 20 && fruit_index <= 25){
        int bomb_index, fuse_frame;
        bomb_index = (int)(GetTime() / 0.05) % 4;
        fuse_frame = (int)(GetTime() / 0.5) % 2;
        if (sliced) {
            DrawFruit(splatter[splatter_index], splatterX, splatterY, SplatterWidth, 0);
            DrawFruit(EMPTY, fruitposition.x, fruitposition.y, BombWidth, rotation);
            if(storeSplatter){
                StorePreviousSplatters(splatter_index, (Vector2){splatterX, splatterY});
                storeSplatter = 0;
            }
        }
        else if (fuse_frame == 1) DrawFruit(bomb[bomb_index], fruitposition.x, fruitposition.y, BombWidth, rotation );
        else DrawFruit(bombfuse[bomb_index], fruitposition.x, fruitposition.y, BombWidth, rotation);
    }
    else{
        if(!sliced) DrawFruit(fruit[fruit_index], fruitposition.x, fruitposition.y, FruitWidth, rotation);
        else{
            DrawFruit(splatter[splatter_index], splatterX, splatterY, SplatterWidth, 0);
            DrawFruit(deadfruit[fruit_index], fruitposition.x, fruitposition.y, FruitWidth, rotation);
            if(storeSplatter){
                StorePreviousSplatters(splatter_index, (Vector2){splatterX, splatterY});
                storeSplatter = 0;
            }
        }
    }
    if(fruitposition.y > ScreenHeight+1000 && fruitspeed.y > 0) {
        if (!sliced && fruit_index < 20) {
            missedFruits++;
            if (missedFruits > 3) {
                TriggerGameOver();
                return;
            }
        }
        InitializeFruitThrow();
    } 
    
}               // speed and position of fruits are updated while the fruits are in the screen

void StorePreviousSplatters(int Splatter_Index, Vector2 Splatter_Position){
    for(int i = 9 ; i > 0 ; i--){
        PreviousSplatter[i] = PreviousSplatter[i-1];
        PreviousSplatterPosition[i] = PreviousSplatterPosition[i-1];
    }
    PreviousSplatter[0] = splatter[splatter_index];
    PreviousSplatterPosition[0] = Splatter_Position;
}

void PreviousSplatters(){
    for(int i = 9; i >= 0; i--){
        if (PreviousSplatter[i].id != 0) {
            DrawFruit(PreviousSplatter[i], PreviousSplatterPosition[i].x, PreviousSplatterPosition[i].y, SplatterWidth, 0);
        }
    }
}

void DrawFruit(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth, float rotation){
    Vector2 center = {fruitwidth/2, fruitwidth/2};
    float temp = (fruit.width - fruit.height)/2;
    DrawTexturePro(fruit,
        (Rectangle){0 , 0, fruit.width, fruit.height}, 
        (Rectangle){TopleftX, TopleftY + temp, fruitwidth, fruitwidth - temp}, 
        center, rotation , WHITE);
    
}               // draws each frame of fruit onscreen

void UnloadFruits(){
    for (int i = 0 ; i < 20 ; i++) {
        UnloadTexture(fruit[i]);
        UnloadTexture(deadfruit[i]);
    }
    for (int i = 0 ; i < 9 ; i++) UnloadTexture(splatter[i]) ;
    for (int i = 0 ; i < 4 ; i++){
        UnloadTexture(bomb[i]);
        UnloadTexture(bombfuse[i]);
    }
    UnloadTexture(KNIFE);
    UnloadTexture(EMPTY);
}           // unloads all fruits after window is closed

void UnloadSounds(){

    //
}