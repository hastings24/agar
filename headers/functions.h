#ifndef FUNCTIONS_H
#define FUNCTIONS_H

//functions' definitions
void	sync_init(struct sync *s);
void	start_read(struct sync *s);
void	end_read(struct sync *s);
void	start_write(struct sync *s);
void	end_write(struct sync *s);

int		paus(int n);
void	restart();
void	drawCircle(float x, float y, int r, int c);
float	distance(float direction_x, float direction_y, int player);
void	timing(struct timeval *t1, struct timeval *t2, float *dt);

#endif