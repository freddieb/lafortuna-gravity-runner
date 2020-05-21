/* COMP2215: Task 02---MODEL ANSWER */
/* For La Fortuna board.            */


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include "rotary.h"

#define WIDTH LCDHEIGHT
#define HEIGHT LCDWIDTH

#define PLATFORM_HEIGHT 12
#define OBSTACLE_WIDTH 10

#define OBSTACLE_COUNT 4

volatile rectangle topPlatform;
volatile rectangle bottomPlatform;
volatile rectangle player;

rectangle obstacles[OBSTACLE_COUNT];
int scrollCount;
int scrollOffset = 0;
int size = 10;
uint8_t isGravityDown = 1;
int lastBtnClick = 0;
uint8_t stopGame = 0;


void init(void);
void showPlatforms(void);
void showPlayer(void);
void setPlayerBottom(void);
void setPlayerTop(void);
void showObstacles(void);
void scrollLoop(uint8_t isBtnClicked);
void gameOver(void);

void main(void) {
    init();

    clear_screen();

    showPlatforms();
    showPlayer();
    showObstacles();

    while (!stopGame) {
        scrollLoop(get_middle());
    }
}


void init(void) {
    /* 8MHz clock, no prescaling (DS, p. 48) */
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    init_lcd();
    init_rotary();
}

void showPlatforms() {
    topPlatform.left = 0;
    topPlatform.right = WIDTH;
    topPlatform.top = 0;
    topPlatform.bottom = PLATFORM_HEIGHT;

    bottomPlatform.left = 0;
    bottomPlatform.right = WIDTH;
    int height = HEIGHT;
    bottomPlatform.top= height-PLATFORM_HEIGHT;
    bottomPlatform.bottom = HEIGHT;
    
    fill_rectangle(topPlatform, GREEN);
    fill_rectangle(bottomPlatform, GREEN);
}

void showPlayer() {
    player.left = 1;
    player.right = player.left + size;
    player.bottom = HEIGHT - PLATFORM_HEIGHT;
    player.top = player.bottom - size;

    fill_rectangle(player, CYAN);
}

void setPlayerBottom() {
    // if (player.top != player.bottom - size) {
        fill_rectangle(player, BLACK);
    // }

    showPlayer();
}

void setPlayerTop() {
    // if (player.top != PLATFORM_HEIGHT) {
        fill_rectangle(player, BLACK);
    // }

    player.top = PLATFORM_HEIGHT;
    player.bottom = player.top + size;
    fill_rectangle(player, CYAN);
}

void showObstacles() {
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        if (i % 2 == 0) {
            obstacles[i].top = PLATFORM_HEIGHT;
            obstacles[i].bottom = 50 + PLATFORM_HEIGHT;
        } else {
            obstacles[i].top = HEIGHT - 50 + PLATFORM_HEIGHT;
            obstacles[i].bottom = HEIGHT - PLATFORM_HEIGHT;
        }

        obstacles[i].left = ((1 + i) * (WIDTH / 4)) - 1;
        obstacles[i].right = obstacles[i].left + OBSTACLE_WIDTH;

        fill_rectangle(obstacles[i], RED);
    }
    
}

void generateNextObstacle(int i) {
    if (i % 2 == 0) {
        obstacles[i].top = PLATFORM_HEIGHT;
        obstacles[i].bottom = 50 + PLATFORM_HEIGHT;
    } else {
        obstacles[i].top = HEIGHT - 50 + PLATFORM_HEIGHT;
        obstacles[i].bottom = HEIGHT - PLATFORM_HEIGHT;
    }
    
    obstacles[i].right = WIDTH + OBSTACLE_WIDTH;
    obstacles[i].left = WIDTH;

    fill_rectangle(obstacles[i], RED);
}

void scrollLoop(uint8_t isBtnClicked) {
    scrollCount += 1;

    // Check for collisions
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        if (player.right >= obstacles[i].left) {
            if (obstacles[i].top == PLATFORM_HEIGHT && player.top <= obstacles[i].bottom) {
                gameOver();
                return;
            } else if (obstacles[i].bottom == (HEIGHT - PLATFORM_HEIGHT) && player.bottom >= obstacles[i].top) {
                gameOver();
                return;
            }
        }
    }

    // Check for button click
    if (isBtnClicked && lastBtnClick < (scrollCount - 12)) {
        isGravityDown = isGravityDown ? 0 : 1;
        if (isGravityDown) {
            setPlayerBottom();
        } else {
            setPlayerTop();
        }

        lastBtnClick = scrollCount;
    }
    
    // Update obstacle positions
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
        if (obstacles[i].right <= 1) {
            rectangle emptyRect = obstacles[i];
            emptyRect.left = obstacles[i].right - 1;

            fill_rectangle(emptyRect, BLACK);
            fill_rectangle(obstacles[i], BLACK);

            generateNextObstacle(i);
        } else {
            rectangle emptyRect = obstacles[i];
            emptyRect.left = obstacles[i].right - 1;
                
            obstacles[i].left = obstacles[i].left - 1;
            obstacles[i].right = obstacles[i].right - 1;

            fill_rectangle(emptyRect, BLACK);
            fill_rectangle(obstacles[i], RED);
        }
    }

    _delay_ms(15);
    
}

void gameOver() {
    stopGame = 1;
    clear_screen();
    display_string("Game over!");
}