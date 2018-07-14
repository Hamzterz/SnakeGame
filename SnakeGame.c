/*	CAB202: Assignment
*	Nom Nom Snake
*
*	Aarond Dino, October 2016
*	Queensland University of Technology
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "graphics.h"
#include "sprite.h"
#include "cpu_speed.h"
#include "lcd.h"
#include "graphics.h"
#include "sprite.h"

/*Definition of Values*/
#define BYTES_PER_SNAKE 9
#define BYTES_PER_NOM 9
#define BYTES_PER_WALL 15

/*Direction Definitions*/
#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4

/*Sprites*/
unsigned char snake_bitmap[BYTES_PER_SNAKE]= {	
	0b11111111, 0b11111111, 0b11111111,
	0b11111111, 0b11111111, 0b11111111,
	0b11111111, 0b11111111, 0b11111111
	
};

unsigned char nom_bitmap[BYTES_PER_NOM]= {	
	0b11111111, 0b11111111, 0b11111111,
	0b11111111, 0b11111111, 0b11111111,
	0b11111111, 0b11111111, 0b11111111
	
};

unsigned char vertical_wall_bitmap[BYTES_PER_WALL]= {
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	0b11111111,
	
};

unsigned char horizontal_wall_bitmap[BYTES_PER_WALL]= {
	0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111, 0b11111111
	
};

/*Sprites*/
Sprite Snake[70];
Sprite Nom;
Sprite Wall[3];
/*Boolean Statements*/
bool game_over = false;
bool walls_on = false;

/*Declaring Global Variables*/
int lives = 3;
int score = 0;
int dx;
int dy;
int direction;
int counter = 1000;
int snake_length = 2;
int snake_x = LCD_X/2;
int snake_y = LCD_Y/2;
int nom_x = 60;
int nom_y = 40;
int wall_length = 15;
uint16_t adc_result1;
int adc1;

void HUD(void);
void start_screen(void);
void snake_setup(void);
void nom_setup(void);
void draw_snake(void);
void draw_nom(void);
void snake_controls(void);
void snake_process(void);
void boundary(void);
void nom_spawning(void);
void seppuku(void);
void flash(void);
bool sprites_collided(double sprite_x_1, double sprite_y_1, double sprite_x_2, double sprite_y_2);
bool sprites_collided2(double sprite_x_1, double sprite_y_1, double sprite_x_2, double sprite_y_2);
bool sprites_collided3(double sprite_x_1, double sprite_y_1, double sprite_x_2, double sprite_y_2);
void walls_controls(void);
void walls_process(void);	
void wall_setup(void);
void pancake(void);
void snake_dead(void);



void setup(void){
    set_clock_speed(CPU_8MHz);
    //initialise the LCD screen
    lcd_init(LCD_DEFAULT_CONTRAST);
    //clear any characters that were written on the screen
    clear_screen();
}

void init_adc(void){
    // AREF = AVcc
    ADMUX = (1<<REFS0);
 
    // ADC Enable and pre-scaler of 128
    // 8000000/128 = 62500
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
}

// read adc value
uint16_t adc_read(uint8_t ch){
    // select the corresponding channel 0~7
    // ANDing with '7' will always keep the value
    // of 'ch' between 0 and 7
    ch &= 0b00000111;  // AND operation with 7
    ADMUX = (ADMUX & 0xF8)|ch;     // clears the bottom 3 bits before ORing
 
    // start single conversion
    // write '1' to ADSC
    ADCSRA |= (1<<ADSC);
 
    // wait for conversion to complete
    // ADSC becomes '0' again
    // till then, run loop continuously
    while(ADCSRA & (1<<ADSC));
 
    return (ADC);
}

void adc_scrolling(void){
	adc_result1 = adc_read(0);
	adc1 = adc_result1;
	counter = adc1;
}

void hardware_init(void){
	//initialising the lcd screen as default
	lcd_init(LCD_DEFAULT_CONTRAST);
	//SETTING THE BUTTONS AS INPUTS (SWITCH0 AND SWITCH1)
	DDRB &= ~((1 << PINB7) | (1 << PINB1));
	DDRD &= ~((1 << PIND1) | (1 << PIND0));
	DDRF &= ~((1 << PINF5) | (1 << PINF6));
	// Configure TIMER1 in "normal" operation mode
    // TODO
	TCCR1B &= ~((1<<WGM12));
    // Set the prescaler for TIMER1 so that the clock overflows every ~8.3 seconds
    // TODO
	TCCR1B |= (1<<CS12) | (1<<CS10);
	
	init_adc();
}

int main(void) {
	setup();
	hardware_init();
	start_screen();
	snake_setup();
	nom_setup();
	wall_setup();
	
	while(!game_over){
		clear_screen();
		adc_scrolling();
		HUD();
		draw_snake();
		draw_nom();
		walls_process();
		nom_spawning();
		seppuku();
		snake_process();
		if(walls_on){
			pancake();
		}
		boundary();
		show_screen();
	}
	return 0;
}

void start_screen(void){
	clear_screen();
	draw_string(LCD_X/2-13, 35, "SNAKE");
	draw_string(15, 15, "Aarond Dino");
	draw_string(20, 25, "n9488383");
	/*left*/
	draw_line(0, 0, 0, 47);
	/*right*/
	draw_line(83, 0, 83, 47);
	/*top*/
	draw_line(0, 0, 83, 0);
	/*bottom*/
	draw_line(0, 47, 83, 47);
	show_screen();
	_delay_ms(2000);
	clear_screen();
}

void game_over_screen(void){
	clear_screen();
	draw_string(LCD_X/2-22, 20, "GAME OVER");
	show_screen();
}

void HUD(void){
	char buffer[LCD_X];
	sprintf(buffer, "%02d (%d)", score, lives);
	draw_string(0, 0, buffer);
	draw_line(0, 8, 83, 8);
	
}

void snake_setup(void){
	for(int i=0; i<snake_length; i++){
		init_sprite(&Snake[i], snake_x-1, snake_y-1, 3, 3, snake_bitmap);			
	}
}

void nom_setup(void){
	nom_x = rand() % +(LCD_X-1);
	nom_y = rand() % (LCD_Y-2-8)+8;
	init_sprite(&Nom, nom_x, nom_y, 3, 3, nom_bitmap);
}

void wall_setup(void){
	init_sprite(&Wall[0],0, 30, 15, 1, horizontal_wall_bitmap);
	init_sprite(&Wall[1], 40, 37, 1, 15, vertical_wall_bitmap);
	init_sprite(&Wall[2], 73, 20, 15, 1, horizontal_wall_bitmap);
	
}

void draw_snake(void){
	for(int i=0; i<snake_length; i++){
		draw_sprite(&Snake[i]);
	}
}

void draw_nom(void){
	draw_sprite(&Nom);
}

void snake_controls(void){
	if(PIND & 0b00000010 && direction != DOWN){
		direction = UP;
	}
	else if(PINB & 0b10000000 && direction != UP){
		direction = DOWN;
	}
	else if(PINB & 0b00000010 && direction != RIGHT){
		direction = LEFT;
	}
	else if(PIND & 0b00000001 && direction !=LEFT){
		direction = RIGHT;
	}
	else if(PIND & 0b00000010 && direction == DOWN){
		snake_dead();
	}
	else if(PINB & 0b10000000 && direction == UP){
		snake_dead();
	}
	else if(PINB & 0b00000010 && direction == RIGHT){
		snake_dead();
	}
	else if(PIND & 0b00000001 && direction == LEFT){
		snake_dead();
	}
}

void snake_process(void){
	Snake[0].dx = 3;
	Snake[0].dy = 3;
	dx = Snake[0].dx;
	dy = Snake[0].dy;
	if ( TCNT1 > counter){
		snake_controls();
		if(direction == UP || direction == DOWN || direction == LEFT || direction == RIGHT){
			for(int i=snake_length-1; i>0; i--){
				Snake[i].x = Snake[i-1].x;
			}
			for(int i=snake_length-1; i>0; i--){
				Snake[i].y = Snake[i-1].y;
			}
		}
		if(direction == UP){
				Snake[0].y -= dy;
		}
		if(direction == DOWN){
				Snake[0].y += dy;
		}
		if(direction == LEFT){
				Snake[0].x -= dx;
		}
		if( direction == RIGHT){
				Snake[0].x += dx;
		}
		TCNT1 = 0;	
	}
}

void boundary(void){
	for(int i = 0; i<snake_length; i++){
		if(Snake[i].x < 0){
			Snake[i].x = LCD_X-2;
		}
		if(Snake[i].x > LCD_X-2){
			Snake[i].x = 0;
		}
		if(Snake[i].y < 8){
			Snake[i].y = LCD_Y-2;
		}
		if(Snake[i].y > LCD_Y-2){
			Snake[i].y = 8;
		}	
	}
}

void walls_controls(void){
	if(PINF & 0b00100000 ){
		_delay_ms(50);
		if(!walls_on){
			Wall[0].x = 0;
			Wall[0].y = 30;
			Wall[1].x = rand() % (50)+18;
			Wall[1].y = 32;
			Wall[2].x = 68;
			Wall[2].y = rand() % (7)+25;
			walls_on = true;			
		}
	}
	if(PINF & 0b01000000 ){
		if(walls_on){
			walls_on = false;
		}
	}
}

void walls_process(void){
	walls_controls();
	if(walls_on){
		/*Wall from left*/
		init_sprite(&Wall[0],Wall[0].x, Wall[0].y, 15, 1, horizontal_wall_bitmap);
		draw_sprite(&Wall[0]);
		/*Wall from bottom*/
		init_sprite(&Wall[1], Wall[1].x, Wall[1].y, 1, 15, vertical_wall_bitmap);
		draw_sprite(&Wall[1]);
		/*Wall from right*/
		init_sprite(&Wall[2], Wall[2].x, Wall[2].y, 15, 1, horizontal_wall_bitmap);
		draw_sprite(&Wall[2]);	
	}
}

void nom_spawning(void){
	srand(TCNT1);
	if(sprites_collided(Snake[0].x, Snake[0].y, nom_x, nom_y)){
		nom_x = rand() % +(LCD_X-2);
		nom_y = rand() % (LCD_Y-2-8)+8;
		score++;
		if(walls_on){
			score++;
		}
		snake_length++;
		init_sprite(&Snake[snake_length-1], Snake[snake_length-1].x, Snake[snake_length-1].y, 3, 3, snake_bitmap);
		//draw_sprite(Snake);
	}
	init_sprite(&Nom, nom_x, nom_y, 3, 3, nom_bitmap);
	draw_snake();
	//draw_sprite(&Nom);
	show_screen();
}

void seppuku(void){
	for(int i = 2; i<snake_length; i++){
		if(sprites_collided(Snake[0].x, Snake[0].y, Snake[i].x, Snake[i].y)){
			snake_dead();
		}
	}
}

void pancake(void){
	if(sprites_collided2(Snake[0].x, Snake[0].y, Wall[0].x, Wall[0].y)||sprites_collided2(Snake[0].x, Snake[0].y, Wall[2].x, Wall[2].y)||sprites_collided3(Snake[0].x, Snake[0].y, Wall[1].x, Wall[1].y)){
		snake_dead();
	}
}

void snake_dead(void){
	if(lives>0){
		lives--;
		snake_length = 2;
		Snake[0].dx = 0;
		Snake[0].dy = 0;
		direction = 0;
		snake_setup();
		lcd_write(LCD_C, 0b00001101);
		_delay_ms(400);
		lcd_write(LCD_C, 0b00001100);
		_delay_ms(400);
		lcd_write(LCD_C, 0b00001101);
		_delay_ms(400);
		lcd_write(LCD_C, 0b00001100);
		_delay_ms(400);
		lcd_write(LCD_C, 0b00001101);;
		_delay_ms(400);
		lcd_write(LCD_C, 0b00001100);
	}
	if(lives == 0){
		game_over = true;
		game_over_screen();
	}
}

void flash(void){
	
}

bool sprites_collided(double sprite_x_1, double sprite_y_1, double sprite_x_2, double sprite_y_2) {
    int sprite_1_top = sprite_y_1,
        sprite_1_bottom = sprite_y_1 + 2,
        sprite_1_left = sprite_x_1,
        sprite_1_right = sprite_x_1 + 2;

    int sprite_2_top = sprite_y_2,
        sprite_2_bottom = sprite_y_2+2,
        sprite_2_left = sprite_x_2,
        sprite_2_right = sprite_x_2 + 2;

    return !(
        sprite_1_bottom < sprite_2_top
        || sprite_1_top > sprite_2_bottom
        || sprite_1_right < sprite_2_left
        || sprite_1_left > sprite_2_right
        );
}

bool sprites_collided2(double sprite_x_1, double sprite_y_1, double sprite_x_2, double sprite_y_2) {
    int sprite_1_top = sprite_y_1,
        sprite_1_bottom = sprite_y_1 + 2,
        sprite_1_left = sprite_x_1,
        sprite_1_right = sprite_x_1 + 2;

    int sprite_2_top = sprite_y_2,
        sprite_2_bottom = sprite_y_2,
        sprite_2_left = sprite_x_2,
        sprite_2_right = sprite_x_2 + 14;

    return !(
        sprite_1_bottom < sprite_2_top
        || sprite_1_top > sprite_2_bottom
        || sprite_1_right < sprite_2_left
        || sprite_1_left > sprite_2_right
        );
}

bool sprites_collided3(double sprite_x_1, double sprite_y_1, double sprite_x_2, double sprite_y_2) {
    int sprite_1_top = sprite_y_1,
        sprite_1_bottom = sprite_y_1 + 2,
        sprite_1_left = sprite_x_1,
        sprite_1_right = sprite_x_1 + 2;

    int sprite_2_top = sprite_y_2,
        sprite_2_bottom = sprite_y_2 + 14,
        sprite_2_left = sprite_x_2,
        sprite_2_right = sprite_x_2;

    return !(
        sprite_1_bottom < sprite_2_top
        || sprite_1_top > sprite_2_bottom
        || sprite_1_right < sprite_2_left
        || sprite_1_left > sprite_2_right
        );
}

