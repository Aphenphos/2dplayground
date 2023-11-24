#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "SDL2/SDL.h"

#define HEIGHT 480
#define WIDTH 640
#define ASPECT_RATIO 1.3334f
#define ANIMATION_FPS 30
#define MAXSIZE 25
const float fpsInMS = 1.0f / ANIMATION_FPS;

struct vec2d{
    float x,y;
};

struct point {
    struct vec2d pos;
    int color[3];
};

struct translate {
    struct vec2d start, end, currentStartLoc, inc;
    int currentStep;
};
static const struct translate emptyTranslate;

struct point points[25];
struct translate animationQueue[MAXSIZE];
bool isFree[MAXSIZE] = {true};

int randInRange(int lower, int upper) {
    return rand() % (upper - lower + 1) + lower;
}

void animate() {
    for (int i=0; i < MAXSIZE; i++) {
        if (isFree[i] == false) {
            animationQueue[i].currentStartLoc.x += animationQueue[i].inc.x;
            animationQueue[i].currentStartLoc.y += animationQueue[i].inc.y;
            animationQueue[i].currentStep++;
            
            if (animationQueue[i].currentStep >= ANIMATION_FPS) {
                animationQueue[i] = emptyTranslate;
                isFree[i] = true;
            }
        }
    }
}

void addTranslation(struct translate t) {
    for (int i = 0; i < MAXSIZE; i++) {
        if (isFree[i] == true) {
            isFree[i] = false;
            animationQueue[i] = t;
            return;
        }
    }
}
void update(SDL_Window* window, SDL_Renderer *renderer, bool a, double deltaTime) {
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < *(&points + 1) - points; i++) {
        SDL_SetRenderDrawColor(renderer, points[i].color[0], points[i].color[1], points[i].color[2], 255);
        SDL_RenderDrawPoint(renderer, points[i].pos.x,points[i].pos.y);
    }

    if (a) {
        animate();
    }
    SDL_SetRenderDrawColor(renderer, 0,0,255,255);
    for (int i = 0; i < MAXSIZE; i++) {
        if(isFree[i] == false) {
            SDL_RenderDrawLine(renderer, animationQueue[i].currentStartLoc.x, animationQueue[i].currentStartLoc.y, animationQueue[i].end.x, animationQueue[i].end.y);
        }
    }
    SDL_RenderPresent(renderer);
    return;
}


void init(SDL_Window* window, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 0,0,0,255);
    SDL_RenderClear(renderer);
    srand(time(0));
    for (int i = 0; i < *(&points + 1) - points; i++) {
        points[i].pos.x = randInRange(0,WIDTH);
        points[i].pos.y = randInRange(0,HEIGHT);
        points[i].color[0] = points[i].color[1] = points[i].color[2] = 255;
        SDL_SetRenderDrawColor(renderer, points[i].color[0], points[i].color[1], points[i].color[2], 255);
        SDL_RenderDrawPoint(renderer, points[i].pos.x,points[i].pos.y);
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

    bool animate, secondPassed, isRunning;
    animate = secondPassed = isRunning = true;
    double deltaTime = 0;
    Uint64 prev = 0;
    Uint64 now = SDL_GetPerformanceCounter();
    init(window, renderer);
    double timePassed, secondCounter;
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
        secondCounter += deltaTime;
        timePassed += deltaTime;
        animate = timePassed > (fpsInMS * 1000.0f);
        secondPassed = secondCounter > 100.0f;
        if (animate) { timePassed = 0.0f;} 
        if (secondPassed) {
            for (int i=0; i < 25; i++) {
                struct translate connection;
                connection.start = points[randInRange(0,25)].pos;
                connection.end = points[randInRange(0,25)].pos;
                connection.inc.x = (connection.end.x - connection.start.x) * fpsInMS;
                connection.inc.y = (connection.end.y - connection.start.y) * fpsInMS;
                connection.currentStartLoc = connection.start;
                connection.currentStep = 1;
                addTranslation(connection);
                connection.end = points[randInRange(0,25)].pos;
                connection.inc.x = (connection.end.x - connection.start.x) * fpsInMS;
                connection.inc.y = (connection.end.y - connection.start.y) * fpsInMS;
                connection.currentStartLoc = connection.start;
                connection.currentStep = 1;
                addTranslation(connection);
            }
            secondCounter = 0.0f;
        }
        update(window, renderer, animate, deltaTime);
    }   

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

