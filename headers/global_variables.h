#ifndef GLOBAL_VARIABLES_H
#define GLOBAL_VARIABLES_H

//static variables
#define FPS 				60							//frames per seconds
#define WIN_ANIMATION_SPEED	1							//final animation speed
#define BG_COLOR 			makecol8(0, 0, 0)			//background color
#define RED 				makecol8(255, 0, 0)			//red color
#define WHITE				makecol8(255, 255, 255)		//white color
#define YELLOW				makecol8(255, 255, 0)		//yellow color
#define 		M_PI 3.14159265358979323846			

//dynamic variables
int		XWIN, YWIN;			//screen size
int		N_BALLS;			//number of balls
int		BET;				//game mode
int		PAUSE;				//pause
int		red, black;			//colors
int		yellow, white;		//colors
int		KILL_ALL;			//set true to kill all the threads and exit
int 	END;				//true if the game is finished
float	diag;				//diagonal length of the screen
float	VELOCITY;			//game velocity (1 is normal)
float	SCALE;				//scale the balls if the screen is larger/smaller
char	**amounts;			//balls' dimensions to be printed
char	**names;			//balls' names to be printed
FONT 	*font_menu;			//menu font
FONT	*font_game;			//game font

struct sync 	sem_balls;		//mutual exclusion for balls' array
struct sync 	sem_pause;		//mutual exclusion for PAUSE variable
struct sync 	sem_velocity;	//mutual exclusion for VELOCITY variable
struct sync 	sem_end;		//mutual exclusion for END variable
struct ball 	*balls;			//balls' array
struct BITMAP 	*buffer;		//buffer to write on the screen

#endif