#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "SDL2/SDL.h"

#define HEIGHT 720
#define WIDTH 1280
#define ASPECT_RATIO 1.3334f
#define ANIMATION_FPS 100
#define MAXSIZE 25
#define POINT_COUNT 100
const float fpsInMS = 1.0f / ANIMATION_FPS;

struct vec2d {
    float x,y;
};

struct point {
    struct vec2d pos, velocity;
    struct vec2d trail[5];
    int color[3];
};

struct point points[POINT_COUNT];


int randInRange(int lower, int upper) {
    return rand() % (upper - lower + 1) + lower;
}

void animate(SDL_Renderer *renderer, double deltaTime) {
    for (int i = 0; i < *(&points + 1) - points; i++) {
        if (points[i].pos.x >= WIDTH) {
            points[i].pos.x = WIDTH;
            points[i].velocity.x = randInRange(-1,1) *.25f;
        } else if (points[i].pos.x <= 1){
            points[i].pos.x = 0;
            points[i].velocity.x = randInRange(-1,1) *.25f; 
        }
        if (points[i].pos.y >= HEIGHT) {
            points[i].pos.y = HEIGHT;
            points[i].velocity.y = randInRange(-1,1) *.25f;
        } else if (points[i].pos.y <= 1){
            points[i].pos.y = 0;
            points[i].velocity.y = randInRange(-1,1) *.25f; 
        }
        if (points[i].velocity.x == 0 || points[i].velocity.y == 0) {
            points[i].velocity.x = randInRange(-1,1) *.25f; 
            points[i].velocity.y = randInRange(-1,1) *.25f; 
        }
    }

}
int count = 0;
void update(SDL_Window* window, SDL_Renderer *renderer, SDL_Texture* trail, bool a, double deltaTime) {
    count++;
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);
    if (a) {
        animate(renderer, deltaTime);
    }
    
    SDL_SetRenderTarget(renderer, NULL);
    for (int i=0; i < *(&points+1) - points; i++) {
        points[i].pos.x += points[i].velocity.x * deltaTime;
        points[i].pos.y += points[i].velocity.y * deltaTime;

        SDL_SetRenderDrawColor(renderer, points[i].color[0], points[i].color[1], points[i].color[2], 255);
        SDL_RenderDrawPoint(renderer, points[i].pos.x,points[i].pos.y);
    }
    SDL_SetRenderTarget(renderer, trail);
    SDL_RenderCopy(renderer, trail, NULL, NULL);
    SDL_RenderPresent(renderer);

    return;
}

void init(SDL_Window* window, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);
    srand(time(0));
    for (int i = 0; i < *(&points + 1) - points; i++) {
        points[i].pos.x =  WIDTH / 2;
        points[i].pos.y = HEIGHT / 2;
        points[i].velocity.x = randInRange(-1,1) *.25f;
        points[i].velocity.y = randInRange(-1,1) *.25f;
        points[i].color[0] = points[i].color[1] = points[i].color[2] = 255;
        SDL_SetRenderDrawColor(renderer, points[i].color[0], points[i].color[1], points[i].color[2], 255);
        SDL_RenderDrawPoint(renderer, points[i].pos.x, points[i].pos.y);
    }
    SDL_RenderPresent(renderer);
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not init %s", SDL_GetError());
    } else {
        printf("SDL ready\n");
    }

    SDL_Window* window = SDL_CreateWindow("Atel2d", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_Texture *texture;
    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    bool animate, secondPassed, isRunning;
    animate = secondPassed = isRunning = true;
    double deltaTime = 0;
    Uint64 prev = 0;
    Uint64 now = SDL_GetPerformanceCounter();
    init(window, renderer);
    double timePassed;
    while(isRunning) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                isRunning = false;
                break;
            }
        }
        prev = now;
        now = SDL_GetPerformanceCounter();

        deltaTime = ((double)((now - prev)*1000 / (double)SDL_GetPerformanceFrequency()));
        timePassed += deltaTime;
        animate = timePassed > (fpsInMS * 1000.0f);
        if (animate) { timePassed = 0.0f;} 
        if (count >=100) {
            SDL_DestroyTexture(texture);
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_TARGET, WIDTH, HEIGHT);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            count = 0;
        }
        update(window, renderer, texture, animate, deltaTime);
    }   

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

