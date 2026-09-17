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
#define FruitWidth 100
#define BombWidth 130
#define SplatterWidth 250
#define KnifeWidth 70
#define TrailWidth 10
#define TrailLength 10
#define STATE_MENU 0
#define STATE_GAMEPLAY 1
#define STATE_GAMEOVER 2

int currentState = STATE_MENU;

int score = 0;
int highScore = 0;
int missedFruits = 0;
bool isNewHighScore = false;
float gameOverTimer = 0;
bool playsound = true;

Texture2D *allocate_TextureMatrix(Texture2D *array, int elements);
Vector2 *allocate_VectorMatrix(Vector2 *array, int elements);
Color *allocate_ColorMatrix(Color *array, int elements);
void InitMatrices();
void LoadGame();
void LoadAssets();
void LoadSounds();
void GameState();
void DrawAsset(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth, float rotation);
void UnloadVector2(Vector2 *array, int elements);
void UnloadTexture2D(Texture2D *array, int elements);
void UnloadColor(Color *array, int elements);
void UnloadAssets();
void UnloadSounds();
void EmptyTextureMatrix(Texture2D *texture);
void EmptyVectorMatrix(Vector2 *vector);
void InitializeFruitThrow();
void UpdateFruitThrow(int sliced, float dt);
void ShowMenu();
int Background();
void PlayGame(float dt);
void CustomCursor();
void StorePreviousSplatters(int Splatter_Index, Vector2 Splatter_Position);
void PreviousSplatters();
void StoreTrails();
void DrawTrails();
void DrawScoreAndStrikes();
void GameOverScreen();
void ResetGame();

int fruit_index, sliced, storeSplatter=0, splatter_index, splatterX=0, splatterY=0, R=Red_BG, G=Green_BG, B=Blue_BG;
float rotation = 0;
Vector2 fruitposition, fruitspeed, initialfruitposition, initialfruitspeed, *PreviousSplatterPosition, *Trailpositions;
Vector2 gravity={0,2500};

Texture2D fruit[20], deadfruit[20], splatter[9], bomb[4], bombfuse[4], KNIFE, EMPTY, *PreviousSplatter;
Sound Throw[4], Slash[3], Splat[3], EXPLODE, fuse, GameOver, HIGHSCORE, MENU, PLAYFN;
Color *TrailColor;

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

    UnloadAssets();
    CloseAudioDevice();
    CloseWindow();

    return 0;
}

Texture2D *allocate_TextureMatrix(Texture2D *array, int elements){
    array = (Texture2D *)malloc(elements*sizeof(Texture2D));
    for (int i = 0 ; i < elements ; i++) array[i] = EMPTY;
    return array;
}

Vector2 *allocate_VectorMatrix(Vector2 *array, int elements){
    array = (Vector2 *)malloc(elements*sizeof(Vector2));
    for (int i = 0 ; i < elements ; i++) array[i] = (Vector2){0,0};
    return array;
}

Color *allocate_ColorMatrix(Color *array, int elements){
    array = (Color *)malloc(elements*sizeof(Color));
    for (int i = 0 ; i < elements ; i++) array[i] = BLANK;
    return array;
}

void LoadGame(){
    InitMatrices();
    InitWindow(ScreenWidth, ScreenHeight,"Fruit Ninja");
    InitAudioDevice();
    SetTargetFPS(60);
    LoadSounds();
    LoadAssets();
}

void InitMatrices(){
    PreviousSplatter = allocate_TextureMatrix(PreviousSplatter, 10);
    PreviousSplatterPosition = allocate_VectorMatrix(PreviousSplatterPosition, 10);
    Trailpositions = allocate_VectorMatrix(Trailpositions, TrailLength);
    TrailColor = allocate_ColorMatrix(TrailColor, TrailLength);
}

void LoadAssets(){
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
    GameOver = LoadSound("assets/audio/die.wav");
    HIGHSCORE = LoadSound("assets/audio/highscore.wav");
    MENU = LoadSound("assets/audio/menu.ogg");
    PLAYFN = LoadSound("assets/audio/playgame.ogg");
}

void GameState(){
    float dt=GetFrameTime();
    if (currentState == STATE_GAMEPLAY || currentState == STATE_GAMEOVER) {
            PreviousSplatters();
            DrawTrails();
        }

        switch (currentState) {
            case STATE_MENU:
                ShowMenu();
                break;
            case STATE_GAMEPLAY:
                PlayGame(dt);
                break;
            case STATE_GAMEOVER:
                GameOverScreen();
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
    DrawAsset(KNIFE, X, Y, KnifeWidth, 90);
    if (currentState == STATE_GAMEPLAY || currentState == STATE_GAMEOVER) StoreTrails();
}

void ResetGame() {
    score = 0;
    missedFruits = 0;
    isNewHighScore = false;
    InitializeFruitThrow();
}

void ShowMenu() {
    if (playsound) PlaySound(MENU);
    playsound = false;
    if (!IsSoundPlaying(MENU)) PlaySound(MENU);
    DrawText("FRUIT NINJA", ScreenWidth/2 - MeasureText("FRUIT NINJA", 70)/2, 200, 70, YELLOW);
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
        playsound = true;
        StopSound(MENU);
    }
}

void DrawScoreAndStrikes() {
    if(playsound) PlaySound(PLAYFN);
    playsound = false;
    if(!IsSoundPlaying(PLAYFN)) PlaySound(PLAYFN);

    Color scoreColor = (score > highScore && highScore > 0) ? GOLD : YELLOW;
    DrawText(TextFormat("SCORE: %d", score), ScreenWidth - 250, 30, 35, scoreColor);
    
    if (score > highScore && highScore > 0) {
        DrawText("NEW HIGH!", ScreenWidth - 250, 70, 20, ORANGE);
    }

    DrawText(TextFormat("STRIKES: %d / 3", missedFruits), 30, 30, 35, (missedFruits >= 2) ? RED : MAROON);
}

void TriggerGameOver() {
    if(IsSoundPlaying(PLAYFN)) StopSound(PLAYFN);

    if (score > highScore) {
        highScore = score;
        isNewHighScore = true;
    }
    playsound = true;
    gameOverTimer = 3;
    currentState = STATE_GAMEOVER;
}

void GameOverScreen() {
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, (Color){ 0, 0, 0, 200 });
    
    DrawText("GAME OVER", ScreenWidth/2 - MeasureText("GAME OVER", 80)/2, 250, 80, RED);
    
    if (isNewHighScore) {
        DrawText("NEW HIGHEST SCORE!!", ScreenWidth/2 - MeasureText("NEW HIGHEST SCORE!!", 40)/2, 360, 40, GOLD);
    }

    DrawText(TextFormat("FINAL SCORE: %d", score), ScreenWidth/2 - MeasureText(TextFormat("FINAL SCORE: %d", score), 35)/2, 430, 35, WHITE);


    gameOverTimer -= GetFrameTime();
    DrawText(TextFormat("Returning to menu in %d...", (int)ceil(gameOverTimer)), ScreenWidth/2 - MeasureText("Returning to menu in X...", 20)/2, 530, 20, LIGHTGRAY);


    if (playsound) {
        if(isNewHighScore) PlaySound(HIGHSCORE);
        else PlaySound(GameOver);
        playsound = false;
        }

    
    if (gameOverTimer <= 0) {
        EmptyTextureMatrix(PreviousSplatter);
        EmptyVectorMatrix(PreviousSplatterPosition);
        EmptyVectorMatrix(Trailpositions);
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
    DrawScoreAndStrikes();
    UpdateFruitThrow(sliced, dt);

    int flag=0;
    Vector2 KnifeVelocity, RelativeVelocity;

    if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) PlaySound(Slash[GetRandomValue(0,2)]);

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && !sliced){
        splatterX = GetMouseX();
        splatterY = GetMouseY(); 

        KnifeVelocity = (Vector2){GetMouseDelta().x/dt, GetMouseDelta().y/dt};
        RelativeVelocity = Vector2Subtract(KnifeVelocity, fruitspeed);

        if((GetMouseX() > fruitposition.x-100 && GetMouseX() < fruitposition.x+100 ) && ( GetMouseY() > fruitposition.y-100 && GetMouseY() < fruitposition.y+100)){
            if ((pow((RelativeVelocity.x),2) + pow((RelativeVelocity.y),2)) > 3600000 ) flag = 1;
        }

        if(flag)
        {
            if (fruit_index >= 20 && fruit_index <= 25){
                PlaySound(EXPLODE);
                StopSound(fuse);
                R = 255;
                G = 255;
                B = 255;
                score -= 30;
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
        if (sliced){
            DrawAsset(splatter[splatter_index], splatterX, splatterY, SplatterWidth, 0);
            DrawAsset(EMPTY, fruitposition.x, fruitposition.y, BombWidth, rotation);
            if(storeSplatter){
                StorePreviousSplatters(splatter_index, (Vector2){splatterX, splatterY});
                storeSplatter = 0;
            }
        }
        else if (fuse_frame == 1) DrawAsset(bomb[bomb_index], fruitposition.x, fruitposition.y, BombWidth, rotation );
        else DrawAsset(bombfuse[bomb_index], fruitposition.x, fruitposition.y, BombWidth, rotation);
    }
    else{
        if(!sliced) DrawAsset(fruit[fruit_index], fruitposition.x, fruitposition.y, FruitWidth, rotation);
        else{
            DrawAsset(splatter[splatter_index], splatterX, splatterY, SplatterWidth, 0);
            DrawAsset(deadfruit[fruit_index], fruitposition.x, fruitposition.y, FruitWidth, rotation);
            if(storeSplatter){
                StorePreviousSplatters(splatter_index, (Vector2){splatterX, splatterY});
                storeSplatter = 0;
            }
        }
    }
    if(fruitposition.y > ScreenHeight+1000 && fruitspeed.y > 0) {
        if (!sliced && fruit_index < 20) {
            missedFruits++;
            if (missedFruits >= 3) {
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
    for(int i = 9; i >= 0; i--) DrawAsset(PreviousSplatter[i], PreviousSplatterPosition[i].x, PreviousSplatterPosition[i].y, SplatterWidth, 0);
}

void StoreTrails(){
    for(int i = TrailLength-1 ; i > 0 ; i--){
        Trailpositions[i] = Trailpositions[i-1];
        TrailColor[i] = TrailColor[i-1];
    }
    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) TrailColor[0] = WHITE;
    else TrailColor[0] = BLANK;
    Trailpositions[0] = GetMousePosition();
}

void DrawTrails(){
    //for(int i = TrailLength-1; i >= 0; i--) DrawAsset(TRAIL[i], Trailpositions[i].x, Trailpositions[i].y, TrailWidth, 0);
    for(int i = TrailLength-1; i >= 1; i--) 
    {
        if(Trailpositions[i].x == 0 && Trailpositions[i].y == 0) continue;
        if(i > (TrailLength-3) || i < 3) DrawLineEx(Trailpositions[i], Trailpositions[i-1], TrailWidth/3, TrailColor[i]);
        else if((i <= (TrailLength-3) && i > (TrailLength-5)) || (i >= 3 && i < 5)) DrawLineEx(Trailpositions[i], Trailpositions[i-1], 2*TrailWidth/3, TrailColor[i]);
        else DrawLineEx(Trailpositions[i], Trailpositions[i-1], TrailWidth, TrailColor[i]);
    }
}

void DrawAsset(Texture2D fruit, float TopleftX, float TopleftY, float fruitwidth, float rotation){
    Vector2 center = {fruitwidth/2, fruitwidth/2};
    float temp = (fruit.width - fruit.height)/2;
    DrawTexturePro(fruit,
        (Rectangle){0 , 0, fruit.width, fruit.height}, 
        (Rectangle){TopleftX, TopleftY + temp, fruitwidth, fruitwidth - temp}, 
        center, rotation, WHITE);
}
                   // draws each asset with proper (square) sizing and not extending the assets

void UnloadAssets(){
    UnloadTexture2D(fruit, 20);
    UnloadTexture2D(deadfruit, 20);
    UnloadTexture2D(splatter, 10);
    UnloadTexture2D(bomb, 4);
    UnloadTexture2D(bombfuse, 4);
    UnloadTexture2D(PreviousSplatter, 10);
    UnloadTexture(KNIFE);
    UnloadTexture(EMPTY);

    UnloadSounds();
    free(PreviousSplatterPosition);
    free(PreviousSplatter);
    free(Trailpositions);
    free(TrailColor);
}           // unloads all stuffs after window is closed

void UnloadSounds(){
    for(int i = 0 ; i < 3 ; i++) {
        UnloadSound(Throw[i]);
        UnloadSound(Splat[i]);
        UnloadSound(Slash[i]);
    }
    UnloadSound(EXPLODE);
    UnloadSound(fuse);
    UnloadSound(GameOver);
    UnloadSound(HIGHSCORE);
    UnloadSound(MENU);
    UnloadSound(PLAYFN);
}

void UnloadTexture2D(Texture2D *array, int elements){
    for (int i=0; i<elements; i++) UnloadTexture (array[i]);
}


void EmptyTextureMatrix(Texture2D *texture){
    for (int i = 0 ; i < 10 ; i++)
    texture[i] = EMPTY;
}

void EmptyVectorMatrix(Vector2 *vector){
    for (int i = 0 ; i < 10 ; i++)
    vector[i] = (Vector2){0, 0};
}