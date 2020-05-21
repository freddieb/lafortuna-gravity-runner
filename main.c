/* COMP2215: Task 02---MODEL ANSWER */
/* For La Fortuna board.            */

#define __DELAY_BACKWARD_COMPATIBLE__

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
uint8_t isGravityDown = 0;
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
void runGame(void);
void reset(void);
void movePlayer(void);

void main(void) {
    init();

    clear_screen();

    showPlatforms();
    showPlayer();
    showObstacles();

    while (1) {
        runGame();

        while (get_middle() != 1) {}

        reset();
        showPlatforms();
        showPlayer();
        showObstacles();
        runGame();
    }
}


void init(void) {
    /* 8MHz clock, no prescaling (DS, p. 48) */
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    init_lcd();
    init_rotary();
}

void reset() {
    clear_screen();

    scrollCount = 0;
    scrollOffset = 0;
    size = 10;
    isGravityDown = 0;
    lastBtnClick = 0;
    stopGame = 0;
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

    // Move player if required
    movePlayer();

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
    // Increase debounce counter as scroll iteration delay decreases
    if (isBtnClicked && scrollCount > 30 && lastBtnClick < (scrollCount - (12 + (0.03 * scrollCount)))) { 
        isGravityDown = isGravityDown ? 0 : 1;
        // if (isGravityDown) {
        //     setPlayerBottom();
        // } else {
        //     setPlayerTop();
        // }

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

    _delay_ms(15 - (0.01 * scrollCount));
    
}

void movePlayer() {
    int stepSize = 6;

    // Check if move required
    if (isGravityDown == 0 && player.bottom != (HEIGHT - PLATFORM_HEIGHT)) {
        // Should move down
        if (player.bottom + stepSize > HEIGHT - PLATFORM_HEIGHT) {
            stepSize = 1;//(player.bottom - (HEIGHT - PLATFORM_HEIGHT)) < 0 ? 0 : (player.bottom - (HEIGHT - PLATFORM_HEIGHT));
        }

        rectangle emptyRect = player;
        emptyRect.bottom = player.top + stepSize;
            
        player.bottom = player.bottom + stepSize;
        player.top = player.top + stepSize;

        fill_rectangle(emptyRect, BLACK);
        fill_rectangle(player, CYAN);
    } else if (isGravityDown == 1 && player.top != PLATFORM_HEIGHT) {
        // Should move up
        if (player.top - stepSize < PLATFORM_HEIGHT) {
            stepSize = 1;//player.top - PLATFORM_HEIGHT;
        }

        rectangle emptyRect = player;
        emptyRect.top = player.bottom - stepSize;
            
        player.bottom = player.bottom - stepSize;
        player.top = player.top - stepSize;

        fill_rectangle(emptyRect, BLACK);
        fill_rectangle(player, CYAN);
    }
}

void runGame() {
    while (!stopGame) {
        scrollLoop(get_middle());
    }
}

void gameOver() {
    stopGame = 1;
    clear_screen();
    
    display_string("\n\n\n\n");

    display_string("            _____                      \n");
    display_string("           / ____|                     \n");
    display_string("          | |  __  __ _ _ __ ___   ___ \n");
    display_string("          | | |_ |/ _` | '_ ` _ \\ / _ \\\n");
    display_string("          | |__| | (_| | | | | | |  __/\n");
    display_string("           \\_____|\\__,_|_| |_| |_|\\___|\n");

    display_string("\n\n");

    display_string("              ____                 _ \n");
    display_string("             / __ \\               | |\n");
    display_string("            | |  | |_   _____ _ __| | \n");
    display_string("            | |  | \\ \\ / / _ \\ '__| |\n");
    display_string("            | |__| |\\ V /  __/ |  |_|\n");
    display_string("             \\____/  \\_/ \\___|_|  (_)\n");

    display_string("\n\n\n");

    display_string("        Click the middle button to try again");
}
