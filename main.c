#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "SDL2/SDL.h"

#define HEIGHT 900
#define WIDTH 1600
#define ASPECT_RATIO 1.3334f
#define ANIMATION_FPS 60
#define TRAIL_DURATION_SECONDS 1
#define POINT_COUNT 1
#define SPEEDMULTI 1.f
#define LOOK_AHEAD 3
#define LOOK_AHEAD_MULTI 1 / LOOK_AHEAD
#define MATH_PI 3.14159265358979323846
static const float fpsInMS = 1.0f / ANIMATION_FPS;

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


static struct point points[POINT_COUNT];
static struct pixel pixels[WIDTH+1][HEIGHT+1] = {0};

int randInRange(int lower, int upper) {
    int range = upper - lower + 1;
    return lower + (int)((double)range * rand() / (RAND_MAX + 1.0));
}

float calcDist(struct vec2d p1, struct vec2d p2) {
    return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
}
struct vec2df adjustVelocity(struct vec2d currentPos, struct vec2d newPos) {
    struct vec2df newVelocity;

    newVelocity.x = newPos.x - currentPos.x;
    newVelocity.y = newPos.y - currentPos.y;
    return newVelocity;
}
static inline float clamp(float d, int min, int max) {
    const int t = d < min ? min : d;
    return t > max ? max : t;
}
void determineDirection(struct point *p) {
    struct vec2df refDirection;
    refDirection.x = -p->velocity.y;
    refDirection.y = p->velocity.x;

    float minAngle = atan2(refDirection.y, refDirection.x) - MATH_PI * .25;
    float maxAngle = atan2(refDirection.y, refDirection.x) + MATH_PI * .25;
    int cnt = -1;
    struct vec2d pick[(int)pow(LOOK_AHEAD,2) + 1];

    for (int x = 0; x <=LOOK_AHEAD; x++) {
        for (int y = -x; y <= x; y++) {
            float rotatedX = x * cos(minAngle) - y * sin(minAngle);
            float rotatedY = x * sin(minAngle) + y * cos(minAngle);
            struct vec2d pixel = {p->pos.x + rotatedX, p->pos.y + rotatedY};
            float pixelAngle = atan2(pixel.y - p->pos.y, pixel.x - p->pos.x);

            if (calcDist(p->pos,pixel) <= LOOK_AHEAD && pixelAngle >= minAngle && pixelAngle <= maxAngle) {
                cnt++;
                pick[cnt] = pixel;
            }
        }
    }
    if (cnt == -1) {
        p->velocity.x = randInRange(-1,1)*SPEEDMULTI;
        p->velocity.y = randInRange(-1,1)*SPEEDMULTI;
        return;
    }
    struct vec2d pointAt = pick[randInRange(0,cnt)];
    struct vec2df newV = adjustVelocity(p->pos, pointAt);
    p->velocity.x = newV.x;
    p->velocity.y = newV.y;
    return;
}

void animate(SDL_Renderer *renderer, double deltaTime) {
    for (int i = 0; i < *(&points + 1) - points; i++) {
        determineDirection(&points[i]);
        points[i].pos.x += points[i].velocity.x;
        points[i].pos.y += points[i].velocity.y;
        points[i].pos.x = clamp(points[i].pos.x,1,WIDTH);
        points[i].pos.y = clamp(points[i].pos.y,1,HEIGHT);
        pixels[points[i].pos.x][points[i].pos.y].duration = ANIMATION_FPS * TRAIL_DURATION_SECONDS;
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
            if (pixels[i][j].duration > 0) {
                SDL_SetRenderDrawColor(renderer, 255,0,0, 255);
                SDL_RenderDrawPoint(renderer,i,j);
                if (a) {
                    pixels[i][j].duration--;
                }
            }
        }
    }
    for (unsigned int i=0; i < *(&points+1) - points; i++) {
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
        points[i].pos.x =  randInRange(0, WIDTH);
        points[i].pos.y = randInRange(0, HEIGHT);
        points[i].velocity.x = randInRange(-1,1);
        points[i].velocity.y = randInRange(-1,1);
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

