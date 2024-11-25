#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define GRID_SIZE 20

typedef struct
{
    int x, y;
} Segment;
typedef enum
{
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
Mix_Music *backgroundMusic = NULL;
Mix_Chunk *foodSound = NULL;
TTF_Font *font = NULL;
SDL_Surface *textSurface = NULL;
SDL_Color textColor = {255, 0, 255, 255};
Segment snake[100];
int snake_length = 1;
Segment food;
Direction direction = RIGHT;
bool isRunning = true;
bool gameStarted = false;
int score = 0;

bool init()
{
    window = SDL_CreateWindow("Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window)
        return false;
    if (TTF_Init())
        return false;
    font = TTF_OpenFont("Limon.ttf", 24);
    if (!font)
        return false;
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048))
        return false;
    if (SDL_Init(SDL_INIT_VIDEO || SDL_INIT_AUDIO))
        return false;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer)
        return false;
    return true;
}

bool loadMedia()
{
    backgroundMusic = Mix_LoadMUS("Limon.mp3");
    if (!backgroundMusic)
        return false;
    foodSound = Mix_LoadWAV("Limon.wav");
    if (!foodSound)
        return false;
    return true;
}

void spawnFood()
{
    food.x = rand() % (SCREEN_WIDTH / GRID_SIZE);
    food.y = rand() % (SCREEN_HEIGHT / GRID_SIZE);
}

void handleInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
            isRunning = false;
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_UP:
                if (direction != DOWN || direction != LEFT || direction != RIGHT)
                    direction = UP;
                break;
            case SDLK_DOWN:
                if (direction != UP || direction != LEFT || direction != RIGHT)
                    direction = DOWN;
                break;
            case SDLK_LEFT:
                if (direction != RIGHT || direction != UP || direction != DOWN)
                    direction = LEFT;
                break;
            case SDLK_RIGHT:
                if (direction != LEFT || direction != UP || direction != DOWN)
                    direction = RIGHT;
                break;
            }
        }
        else if (event.type == SDL_MOUSEBUTTONDOWN && !gameStarted)
        {
            int mouseX = event.button.x, mouseY = event.button.y;
            if (mouseX >= 230 && mouseX <= 410 && mouseY >= 200 && mouseY <= 250)
            {
                gameStarted = true;
                spawnFood();
            }
        }
    }
}

void update()
{
    if (!gameStarted)
        return;
    for (int i = snake_length - 1; i > 0; --i)
        snake[i] = snake[i - 1];
    switch (direction)
    {
    case RIGHT:
        snake[0].x++;
        break;
    case LEFT:
        snake[0].x--;
        break;
    case UP:
        snake[0].y--;
        break;
    case DOWN:
        snake[0].y++;
        break;
    }
    if (snake[0].x < 0)
        snake[0].x = (SCREEN_WIDTH / GRID_SIZE) - 1;
    if (snake[0].x >= SCREEN_WIDTH / GRID_SIZE)
        snake[0].x = 0;
    if (snake[0].y < 0)
        snake[0].y = (SCREEN_HEIGHT / GRID_SIZE) - 1;
    if (snake[0].y >= SCREEN_HEIGHT / GRID_SIZE)
        snake[0].y = 0;
    if (snake[0].x == food.x && snake[0].y == food.y)
    {
        snake_length += 3;
        score += 10;
        spawnFood();
        Mix_PlayChannel(-1, foodSound, 0);
    }
    for (int i = 1; i < snake_length; ++i)
    {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y)
            isRunning = false;
    }
}

void renderScore()
{
    char scoreText[20];
    snprintf(scoreText, sizeof(scoreText), "Score: %d", score);
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {530, 10, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_RenderPresent(renderer);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderGameOver()
{
    SDL_SetRenderDrawColor(renderer, 25, 25, 112, 255);
    SDL_RenderClear(renderer);
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, "Game Over!", textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 2, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
    char scoreText[20];
    snprintf(scoreText, sizeof(scoreText), "Final Score: %d", score);
    textSurface = TTF_RenderText_Solid(font, scoreText, textColor);
    textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    textRect = {SCREEN_WIDTH / 4, SCREEN_HEIGHT / 3, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_RenderPresent(renderer);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void renderStartButton()
{
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    SDL_Rect startButton = {230, 200, 180, 50};
    bool isPresent = (mouseX >= startButton.x && mouseX <= startButton.x + startButton.w && mouseY >= startButton.y && mouseY <= startButton.y + startButton.h);
    SDL_Color buttonColor = isPresent ? (SDL_Color){0, 191, 255, 255} : (SDL_Color){173, 216, 230, 255};
    SDL_SetRenderDrawColor(renderer, buttonColor.r, buttonColor.g, buttonColor.b, buttonColor.a);
    SDL_RenderFillRect(renderer, &startButton);
    SDL_SetRenderDrawColor(renderer, 0, 0, 139, 255);
    SDL_RenderDrawRect(renderer, &startButton);
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, "Start Game", textColor);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_Rect textRect = {startButton.x + (startButton.w - textSurface->w) / 2, startButton.y + (startButton.h - textSurface->h) / 2, textSurface->w, textSurface->h};
    SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
    SDL_RenderPresent(renderer);
    SDL_FreeSurface(textSurface);
    SDL_DestroyTexture(textTexture);
}

void render()
{

    SDL_SetRenderDrawColor(renderer, 173, 216, 230, 255);
    SDL_RenderClear(renderer);

    if (!gameStarted)
        renderStartButton();
    else
    {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_Rect foodRect = {food.x * GRID_SIZE, food.y * GRID_SIZE, GRID_SIZE, GRID_SIZE};
        SDL_RenderFillRect(renderer, &foodRect);
        for (int i = 0; i < snake_length; ++i)
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_Rect snakeRect = {snake[i].x * GRID_SIZE, snake[i].y * GRID_SIZE, GRID_SIZE, GRID_SIZE};
            SDL_RenderFillRect(renderer, &snakeRect);
        }
        renderScore();
    }
    SDL_RenderPresent(renderer);
}

void cleanup()
{
    Mix_FreeMusic(backgroundMusic);
    Mix_FreeChunk(foodSound);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    Mix_CloseAudio();
    TTF_Quit();
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    if (!init())
        return -1;
    if (!loadMedia())
        return -1;
    if (Mix_PlayMusic(backgroundMusic, -1) == 1)
        printf("Failed to play music: %s\n", Mix_GetError());
    while (isRunning)
    {
        handleInput();
        update();
        render();
        SDL_Delay(150);
    }
    renderGameOver();
    SDL_Delay(3000);
    cleanup();
    return 0;
}

