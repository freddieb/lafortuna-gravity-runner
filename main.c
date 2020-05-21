/* COMP2215: Task 02---MODEL ANSWER */
/* For La Fortuna board.            */

#define __DELAY_BACKWARD_COMPATIBLE__

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"
#include "rotary.h"
#include <time.h>
#include <stdlib.h>

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
void displayStartScreen(void);

void main(void) {
    init();

    displayStartScreen();
    while (get_middle() != 1) {}

    while (1) {
        clear_screen();
        reset();
        showPlatforms();
        showPlayer();
        showObstacles();
        runGame();

        while (get_middle() != 1) {}
    }
}


void init(void) {
    /* 8MHz clock, no prescaling (DS, p. 48) */
    CLKPR = (1 << CLKPCE);
    CLKPR = 0;

    init_lcd();
    init_rotary();
    
    srand(time(NULL));   
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
    topPlatform.bottom = PLATFORM_HEIGHT-1;

    bottomPlatform.left = 0;
    bottomPlatform.right = WIDTH;
    int height = HEIGHT;
    bottomPlatform.top= height-PLATFORM_HEIGHT+1;
    bottomPlatform.bottom = HEIGHT;
    
    fill_rectangle(topPlatform, WHITE);
    fill_rectangle(bottomPlatform, WHITE);
}

void showPlayer() {
    player.left = 1;
    player.right = player.left + size;
    player.bottom = HEIGHT - PLATFORM_HEIGHT;
    player.top = player.bottom - size;

    fill_rectangle(player, GREEN);
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
    fill_rectangle(player, GREEN);
}

void showObstacles() {
    for (int i = 0; i < OBSTACLE_COUNT; i++) {
       int r1 = 30 + (rand() % 70);

        if (i % 2 == 0) {
            obstacles[i].top = PLATFORM_HEIGHT;
            obstacles[i].bottom = r1 + PLATFORM_HEIGHT;
        } else {
            obstacles[i].top = HEIGHT - r1 + PLATFORM_HEIGHT;
            obstacles[i].bottom = HEIGHT - PLATFORM_HEIGHT;
        }

        int r2 = (rand() % 40) - 20;

        obstacles[i].left = ((1 + i) * (WIDTH / 4)) + r2;
        obstacles[i].right = obstacles[i].left + OBSTACLE_WIDTH;

        fill_rectangle(obstacles[i], WHITE);
    }
    
}

void generateNextObstacle(int i) {
    int r2 = 30 + (rand() % 70);

    if (i % 2 == 0) {
        obstacles[i].top = PLATFORM_HEIGHT;
        obstacles[i].bottom = r2 + PLATFORM_HEIGHT;
    } else {
        obstacles[i].top = HEIGHT - r2 + PLATFORM_HEIGHT;
        obstacles[i].bottom = HEIGHT - PLATFORM_HEIGHT;
    }
    
    int r1 = (rand() % 40);

    obstacles[i].right = r1 + WIDTH + OBSTACLE_WIDTH;
    obstacles[i].left = r1 + WIDTH;

    fill_rectangle(obstacles[i], WHITE);
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
            fill_rectangle(obstacles[i], WHITE);
        }
    }

    _delay_ms(15 - (0.01 * scrollCount)); // causes exponential speed increase
    
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
        fill_rectangle(player, GREEN);
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
        fill_rectangle(player, GREEN);
    }
}

void runGame() {
    while (stopGame != 1) {
        scrollLoop(get_middle());
    }
}

void gameOver() {
    stopGame = 1;
    clear_screen();

    display.foreground = GREEN;
    
    display_string("\n\n\n\n");

    display_string("             _____                      \n");
    display_string("            / ____|                     \n");
    display_string("           | |  __  __ _ _ __ ___   ___ \n");
    display_string("           | | |_ |/ _` | '_ ` _ \\ / _ \\\n");
    display_string("           | |__| | (_| | | | | | |  __/\n");
    display_string("            \\_____|\\__,_|_| |_| |_|\\___|\n");

    display_string("\n\n");

    display_string("               ____                 _ \n");
    display_string("              / __ \\               | |\n");
    display_string("             | |  | |_   _____ _ __| | \n");
    display_string("             | |  | \\ \\ / / _ \\ '__| |\n");
    display_string("             | |__| |\\ V /  __/ |  |_|\n");
    display_string("              \\____/  \\_/ \\___|_|  (_)\n");

    display_string("\n\n\n");

    display.foreground = WHITE;
    display_string("          Click the middle button to try again");
}

void displayStartScreen() {
    clear_screen();

    display.foreground = GREEN;
    
    display_string("\n\n\n\n");

    display_string("         _____                 _ _         \n");
    display_string("        / ____|               (_) |        \n");
    display_string("       | |  __ _ __ __ ___   ___| |_ _   _ \n");
    display_string("       | | |_ | '__/ _` \\ \\ / / | __| | | |\n");
    display_string("       | |__| | | | (_| |\\ V /| | |_| |_| |\n");
    display_string("        \\_____|_|  \\__,_| \\_/ |_|\\__|\\__, |\n");
    display_string("                                      __/ |\n");
    display_string("                                     |___/ \n");

    display_string("\n\n");

    display_string("          _____                             \n");
    display_string("         |  __ \\                            \n");
    display_string("         | |__) |   _ _ __  _ __   ___ _ __ \n");
    display_string("         |  _  / | | | '_ \\| '_ \\ / _ \\ '__|\n");
    display_string("         | | \\ \\ |_| | | | | | | |  __/ |   \n");
    display_string("         |_|  \\_\\__,_|_| |_|_| |_|\\___|_|   \n");

    display_string("\n\n\n");

    display.foreground = WHITE;
    display_string("         Click the middle button to start");
}

                                  