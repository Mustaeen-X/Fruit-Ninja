#include "raylib.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG 0
#define ScreenWidth 1400
#define ScreenHeight 900
#define TINT 0
#ifndef TINT
    #define Red_BG 97
    #define Green_BG 50
    #define Blue_BG 24
#endif
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
#define STATE_COUNTDOWN 3
#define STATE_LEADERBOARD 4
#define STATE_MENU_EXIT 5
#define strikes 5
#define maxfruits 10
#define throwspeed 2200
#define slidespeed 5000
#define slidedespeed .3

#define MAX_NAME_LEN 15
#define MAX_PLAYERS 100

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
} FruitThrow; 

typedef struct {
    char name[MAX_NAME_LEN + 1];
    int score;
} LeaderboardEntry;

FruitThrow thrownfruit[maxfruits];

LeaderboardEntry leaderboard[MAX_PLAYERS];

char currentPlayer[MAX_NAME_LEN + 1] = "\0";

int fruit_throw_count, wave=0, MusicVolume=1, SFXVolume=1, PAUSE=0, Cursor=1, Close=0, totalPlayers = 0, tint=TINT, score = 0, highScore = 0, missedFruits = 0, lastComboHit = 0, comboCount = 0, currentState = STATE_MENU;;

float gameOverTimer = 0, countdowntimer = 0, comboDisplayTimer = 0, flickertimer=0;

bool typingName = false, isNewHighScore = false;

Texture2D *allocate_TextureMatrix(Texture2D *array, int elements);
Texture2D fruit[20], deadfruit[20], splatter[9], bomb[4], bombfuse[4], KNIFE, EMPTY, MusicIcon, BACKGROUND, Menu_Map, *PreviousSplatter;

Vector2 *allocate_VectorMatrix(Vector2 *array, int elements);
Vector2 *PreviousSplatterPosition, *Trailpositions, gravity={0,throwspeed}, slidingAcc={0,slidespeed}, menuPos={0,-ScreenHeight}, menuSpeed={0,0};

Color *allocate_ColorMatrix(Color *array, int elements);
Color *TrailColor;

Rectangle nameBox = {ScreenWidth/2 - 120, 220, 240, 45};
Rectangle lbButton = {ScreenWidth/2 - 120, 280, 240, 60};
Rectangle PlayButton = {ScreenWidth/2 - 120, 360, 240, 70};
Rectangle QuitButton = {ScreenWidth/2 - 120, 450, 240, 70};
Rectangle SFXButton = {ScreenWidth - 300, ScreenHeight - 220, 70, 70};
Rectangle MusicButton = {ScreenWidth - 180, ScreenHeight - 220, 70, 70};
Rectangle CursorButton = {ScreenWidth - 420, ScreenHeight - 220, 70, 70};
Rectangle backButton = {ScreenWidth/2 - 120, ScreenHeight - 130, 240, 60};

Sound Throw[4], Slash[3], Splat[3], EXPLODE, fuse, GameOver, HIGHSCORE, MENU, PLAYFN, COUNTDOWN, COMBO, SUPERCOMBO;

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
void ShowLeaderboardScreen();
int Background();
void ShowMenuExit();
void PlayGame();
void CustomCursor();
void StorePreviousSplatters(int Splatter_Index, Vector2 Splatter_Position);
void PreviousSplatters();
void StoreTrails();
void DrawTrails();
void CountDownScreen();
void DrawScoreAndStrikes();
void GameOverScreen();
void ResetGame();
void LoadLeaderboard();
void SaveLeaderboard();
void UpdateLeaderboard(const char* name, int newScore);
int GetPersonalHighScore(const char* name);

int main(){
    LoadGame();
    while(!WindowShouldClose() && !Close){

        float dt = GetFrameTime();
        BeginDrawing();
        Background();

        GameState();

        DrawText("ruhanCodes119 | Mustaeen-X", ScreenWidth - MeasureText("ruhanCodes119 | Mustaeen-X", 18) - 15, ScreenHeight - 25, 18, (Color){200, 200, 200, 180});

        CustomCursor();
        if (currentState == STATE_GAMEPLAY || currentState == STATE_GAMEOVER || currentState == STATE_COUNTDOWN) StoreTrails();
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

void LoadLeaderboard() {
    totalPlayers = 0;
    FILE *f = fopen("leaderboard.txt", "r");
    if (f) {
        while (fscanf(f, "%15s %d", leaderboard[totalPlayers].name, &leaderboard[totalPlayers].score) == 2) {
            totalPlayers++;
            if (totalPlayers >= MAX_PLAYERS) break;
        }
        fclose(f);
    }
    if (totalPlayers > 0) strcpy(currentPlayer, leaderboard[0].name); 
}

void SaveLeaderboard() {
    FILE *f = fopen("leaderboard.txt", "w");
    if (f) {
        for (int i = 0; i < totalPlayers; i++) {
            fprintf(f, "%s %d\n", leaderboard[i].name, leaderboard[i].score);
        }
        fclose(f);
    }
}

void UpdateLeaderboard(const char* name, int newScore) {
    int idx = -1;
    for (int i = 0; i < totalPlayers; i++) {
        if (strcmp(leaderboard[i].name, name) == 0) {
            idx = i;
            break;
        }
    }
    if (idx != -1) {
        if (newScore > leaderboard[idx].score) leaderboard[idx].score = newScore;
    } else {
        if (totalPlayers < MAX_PLAYERS) {
            strcpy(leaderboard[totalPlayers].name, name);
            leaderboard[totalPlayers].score = newScore;
            totalPlayers++;
        }
    }

    for (int i = 0; i < totalPlayers - 1; i++) {
        for (int j = i + 1; j < totalPlayers; j++) {
            if (leaderboard[j].score > leaderboard[i].score) {
                LeaderboardEntry temp = leaderboard[i];
                leaderboard[i] = leaderboard[j];
                leaderboard[j] = temp;
            }
        }
    }
    SaveLeaderboard();
}

int GetPersonalHighScore(const char* name) {
    for (int i = 0; i < totalPlayers; i++) {
        if (strcmp(leaderboard[i].name, name) == 0) return leaderboard[i].score;
    }
    return 0;
}

void LoadGame(){
    InitMatrices();
    InitWindow(ScreenWidth, ScreenHeight,"Fruit Ninja");
    InitAudioDevice();
    SetTargetFPS(60);
    LoadSounds();
    LoadAssets();
    LoadLeaderboard();
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

    Menu_Map = LoadTexture("assets/menu_map.png");
    BACKGROUND = LoadTexture("assets/background.png");

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
}

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
    COUNTDOWN = LoadSound("assets/audio/countdown.wav");
    COMBO = LoadSound("assets/audio/combo.mp3");
    SUPERCOMBO = LoadSound("assets/audio/supercombo.mp3");
}

void GameState(){
    float dt = GetFrameTime();

    if(IsKeyPressed(KEY_P)) PAUSE = !PAUSE;
    Rectangle pauseBox = {ScreenWidth/2 - 120, ScreenHeight/2 - 35, 250, 100};
    if(PAUSE) if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), pauseBox)) PAUSE = !PAUSE;
    SetMasterVolume(1 - PAUSE*.7);

    if (currentState == STATE_GAMEPLAY || currentState == STATE_GAMEOVER || currentState == STATE_COUNTDOWN) PreviousSplatters();

    if (!PAUSE) {
        switch (currentState) {
            case STATE_MENU:
                ShowMenu();
                break;
            case STATE_LEADERBOARD:
                ShowLeaderboardScreen();
                break;
            case STATE_MENU_EXIT:
                ShowMenuExit();
                break;
            case STATE_GAMEPLAY:
                PlayGame();
                break;
            case STATE_COUNTDOWN:
                CountDownScreen();
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
    if (currentState == STATE_GAMEPLAY || currentState == STATE_GAMEOVER || currentState == STATE_COUNTDOWN) DrawTrails();
}

int Background(){
    DrawTexturePro(BACKGROUND,
    (Rectangle){ 0, 0, BACKGROUND.width, BACKGROUND.height },
    (Rectangle){ 0, 0, ScreenWidth, ScreenHeight},
    (Vector2){ 0, 0 }, 0, WHITE);
    tint = tint > TINT+10 ? tint-10 : TINT;
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, (Color){255, 255, 255, tint});
    if (tint == TINT) return 1;
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
    comboCount = 0;
    comboDisplayTimer = 0;
    for (int i = 0; i < maxfruits; i++) thrownfruit[i].isitonscreen = 0;
    InitializeFruitThrow(0);
}

void ShowMenu() {
    if(MusicVolume) if(!IsSoundPlaying(MENU)) PlaySound(MENU);
    if(MusicVolume) if(!IsSoundPlaying(MENU)) PlaySound(MENU);

    float dt = GetFrameTime();


    if (menuPos.y<0 || menuSpeed.y != 0) {
        menuSpeed = Vector2Add(menuSpeed, Vector2Scale(slidingAcc, dt));
        menuPos   = Vector2Add(menuPos, Vector2Scale(menuSpeed, dt));

        if (menuPos.y >= 0) {
            menuPos.y = 0;
            menuSpeed.y = -menuSpeed.y * slidedespeed;
            if (menuSpeed.y < 150 && menuSpeed.y > -150) menuSpeed.y = 0.0f;
        }
    }

    DrawTexturePro(Menu_Map,
    (Rectangle){ 0, 0, Menu_Map.width, Menu_Map.height },
    (Rectangle){ 15, 15+menuPos.y, ScreenWidth-30, ScreenHeight-30 },
    (Vector2){ 0, 0 }, 0, WHITE);

    DrawText("FRUIT NINJA", ScreenWidth/2 - MeasureText("FRUIT NINJA", 70)/2, 120+menuPos.y, 70, YELLOW);

    nameBox.y      = 220 + menuPos.y;
    lbButton.y     = 280 + menuPos.y;
    PlayButton.y   = 360 + menuPos.y;
    QuitButton.y   = 450 + menuPos.y;
    SFXButton.y    = (ScreenHeight - 220) + menuPos.y;
    MusicButton.y  = (ScreenHeight - 220) + menuPos.y;
    CursorButton.y = (ScreenHeight - 220) + menuPos.y;

    bool hoveringName = CheckCollisionPointRec(GetMousePosition(), nameBox);
    bool hoveringLB = CheckCollisionPointRec(GetMousePosition(), lbButton);
    bool hoveringPLAY = CheckCollisionPointRec(GetMousePosition(), PlayButton);
    bool hoveringTMUSIC = CheckCollisionPointRec(GetMousePosition(), MusicButton);
    bool hoveringSFX = CheckCollisionPointRec(GetMousePosition(), SFXButton);
    bool hoveringCursor = CheckCollisionPointRec(GetMousePosition(), CursorButton);
    bool hoveringQuit = CheckCollisionPointRec(GetMousePosition(), QuitButton);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) typingName = hoveringName;

    if (typingName) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(currentPlayer) < MAX_NAME_LEN)) {
                currentPlayer[strlen(currentPlayer)] = (char)key;
                currentPlayer[strlen(currentPlayer)+1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) if(strlen(currentPlayer) > 0) currentPlayer[strlen(currentPlayer)-1] = '\0';
    }

    DrawRectangleRec(nameBox, typingName ? LIGHTGRAY : RAYWHITE);
    DrawRectangleLinesEx(nameBox, 3, typingName ? RED : DARKGRAY);
    if (strlen(currentPlayer) == 0 && !typingName) {
        DrawText("Enter Name...", nameBox.x + 10, nameBox.y + 12, 20, GRAY);
    } else {
        DrawText(currentPlayer, nameBox.x + 10, nameBox.y + 12, 25, DARKGRAY);
    }

    DrawRectangleRec(lbButton, hoveringLB ? ORANGE : GOLD);
    DrawRectangleLinesEx(lbButton, 4, DARKGRAY);
    DrawText("LEADERBOARD", lbButton.x + lbButton.width/2 - MeasureText("LEADERBOARD", 25)/2, lbButton.y + 18, 25, WHITE);
    if (hoveringLB && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) currentState = STATE_LEADERBOARD;

    DrawRectangleRec(PlayButton, (hoveringPLAY || (flickertimer > 0) && (((int)(flickertimer / 0.1)) % 2 == 0))? DARKGREEN : GREEN);
    DrawRectangleLinesEx(PlayButton, 5, DARKGRAY);
    DrawText("PLAY", PlayButton.x + PlayButton.width/2 - MeasureText("PLAY", 35)/2, PlayButton.y + 18, 35, WHITE);
    if (hoveringPLAY && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        if (strlen(currentPlayer) == 0) strcpy(currentPlayer, "Player");
        highScore = GetPersonalHighScore(currentPlayer);
        currentState = STATE_MENU_EXIT;
        menuSpeed = (Vector2){0,-1000};
        flickertimer = 3;
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

void ShowLeaderboardScreen() {
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, (Color){ 0, 0, 0, 210 });
    DrawText("TOP 10 SCORES", ScreenWidth/2 - MeasureText("TOP 10 SCORES", 60)/2, 80, 60, GOLD);
    
    for (int i = 0; i < 10; i++) {
        if (i < totalPlayers) {
            DrawText(TextFormat("%d. %s", i+1, leaderboard[i].name), ScreenWidth/2 - 200, 180 + i*40, 35, WHITE);
            DrawText(TextFormat("%d", leaderboard[i].score), ScreenWidth/2 + 100, 180 + i*40, 35, YELLOW);
        } else {
            DrawText(TextFormat("%d. ---", i+1), ScreenWidth/2 - 200, 180 + i*40, 35, GRAY);
            DrawText("0", ScreenWidth/2 + 100, 180 + i*40, 35, DARKGRAY);
        }
    }
    bool hoveringBack = CheckCollisionPointRec(GetMousePosition(), backButton);
    DrawRectangleRec(backButton, hoveringBack ? RED : MAROON);
    DrawRectangleLinesEx(backButton, 4, DARKGRAY);
    DrawText("BACK", backButton.x + backButton.width/2 - MeasureText("BACK", 35)/2, backButton.y + 12, 35, WHITE);

    if (hoveringBack && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        currentState = STATE_MENU;
        menuPos = (Vector2){0,-ScreenHeight};
        menuSpeed = (Vector2){0,0};
    }
}

void ShowMenuExit() {
    if(flickertimer>0) flickertimer -= GetFrameTime();

    float dt = GetFrameTime();

    menuSpeed = Vector2Add(menuSpeed, Vector2Scale(slidingAcc, dt));
    menuPos   = Vector2Add(menuPos, Vector2Scale(menuSpeed, dt));

    DrawTexturePro(Menu_Map,
        (Rectangle){0,0,Menu_Map.width,Menu_Map.height },
        (Rectangle){15,15 + menuPos.y,ScreenWidth - 30,ScreenHeight - 30 },
        (Vector2){0,0}, 0, WHITE);

    DrawText("FRUIT NINJA", ScreenWidth/2 - MeasureText("FRUIT NINJA", 70)/2, 120 + menuPos.y, 70, YELLOW);

    nameBox.y      = 220 + menuPos.y;
    lbButton.y     = 280 + menuPos.y;
    PlayButton.y   = 360 + menuPos.y;
    QuitButton.y   = 450 + menuPos.y;
    SFXButton.y    = (ScreenHeight - 220) + menuPos.y;
    MusicButton.y  = (ScreenHeight - 220) + menuPos.y;
    CursorButton.y = (ScreenHeight - 220) + menuPos.y;

    DrawRectangleRec(nameBox, RAYWHITE);
    DrawRectangleLinesEx(nameBox, 3, DARKGRAY);
    DrawText(currentPlayer, nameBox.x + 10, nameBox.y + 12, 25, DARKGRAY);

    DrawRectangleRec(lbButton, GOLD);
    DrawRectangleLinesEx(lbButton, 4, DARKGRAY);
    DrawText("LEADERBOARD", lbButton.x + lbButton.width/2 - MeasureText("LEADERBOARD", 25)/2, lbButton.y + 18, 25, WHITE);

    DrawRectangleRec(PlayButton, (flickertimer > 0) && (((int)(flickertimer / 0.05)) % 2 == 0)? DARKGREEN:GREEN);
    DrawRectangleLinesEx(PlayButton, 5, DARKGRAY);
    DrawText("PLAY", PlayButton.x + PlayButton.width/2 - MeasureText("PLAY", 35)/2, PlayButton.y + 18, 35, WHITE);

    DrawRectangleRec(QuitButton, (Color){255, 100, 252, 255});
    DrawRectangleLinesEx(QuitButton, 5, (Color){200, 60, 195, 255});
    DrawText("QUIT GAME", QuitButton.x + QuitButton.width/2 - MeasureText("QUIT GAME", 35)/2, QuitButton.y + 18, 35, WHITE);

    DrawRectangleRec(MusicButton, (Color){220, 220, 220, 255});
    DrawRectangleLinesEx(MusicButton, 5, (Color){100, 100, 100, 255});
    DrawAsset(MusicIcon, MusicButton.x + MusicButton.width/2, MusicButton.y + MusicButton.height/2, MusicButton.width - 20, 0);

    DrawRectangleRec(SFXButton, (Color){220, 220, 220, 255});
    DrawRectangleLinesEx(SFXButton, 5, (Color){100, 100, 100, 255});
    DrawText("SFX", SFXButton.x + 10, SFXButton.y + 25, 25, (Color){100, 100, 100, 255});

    DrawRectangleRec(CursorButton, (Color){220, 220, 220, 255});
    DrawRectangleLinesEx(CursorButton, 5, (Color){100, 100, 100, 255});
    DrawAsset(KNIFE, CursorButton.x + CursorButton.width/2, CursorButton.y + CursorButton.height/2, CursorButton.width - 20, 90);

    if (menuPos.y>=ScreenHeight) {
        StopSound(MENU);
        if (SFXVolume) PlaySound(COUNTDOWN);
        countdowntimer = 4.2;
        currentState = STATE_COUNTDOWN;
    }
}

void DrawScoreAndStrikes() {
    if(MusicVolume && currentState==STATE_GAMEPLAY) if(!IsSoundPlaying(PLAYFN)) PlaySound(PLAYFN);

    Color scoreColor = (score > highScore && highScore > 0) ? GOLD : YELLOW;
    DrawText(TextFormat("SCORE: %d", score), ScreenWidth - 250 + countdowntimer*300, 30, 35, scoreColor);
    
    if (score > highScore && highScore > 0) {
        DrawText("NEW PERSONAL BEST!", ScreenWidth - 280, 70, 20, ORANGE);
    }

    DrawText(TextFormat("STRIKES: %d / %d", missedFruits, strikes), 30 - countdowntimer*300, 30, 50, (missedFruits >= 2) ? RED : MAROON);

    if (comboDisplayTimer > 0) {
        comboDisplayTimer -= GetFrameTime();
        int alpha = comboDisplayTimer > 1 ? 255 : (int)(comboDisplayTimer * 255);
        if (alpha < 0) alpha = 0;
        DrawText(TextFormat("COMBO: %d!!!", lastComboHit), ScreenWidth/2 - MeasureText(TextFormat("COMBO: %d!!!", lastComboHit), 60)/2, ScreenHeight/2 - 30, 60, (((int)(comboDisplayTimer/0.1))%2) == 0? (Color){ 255, 165, 0, alpha } : (Color){ 190, 117, 0, alpha });
    }

    #if DEBUG
    char debugtext[100];
    sprintf(debugtext, "Wave Number: %d", wave);
    DrawText(debugtext, 30, 100, 35, GRAY);

    sprintf(debugtext, "Debug Mode: ON");
    DrawText(debugtext, 30, 150, 35, GRAY);

    sprintf(debugtext, "Combo Count: %d",comboCount);
    DrawText(debugtext, 30, 200, 35, GRAY);
    #endif
}

void TriggerGameOver() {
    if(IsSoundPlaying(PLAYFN)) StopSound(PLAYFN);

    if (score > highScore) {
        highScore = score;
        isNewHighScore = true;
    }

    if(SFXVolume) {
        if(isNewHighScore) PlaySound(HIGHSCORE);
        else PlaySound(GameOver);
        }
    
    UpdateLeaderboard(currentPlayer, score);
    gameOverTimer = 3;
    currentState = STATE_GAMEOVER;
}

void CountDownScreen() {
    countdowntimer -= GetFrameTime();

    const char* countdown;
    Color timercolor;

    if (countdowntimer > 3){
        countdown = "3";
        timercolor = RED;
    }else if(countdowntimer > 2){
        countdown = "2";
        timercolor = ORANGE;
    }else if(countdowntimer > 1){
        countdown = "1";
        timercolor = YELLOW;
    }else if(countdowntimer > 0){
        countdown = "GO!";
        timercolor = GREEN;
        DrawScoreAndStrikes();
    }else{
        ResetGame();
        currentState = STATE_GAMEPLAY;
        return;
    }

    DrawText(countdown, ScreenWidth/2 - MeasureText(countdown,120)/2, ScreenHeight/2 - 120/2, 120, timercolor);
}

void GameOverScreen() {
    DrawRectangle(0, 0, ScreenWidth, ScreenHeight, (Color){ 0, 0, 0, 200 });
    
    DrawText("GAME OVER", ScreenWidth/2 - MeasureText("GAME OVER", 80)/2, 250, 80, RED);
    
    if (isNewHighScore) {
        DrawText("NEW PERSONAL BEST!!", ScreenWidth/2 - MeasureText("NEW PERSONAL BEST!!", 40)/2, 360, 40, GOLD);
    }

    DrawText(TextFormat("FINAL SCORE: %d", score), ScreenWidth/2 - MeasureText(TextFormat("FINAL SCORE: %d", score), 35)/2, 430, 35, WHITE);


    gameOverTimer -= GetFrameTime();
    DrawText(TextFormat("Returning to menu in %d...", (int)ceil(gameOverTimer)), ScreenWidth/2 - MeasureText("Returning to menu in X...", 20)/2, 530, 20, LIGHTGRAY);
    
    if (gameOverTimer <= 0) {
        EmptyTextureMatrix(PreviousSplatter, maxSplatters);
        EmptyVectorMatrix(PreviousSplatterPosition, maxSplatters);
        EmptyVectorMatrix(Trailpositions, TrailLength);
        wave = 0;
        currentState = STATE_MENU;
        menuPos = (Vector2){0,-ScreenHeight};
        menuSpeed = (Vector2){0,0};
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
}

void PlayGame(){
    float dt = GetFrameTime();
    UpdateFruitThrow();
    if(currentState == STATE_GAMEPLAY) DrawScoreAndStrikes();

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
                tint = 255;
                comboCount = 0;
                score -= 30;
                if (score < 0) {
                    TriggerGameOver();
                }
            }
            else {
                if(SFXVolume) PlaySound(Splat[GetRandomValue(0,2)]);
                comboCount++;
                
                int pointBonus = 10;
                if (comboCount >= 50) pointBonus = 35;
                else if (comboCount >= 40) pointBonus = 30;
                else if (comboCount >= 30) pointBonus = 25;
                else if (comboCount >= 20) pointBonus = 20;
                else if (comboCount >= 10) pointBonus = 15;
                
                score += pointBonus;

                if (comboCount % 10 == 0 && comboCount <= 50) {
                    comboDisplayTimer = 2;
                    lastComboHit = comboCount;
                    if(SFXVolume) PlaySound(comboCount==20? SUPERCOMBO:COMBO);
                }
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
            comboCount = 0;
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
}

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
}

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