#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "SDL2/SDL.h"

#define HEIGHT 900
#define WIDTH 1600
#define ASPECT_RATIO 1.3334f
#define ANIMATION_FPS 60
#define TRAIL_DURATION_SECONDS 1
#define POINT_COUNT 1000
#define SPEEDMULTI .5f
const float fpsInMS = 1.0f / ANIMATION_FPS;

enum PheremoneType {
    NEUTRAL,
    AVOID,
    FOLLOW
};

struct vec2df {
    float x,y;
};

struct vec2d {
    int16_t x,y;
};

struct point {
    struct vec2d pos; 
    struct vec2df velocity;
    int PheremoneType;
    uint8_t color[3];
};

struct pixel {
    int16_t duration;
    uint8_t color[3];

};


struct point points[POINT_COUNT];
//implement pheremone strength
int16_t pixels[WIDTH+1][HEIGHT+1] = {0};

int randInRange(int lower, int upper) {
    return rand() % (upper - lower + 1) + lower;
}

void determineDirection(struct point *p) {
    int x = p->pos.x + p->velocity.x;
    int y = p->pos.y + p->velocity.y;
    switch (p->PheremoneType) {
        case (FOLLOW): {
        if (pixels[x][y] < 0) {
            p->velocity.x = randInRange(-1,1)*SPEEDMULTI;
            p->velocity.y = randInRange(-1,1)*SPEEDMULTI;
            } 
        }
        case (AVOID): {
        if (pixels[x][y] > 0) {
            p->velocity.x = randInRange(-1,1)*SPEEDMULTI;
            p->velocity.y = randInRange(-1,1)*SPEEDMULTI;
            }             
        }
    }
    return;
}

void animate(SDL_Renderer *renderer, double deltaTime) {
    for (int i = 0; i < *(&points + 1) - points; i++) {
        if (points[i].pos.x >= WIDTH) {
            points[i].pos.x = WIDTH;
            points[i].velocity.x = randInRange(-1,1)*SPEEDMULTI;
        } else if (points[i].pos.x <= 1){
            points[i].pos.x = 1;
            points[i].velocity.x = randInRange(-1,1)*SPEEDMULTI; 
        }
        if (points[i].pos.y >= HEIGHT) {
            points[i].pos.y = HEIGHT;
            points[i].velocity.y = randInRange(-1,1)* SPEEDMULTI;
        } else if (points[i].pos.y <= 1){
            points[i].pos.y = 1;
            points[i].velocity.y = randInRange(-1,1)* SPEEDMULTI; 
        }
        determineDirection(&points[i]);
        pixels[points[i].pos.x][points[i].pos.y] = ANIMATION_FPS * TRAIL_DURATION_SECONDS;
    }
}

void update(SDL_Window* window, SDL_Renderer *renderer, bool a, double deltaTime) {
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);
    if (a) {
        animate(renderer, deltaTime);
    }
    for (unsigned int i=0; i < WIDTH; i++) {
        for (unsigned int j=0; j < HEIGHT; j++) {
            if (pixels[i][j] > 0) {
                SDL_SetRenderDrawColor(renderer, 255,0,0, 255);
                SDL_RenderDrawPoint(renderer,i,j);
                if (a) {
                    pixels[i][j]--;
                }
            }
        }
    }
    for (unsigned int i=0; i < *(&points+1) - points; i++) {
        points[i].pos.x += points[i].velocity.x * deltaTime;
        points[i].pos.y += points[i].velocity.y * deltaTime;

        SDL_SetRenderDrawColor(renderer, points[i].color[0], points[i].color[1], points[i].color[2], 255);
        SDL_RenderDrawPoint(renderer, points[i].pos.x, points[i].pos.y);
    }

    SDL_RenderPresent(renderer);
    return;
}


void init(SDL_Window* window, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);
    srand(time(0));
    for (int i = 0; i < *(&points + 1) - points; i++) {
        points[i].PheremoneType = randInRange(1,2);
        points[i].pos.x =  WIDTH / 2;
        points[i].pos.y = HEIGHT / 2;
        points[i].velocity.x = randInRange(-1,1)*SPEEDMULTI;
        points[i].velocity.y = randInRange(-1,1)*SPEEDMULTI;
        if (points[i].velocity.x == 0 || points[i].velocity.y == 0) {
            points[i].velocity.x = randInRange(-1,1)* SPEEDMULTI; 
            points[i].velocity.y = randInRange(-1,1)* SPEEDMULTI; 
        }
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
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
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
        update(window, renderer, animate, deltaTime);
    }   

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

