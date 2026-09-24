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
#define BombWidth 160
#define SplatterWidth 250
#define maxSplatters 15
#define KnifeWidth 70
#define TrailWidth 10
#define TrailLength 10
#define STATE_MENU 0
#define STATE_GAMEPLAY 1
#define STATE_GAMEOVER 2
#define strikes 5
#define maxfruits 10

typedef struct {
    Vector2 position;
    Vector2 speed;
    int fruit_index;
    int sliced;
    float rotation;
    int splatter_index;
    float splatterX;
    float splatterY;
    int storeSplatter;
    int isitonscreen;
} FruitThrow; //each fruit's data is independent now

FruitThrow thrownfruit[maxfruits];

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
void DrawAsset(Texture2D fruit, float CentreX, float CentreY, float fruitwidth, float rotation);
void UnloadTexture2D(Texture2D *array, int elements);
void UnloadAssets();
void UnloadSounds();
void EmptyTextureMatrix(Texture2D *texture, int elements);
void EmptyVectorMatrix(Vector2 *vector, int elements);
void InitializeFruitThrow(int fruit_number);
void UpdateFruitThrow();
int FruitCount(int maxfruitno);
void ShowMenu();
int Background();
void PlayGame();
void CustomCursor();
void StorePreviousSplatters(int Splatter_Index, Vector2 Splatter_Position);
void PreviousSplatters();
void StoreTrails();
void DrawTrails();
void DrawScoreAndStrikes();
void GameOverScreen();
void ResetGame();

int R=Red_BG, G=Green_BG, B=Blue_BG, fruit_throw_count, wave=0, MusicVolume=1, SFXVolume=1, PAUSE=0, Cursor=1, Close=0;
Vector2 *PreviousSplatterPosition, *Trailpositions;
Vector2 gravity={0,2500};

Texture2D fruit[20], deadfruit[20], splatter[9], bomb[4], bombfuse[4], KNIFE, EMPTY, MusicIcon, *PreviousSplatter;
Sound Throw[4], Slash[3], Splat[3], EXPLODE, fuse, GameOver, HIGHSCORE, MENU, PLAYFN;
Color *TrailColor;

int main(){
    LoadGame();
    while(!WindowShouldClose() && !Close){

        float dt = GetFrameTime();
        BeginDrawing();
        Background();

        GameState();

        CustomCursor();
        if (currentState == STATE_GAMEPLAY || currentState == STATE_GAMEOVER) StoreTrails();
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
    PreviousSplatter = allocate_TextureMatrix(PreviousSplatter, maxSplatters);
    PreviousSplatterPosition = allocate_VectorMatrix(PreviousSplatterPosition, maxSplatters);
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

    temp = LoadImage("assets/MusicIcon.png");
    maxdim = temp.width >= temp.height? temp.width : temp.height;
    ImageResizeCanvas(&temp, maxdim, maxdim, (maxdim - temp.width) / 2 , (maxdim - temp.height) / 2, BLANK);
    MusicIcon = LoadTextureFromImage(temp);
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
    float dt = GetFrameTime();

    if(IsKeyPressed(KEY_P)) PAUSE = !PAUSE;
    Rectangle pauseBox = {ScreenWidth/2 - 120, ScreenHeight/2 - 35, 250, 100};
    if(PAUSE) if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), pauseBox)) PAUSE = !PAUSE;
    SetMasterVolume(1 - PAUSE*.7);

    if (currentState == STATE_GAMEPLAY || currentState == STATE_GAMEOVER) {
        PreviousSplatters();
        DrawTrails();
    }

    if (!PAUSE) {
        switch (currentState) {
            case STATE_MENU:
                ShowMenu();
                break;
            case STATE_GAMEPLAY:
                PlayGame();
                break;
            case STATE_GAMEOVER:
                GameOverScreen();
                break;
        }
    } else {
        bool hoveringPAUSE = CheckCollisionPointRec(GetMousePosition(), pauseBox);
        DrawRectangle(0, 0, ScreenWidth, ScreenHeight, (Color){ 0, 0, 0, 150 });
        DrawRectangleRec(pauseBox, hoveringPAUSE? (Color){0, 15, 120, 255} : (Color){0, 26, 140, 255});
        DrawRectangleLinesEx(pauseBox, 5, (Color){0, 17, 92, 255});
        DrawText("PAUSED", pauseBox.x + pauseBox.width/2 - MeasureText("PAUSED", 35)/2, pauseBox.y + 35, 35, YELLOW);
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
    if(!Cursor) ShowCursor();
    else{
        HideCursor();
        DrawAsset(KNIFE, X+KnifeWidth/2, Y+KnifeWidth/2, KnifeWidth, 90);
    }
}

void ResetGame() {
    score = 0;
    missedFruits = 0;
    isNewHighScore = false;
    wave = 1;
    for (int i = 0; i < maxfruits; i++) thrownfruit[i].isitonscreen = 0;
    InitializeFruitThrow(0);
}

void ShowMenu() {
    if(MusicVolume) if(playsound) PlaySound(MENU);
    playsound = false;
    if(MusicVolume) if(!IsSoundPlaying(MENU)) PlaySound(MENU);
    DrawText("FRUIT NINJA", ScreenWidth/2 - MeasureText("FRUIT NINJA", 70)/2, 200, 70, YELLOW);
    char hightext[50];
    sprintf(hightext, "HIGH SCORE: %d", highScore);
    DrawText(hightext, ScreenWidth/2 - MeasureText(hightext, 30)/2, 320, 30, GRAY);

    Rectangle PlayButton = {ScreenWidth/2 - 120, 450, 240, 70};
    Rectangle SFXButton = {ScreenWidth - 240, ScreenHeight - 120, 70, 70};
    Rectangle MusicButton = {ScreenWidth - 120, ScreenHeight - 120, 70, 70};
    Rectangle CursorButton = {ScreenWidth - 360, ScreenHeight - 120, 70, 70};
    Rectangle QuitButton = {ScreenWidth/2 - 120, 570, 240, 70};

    bool hoveringPLAY = CheckCollisionPointRec(GetMousePosition(), PlayButton);
    bool hoveringTMUSIC = CheckCollisionPointRec(GetMousePosition(), MusicButton);
    bool hoveringSFX = CheckCollisionPointRec(GetMousePosition(), SFXButton);
    bool hoveringCursor = CheckCollisionPointRec(GetMousePosition(), CursorButton);
    bool hoveringQuit = CheckCollisionPointRec(GetMousePosition(), QuitButton);

    DrawRectangleRec(PlayButton, hoveringPLAY ? DARKGREEN : GREEN);
    DrawRectangleLinesEx(PlayButton, 5, DARKGRAY);
    DrawText("PLAY", PlayButton.x + PlayButton.width/2 - MeasureText("PLAY", 35)/2, PlayButton.y + 18, 35, WHITE);
    if (hoveringPLAY && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        ResetGame();
        currentState = STATE_GAMEPLAY;
        playsound = true;
        StopSound(MENU);
    }

    DrawRectangleRec(QuitButton, hoveringQuit ? (Color){163, 64, 162, 255} : (Color){255, 100, 252, 255});
    DrawRectangleLinesEx(QuitButton, 5, (Color){200, 60, 195, 255});
    DrawText("QUIT GAME", QuitButton.x + QuitButton.width/2 - MeasureText("QUIT GAME", 35)/2, QuitButton.y + 18, 35, WHITE);
    if (hoveringQuit && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) Close = !Close;

    DrawRectangleRec(MusicButton, hoveringTMUSIC? (Color){180, 180, 180, 255} : (Color){220, 220, 220, 255});
    DrawRectangleLinesEx(MusicButton, 5, (Color){100, 100, 100, 255});
    if(hoveringTMUSIC && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) MusicVolume = !MusicVolume;
    DrawAsset(MusicIcon, MusicButton.x+MusicButton.width/2, MusicButton.y+MusicButton.height/2, MusicButton.width-20, 0);
    if (!MusicVolume) {
    Vector2 startPos = { MusicButton.x - 2, MusicButton.y - 2 };
    Vector2 endPos   = { MusicButton.x + MusicButton.width + 2, MusicButton.y + MusicButton.height + 2 };
    DrawLineEx(startPos, endPos, 7, MAROON);
    if(IsSoundPlaying(MENU)) StopSound(MENU);
    }

    DrawRectangleRec(SFXButton, hoveringSFX? (Color){180, 180, 180, 255} : (Color){220, 220, 220, 255});
    DrawRectangleLinesEx(SFXButton, 5, (Color){100, 100, 100, 255});
    DrawText("SFX", SFXButton.x + 10, SFXButton.y + 25, 25, (Color){100,100,100,255});
    if(hoveringSFX && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) SFXVolume = !SFXVolume;
    if (!SFXVolume) {
    Vector2 startPos = { SFXButton.x - 2, SFXButton.y - 2 };
    Vector2 endPos   = { SFXButton.x + SFXButton.width + 2, SFXButton.y + SFXButton.height + 2 };
    DrawLineEx(startPos, endPos, 7, MAROON);
    }

    DrawRectangleRec(CursorButton, hoveringCursor? (Color){180, 180, 180, 255} : (Color){220, 220, 220, 255});
    DrawRectangleLinesEx(CursorButton, 5, (Color){100, 100, 100, 255});
    DrawAsset(KNIFE, CursorButton.x+CursorButton.width/2, CursorButton.y+CursorButton.height/2, CursorButton.width-20, 90);
    if(hoveringCursor && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) Cursor = !Cursor;
    if (!Cursor) {
    Vector2 startPos = { CursorButton.x - 2, CursorButton.y - 2 };
    Vector2 endPos   = { CursorButton.x + CursorButton.width + 2, CursorButton.y + CursorButton.height + 2 };
    DrawLineEx(startPos, endPos, 7, MAROON);
    }
}

void DrawScoreAndStrikes() {
    if(MusicVolume) if(playsound) PlaySound(PLAYFN);
    playsound = false;
    if(MusicVolume) if(!IsSoundPlaying(PLAYFN)) PlaySound(PLAYFN);

    Color scoreColor = (score > highScore && highScore > 0) ? GOLD : YELLOW;
    DrawText(TextFormat("SCORE: %d", score), ScreenWidth - 250, 30, 35, scoreColor);
    
    if (score > highScore && highScore > 0) {
        DrawText("NEW HIGH!", ScreenWidth - 250, 70, 20, ORANGE);
    }

    DrawText(TextFormat("STRIKES: %d / %d", missedFruits, strikes), 30, 30, 50, (missedFruits >= 2) ? RED : MAROON);

    #if DEBUG
    char wavenumber[100];
    sprintf(wavenumber, "Wave Number: %d", wave);
    DrawText(wavenumber, 30, 100, 35, GRAY);
    #endif
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
        if(SFXVolume) if(isNewHighScore) PlaySound(HIGHSCORE);
        else if(SFXVolume) PlaySound(GameOver);
        playsound = false;
        }

    
    if (gameOverTimer <= 0) {
        EmptyTextureMatrix(PreviousSplatter, maxSplatters);
        EmptyVectorMatrix(PreviousSplatterPosition, maxSplatters);
        EmptyVectorMatrix(Trailpositions, TrailLength);
        wave = 0;
        currentState = STATE_MENU;
    }
}

void InitializeFruitThrow(int fruit_number){
    int fruitrise = GetRandomValue(100,ScreenWidth);
    int fruitfall = GetRandomValue(100,ScreenWidth);
    float fruitHspeed = (float)(fruitfall - fruitrise)/5;
    int fruitheight = GetRandomValue((int)(ScreenHeight/3), ScreenHeight-100);
    float fruitVspeed = -sqrt(2*gravity.y*fruitheight);

    thrownfruit[fruit_number].isitonscreen = 1;
    thrownfruit[fruit_number].position = (Vector2){fruitrise, ScreenHeight};
    thrownfruit[fruit_number].speed = (Vector2){fruitHspeed, fruitVspeed};
    if(wave < 5) thrownfruit[fruit_number].fruit_index = GetRandomValue(0,19);
    else if(wave == 5) thrownfruit[fruit_number].fruit_index = 20; 
    else if(wave == 6){
        if(fruit_number == 0) thrownfruit[fruit_number].fruit_index = 20;
        else thrownfruit[fruit_number].fruit_index = GetRandomValue(0,19);
    }
    else thrownfruit[fruit_number].fruit_index = GetRandomValue(0,25);

    if (thrownfruit[fruit_number].fruit_index == 0 || thrownfruit[fruit_number].fruit_index == 6 || thrownfruit[fruit_number].fruit_index == 7 || thrownfruit[fruit_number].fruit_index == 8) thrownfruit[fruit_number].splatter_index = 0;
    else if (thrownfruit[fruit_number].fruit_index == 10) thrownfruit[fruit_number].splatter_index = 1;
    else if (thrownfruit[fruit_number].fruit_index == 4 || thrownfruit[fruit_number].fruit_index == 14) thrownfruit[fruit_number].splatter_index = 2;
    else if (thrownfruit[fruit_number].fruit_index == 2 || thrownfruit[fruit_number].fruit_index == 11 || thrownfruit[fruit_number].fruit_index == 15 || thrownfruit[fruit_number].fruit_index == 13) thrownfruit[fruit_number].splatter_index = 3;
    else if (thrownfruit[fruit_number].fruit_index == 1 || thrownfruit[fruit_number].fruit_index == 3 || thrownfruit[fruit_number].fruit_index == 5 || thrownfruit[fruit_number].fruit_index == 9) thrownfruit[fruit_number].splatter_index = 4;
    else if (thrownfruit[fruit_number].fruit_index == 17 || thrownfruit[fruit_number].fruit_index == 18 || thrownfruit[fruit_number].fruit_index == 12) thrownfruit[fruit_number].splatter_index = 5;
    else if (thrownfruit[fruit_number].fruit_index == 16) thrownfruit[fruit_number].splatter_index = 6;
    else if (thrownfruit[fruit_number].fruit_index == 19) thrownfruit[fruit_number].splatter_index = 7;
    else thrownfruit[fruit_number].splatter_index = 8;

    if(SFXVolume) if (thrownfruit[fruit_number].fruit_index >= 20 && thrownfruit[fruit_number].fruit_index <= 25) PlaySound(fuse);
    else {
    if(SFXVolume) PlaySound(Throw[GetRandomValue(0,2)]);
    }
    thrownfruit[fruit_number].sliced = 0;
    thrownfruit[fruit_number].rotation = 0;
    thrownfruit[fruit_number].storeSplatter = 0;
}                   // before each throw, the position and speed parameters are initialized, with the throwing audio and splatter color

void PlayGame(){
    float dt = GetFrameTime();
    DrawScoreAndStrikes();
    UpdateFruitThrow();

    int flag=0;
    Vector2 KnifeVelocity, RelativeVelocity;

    if(SFXVolume) if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) PlaySound(Slash[GetRandomValue(0,2)]);

    if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && (GetMouseX() > 0) && GetMouseX() < ScreenWidth && GetMouseY() > 0 && GetMouseY() < ScreenHeight){
        KnifeVelocity = (Vector2){GetMouseDelta().x/dt, GetMouseDelta().y/dt};
        for(int i=0; i<maxfruits; i++){
            if (thrownfruit[i].isitonscreen && !thrownfruit[i].sliced){

        #if DEBUG
        DrawAsset(KNIFE, thrownfruit[i].position.x, thrownfruit[i].position.y+FruitWidth/2, 40, 0);
        DrawAsset(KNIFE, thrownfruit[i].position.x, thrownfruit[i].position.y-FruitWidth/2, 40, 0);
        DrawAsset(KNIFE, thrownfruit[i].position.x+FruitWidth/2, thrownfruit[i].position.y, 40, 0);
        DrawAsset(KNIFE, thrownfruit[i].position.x-FruitWidth/2, thrownfruit[i].position.y, 40, 0);
        DrawCircleLinesV(thrownfruit[i].position, FruitWidth/2, GRAY);
        #endif

        RelativeVelocity = Vector2Subtract(KnifeVelocity, thrownfruit[i].speed);

        if(CheckCollisionCircleLine(thrownfruit[i].position, FruitWidth/2, Trailpositions[0], Trailpositions[1]) && ((pow((RelativeVelocity.x),2) + pow((RelativeVelocity.y),2)) > 300000)){
            if (thrownfruit[i].fruit_index >= 20 && thrownfruit[i].fruit_index <= 25){
                if(SFXVolume) PlaySound(EXPLODE);
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
                if(SFXVolume) PlaySound(Splat[GetRandomValue(0,2)]);
                score += 10;
            }
            thrownfruit[i].sliced = 1;
            thrownfruit[i].storeSplatter = 1;
            thrownfruit[i].splatterX = thrownfruit[i].position.x;
            thrownfruit[i].splatterY = thrownfruit[i].position.y;
        }
    }
    }
    }  
}

void UpdateFruitThrow(){
    float dt = GetFrameTime();
    int throwNewFruits = 0;
    int fruitsOnScreen = 0;

    for(int i=0; i<maxfruits; i++){
    if (thrownfruit[i].isitonscreen == 0) continue;
    fruitsOnScreen++;

    thrownfruit[i].rotation += 70*dt;
    thrownfruit[i].speed = Vector2Add(thrownfruit[i].speed, Vector2Scale(gravity,dt));
    thrownfruit[i].position = Vector2Add(thrownfruit[i].position, Vector2Scale(thrownfruit[i].speed,dt));

    if(thrownfruit[i].fruit_index >= 20 && thrownfruit[i].fruit_index <= 25){
        int bomb_index, fuse_frame;
        bomb_index = (int)(GetTime() / 0.05) % 4;
        fuse_frame = (int)(GetTime() / 0.5) % 2;
        if (thrownfruit[i].sliced){
            DrawAsset(splatter[thrownfruit[i].splatter_index], thrownfruit[i].splatterX, thrownfruit[i].splatterY, SplatterWidth, 0);
            DrawAsset(EMPTY, thrownfruit[i].position.x, thrownfruit[i].position.y, BombWidth, thrownfruit[i].rotation);
            if(thrownfruit[i].storeSplatter){
                StorePreviousSplatters(thrownfruit[i].splatter_index, (Vector2){thrownfruit[i].splatterX, thrownfruit[i].splatterY});
                thrownfruit[i].storeSplatter = 0;
            }
        }
        else if (fuse_frame == 1) DrawAsset(bomb[bomb_index], thrownfruit[i].position.x, thrownfruit[i].position.y, BombWidth, thrownfruit[i].rotation );
        else DrawAsset(bombfuse[bomb_index], thrownfruit[i].position.x, thrownfruit[i].position.y, BombWidth, thrownfruit[i].rotation);
    }
    else{
        if(!thrownfruit[i].sliced) DrawAsset(fruit[thrownfruit[i].fruit_index], thrownfruit[i].position.x, thrownfruit[i].position.y, FruitWidth, thrownfruit[i].rotation);
        else{
            DrawAsset(splatter[thrownfruit[i].splatter_index], thrownfruit[i].splatterX, thrownfruit[i].splatterY, SplatterWidth, 0);
            DrawAsset(deadfruit[thrownfruit[i].fruit_index], thrownfruit[i].position.x, thrownfruit[i].position.y, FruitWidth, thrownfruit[i].rotation);
            if(thrownfruit[i].storeSplatter){
                StorePreviousSplatters(thrownfruit[i].splatter_index, (Vector2){thrownfruit[i].splatterX, thrownfruit[i].splatterY});
                thrownfruit[i].storeSplatter = 0;
            }
        }
    }
    if(thrownfruit[i].position.y > ScreenHeight+1000 && thrownfruit[i].speed.y > 0) {
        if (!thrownfruit[i].sliced && thrownfruit[i].fruit_index < 20) {
            missedFruits++;
            if (missedFruits >= strikes) {
                TriggerGameOver();
                return;
            }
        }
        thrownfruit[i].isitonscreen = 0;
        
        
    } 
    }
    if(fruitsOnScreen == 0){
        wave++;
        fruit_throw_count = FruitCount(maxfruits);
        for(int i = 0; i < fruit_throw_count; i++) InitializeFruitThrow(i);
    }
}               // speed and position of fruits are updated while the fruits are in the screen

int FruitCount(int maxfruitno){
    int out;
    if(wave < 6) out = maxfruitno/10;
        else if(wave < 9) out = 2*maxfruitno/10;
        else if(wave < 16) out = 3*maxfruitno/10;
        else if(wave < 50) out = (wave/5)*maxfruitno/10;
        else out = GetRandomValue(1, maxfruitno);
        return out;
}

void StorePreviousSplatters(int Splatter_Index, Vector2 Splatter_Position){
    for(int i = maxSplatters-1 ; i > 0 ; i--){
        PreviousSplatter[i] = PreviousSplatter[i-1];
        PreviousSplatterPosition[i] = PreviousSplatterPosition[i-1];
    }
    PreviousSplatter[0] = splatter[Splatter_Index];
    PreviousSplatterPosition[0] = Splatter_Position;
}

void PreviousSplatters(){
    for(int i = maxSplatters-1 ; i >= 0; i--) DrawAsset(PreviousSplatter[i], PreviousSplatterPosition[i].x, PreviousSplatterPosition[i].y, SplatterWidth, 0);
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

void DrawAsset(Texture2D fruit, float CentreX, float CentreY, float fruitwidth, float rotation){
    Vector2 center = {fruitwidth/2, fruitwidth/2};
    float temp = (fruit.width - fruit.height)/2;
    DrawTexturePro(fruit,
        (Rectangle){0 , 0, fruit.width, fruit.height}, 
        (Rectangle){CentreX, CentreY + temp, fruitwidth, fruitwidth - temp}, 
        center, rotation, WHITE);
}
                   // draws each asset with proper (square) sizing and not extending the assets

void UnloadAssets(){
    UnloadTexture2D(fruit, 20);
    UnloadTexture2D(deadfruit, 20);
    UnloadTexture2D(splatter, 9);
    UnloadTexture2D(bomb, 4);
    UnloadTexture2D(bombfuse, 4);
    UnloadTexture2D(PreviousSplatter, maxSplatters);
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

void EmptyTextureMatrix(Texture2D *texture, int elements){
    for (int i = 0 ; i < elements ; i++)
    texture[i] = EMPTY;
}

void EmptyVectorMatrix(Vector2 *vector, int elements){
    for (int i = 0 ; i < elements ; i++)
    vector[i] = (Vector2){0, 0};
}