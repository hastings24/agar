#include "./headers/libraries.h"
#include "./headers/structures.h"
#include "./headers/global_variables.h"
#include "./headers/functions.h"

//------------------------------------------//
//  initializes a syncronization structure  //
//------------------------------------------//
void	sync_init(struct sync *s)
{
	s->nbw = 0;
	s->nbr = 0;
	s->nr = 0;
	s->nw = 0;

	sem_init(&s->priv_r, TRUE, 0);
	sem_init(&s->priv_w, TRUE, 0);
	sem_init(&s->m, TRUE, 1);
}

//--------------------------------//
//  starts reading of a resource  //
//--------------------------------//
void	start_read(struct sync *s)
{
	sem_wait(&s->m);

	if (s->nw > 0 || s->nbw > 0) {
		s->nbr++;
	}
	else {
		s->nr++;
		sem_post(&s->priv_r);
	}

	sem_post(&s->m);
	sem_wait(&s->priv_r);
}

//------------------------------//
//  ends reading of a resource  //
//------------------------------//
void	end_read(struct sync *s)
{
	sem_wait(&s->m);
	s->nr--;

	if (s->nbw > 0 && s->nr == 0) {
		s->nbw--;
		s->nw++;
		sem_post(&s->priv_w);
	}

	sem_post(&s->m);
}

//--------------------------------//
//  starts writing to a resource  //
//--------------------------------//
void	start_write(struct sync *s)
{
	sem_wait(&s->m);

	if (s->nr > 0 || s->nw > 0)	{
		s->nbw++;
	}
	else {
		s->nw++;
		sem_post(&s->priv_w);
	}

	sem_post(&s->m);
	sem_wait(&s->priv_w);
}

//------------------------------//
//  ends writing to a resource  //
//------------------------------//
void	end_write(struct sync *s)
{
	sem_wait(&s->m);
	s->nw--;

	if (s->nbr > 0)	{
		while (s->nbr > 0) {
			s->nbr--;
			s->nr++;
			sem_post(&s->priv_r);
		}
	}
	else if (s->nbw > 0) {
		s->nbw--;
		s->nw++;
		sem_post(&s->priv_w);
	}

	sem_post(&s->m);
}

//--------------------------------//
//  draws a circle on the buffer  //
//--------------------------------//
void	drawCircle(float x, float y, int r, int c)
{
	acquire_bitmap(buffer);
	circlefill(buffer, x, y, r, c);
	release_bitmap(buffer);
}

//---------------------------------------//
//  returns distance between the center  //
//  of a ball and a position             //
//---------------------------------------//
float	distance(float position_x, float position_y, int index)
{
	return sqrt((position_x - balls[index].px) 
	* (position_x - balls[index].px)
	+ (position_y - balls[index].py) 
	* (position_y - balls[index].py));
}

//--------------------------------------//
//  returns distance between two balls  //
//  (zero if they are overlapped)       //
//--------------------------------------//
float	overlap_distance(int ball1, int ball2)
{
	if (ball1 > -1 && ball2 > -1) {
		if (balls[ball1].r < balls[ball2].r) {
			return sqrt((balls[ball1].px - balls[ball2].px)
			* (balls[ball1].px - balls[ball2].px) 
			+ (balls[ball1].py - balls[ball2].py)
			* (balls[ball1].py - balls[ball2].py)) 
			+ balls[ball1].r - balls[ball2].r;
		}
		else {
			return sqrt((balls[ball1].px - balls[ball2].px)
			* (balls[ball1].px - balls[ball2].px) 
			+ (balls[ball1].py - balls[ball2].py)
			* (balls[ball1].py - balls[ball2].py)) 
			+ balls[ball2].r - balls[ball1].r;
		}
	}
	else {
		return 10000.0;
	}
}

//--------------------------------------//
//  returns distance between two balls  //
//  (zero if they are colliding)        //
//--------------------------------------//
float	collision_distance(int ball1, int ball2)
{
	if (ball1 > -1 && ball2 > -1) {
		return sqrt((balls[ball1].px - balls[ball2].px) 
		* (balls[ball1].px - balls[ball2].px) 
		+ (balls[ball1].py - balls[ball2].py) 
		* (balls[ball1].py - balls[ball2].py))
		- (balls[ball1].r + balls[ball2].r) * 3 / 5;
	}
	else {
		return 10000.0;
	}
}

//-----------------------------------------------//
//  writes an integer in ASCII to a given char*  //
//  N.B: string must be previously allocated     //
//-----------------------------------------------//
void	itos(char *string, int n)
{
	int	tmp;
	int	i, j;

	for (i = 11; i > 0; i--) {
		if (n / (int)(pow(10, i))) {
			for (j = 0; i > 0; j++, i--) {
				tmp = n / (int)(pow(10, i));
				string[j] = (char)((n / (int)(pow(10, i)) % 10) + 48);
			}
			string[j] = 0;
			return;
		}
	}
}

//----------------------------------------//
//  returns the indexes of the two balls  //
//  nearest to the given index            //
//----------------------------------------//
int	nearest(int index)
{
	int		near = -1, near2 = -1;
	int		dist = 10000000;
	float	d;
	int 	i;

	for (i = 0; i < N_BALLS; i++) {
		d = collision_distance(i, index);

		if (d < dist && index != i && balls[i].alive) {
			dist = d;
			near = i;
		}
	}

	dist = 10000000;

	for (i = 0; i < N_BALLS; i++) {
		d = collision_distance(i, index);

		if (i != near && d < dist && index != i && balls[i].alive) {
			dist = d;
			near2 = i;
		}
	}

	return near2 * 128 + near;
}

//------------------------------------------------//
//  checks if the given ball is in a corner area  //
//  and tries to avoid to be stuck there          //
//------------------------------------------------//
void	check_corner(int *index, float *angle, float *dist, int *corner)
{
	if (balls[*index].py + balls[*index].px 
		- YWIN + XWIN / 3 - balls[*index].r * 2 < 0)
		*corner = 1;

	if (balls[*index].py - balls[*index].px + XWIN 
		- YWIN + XWIN / 3 - balls[*index].r * 2 < 0)
		*corner = 2;

	if (balls[*index].py + balls[*index].px - XWIN 
		- XWIN / 3 + balls[*index].r * 2 > 0)
		*corner = 3;
	
	if (balls[*index].py - balls[*index].px 
		- XWIN / 3 + balls[*index].r * 2 > 0)
		*corner = 4;

	if (*corner) {
		*dist = ABS(diag / 2 - balls[*index].r * 2 
				- distance(XWIN / 2, YWIN / 2, *index));
		*angle = atanf((balls[*index].py - YWIN / 2) 
					/ (XWIN / 2 - balls[*index].px));

		if (*corner == 2 || *corner == 3) {
			*angle += M_PI;
		}
	}
}

//----------------------------------------------------------//
//  checks if the given ball is in a border area and tries  //
//  to avoid to be stuck there (overwrites check_corner)    //
//----------------------------------------------------------//
void	check_border(int *index, float *angle, float *dist, int *near)
{
	if (balls[*index].px < YWIN * 3 / 9 
		|| balls[*index].px > XWIN - YWIN * 3 / 9
		|| balls[*index].py < YWIN * 2 / 9 
		|| balls[*index].py > YWIN - YWIN * 2 / 9)
	{
		if (balls[*index].px < *dist) {
			*dist = balls[*index].px;
			*near = -2;
		}

		if (XWIN - balls[*index].px < *dist) {
			*dist = XWIN - balls[*index].px;
			*near = -4;
		}

		if (balls[*index].py < *dist) {
			*dist = balls[*index].py;
			*near = -3;
		}

		if (YWIN - balls[*index].py < *dist) {
			*dist = YWIN - balls[*index].py;
			*near = -5;
		}

		if (*near < -1)	{
			*angle = atanf((balls[*index].py - YWIN / 2) 
						/ (XWIN / 2 - balls[*index].px));
			if (XWIN / 2 < balls[*index].px)
				*angle += M_PI;
		}
	}
}

//---------------------------//
//  assignes first distance  //
//---------------------------//
int	assign_dist1(int *index, int *near, float *dist, float *dist2)
{
	if (*near > -1) {
		*dist = collision_distance(*index, *near);
		return TRUE;
	}
	else if (*near == -1) {
		*dist2 = 0;
		*dist = 10000;
		return FALSE;
	}

}

//----------------------------//
//  assignes second distance  //
//----------------------------//
void	assign_dist2(int *index, int *near2, float *dist2, int *corner)
{
	if (*near2 > -1 && !(*corner)) {
		*dist2 = collision_distance(*index, *near2);
	}
	else if (!(*corner)) {
		*dist2 = 10000;
	}
}

//-----------------------------------------------//
//  modifies angles not to invert the direction  //
//-----------------------------------------------//
void	adjust_angles(int *near, int* near2, float *angle, 
					float *angle2, int* index, int *corner)
{
	if (balls[*near].px > balls[*index].px) {
		*angle += M_PI;
	}

	if (balls[*near2].px > balls[*index].px && *near2 > -1 && !*corner) {
		*angle2 += M_PI;
	}

	if (ABS(*angle - *angle2) > M_PI) {
		if (*angle > *angle2)
			*angle2 += 2 * M_PI;
		else
			*angle += 2 * M_PI;
	}
}

//----------------------------------------------//
//  prevent the ball to escape from the screen  //
//----------------------------------------------//
void	hold_ball(int *index)
{
	if (balls[*index].px < 0) {
		balls[*index].px = 0;
	}

	if (balls[*index].px > XWIN) {
		balls[*index].px = XWIN;
	}

	if (balls[*index].py < 0) {
		balls[*index].py = 0;
	}

	if (balls[*index].py > YWIN) {
		balls[*index].py = YWIN;
	}
}

//--------------------------//
//  escapes from two balls  //
//--------------------------//
void	escape(int index, int near, int near2, float dt)
{
	float	dist = -1, dist2 = -1, v, angle, angle2;
	int		corner = 0;

	if (!assign_dist1(&index, &near, &dist, &dist2))
		return;

	check_corner(&index, &angle2, &dist2, &corner);

	assign_dist2(&index, &near2, &dist2, &corner);

	start_read(&sem_velocity);
	v = VELOCITY * 4 / balls[index].r;
	end_read(&sem_velocity);

	angle = atanf((balls[near].py - balls[index].py) 
			/ (balls[index].px - balls[near].px));

	check_border(&index, &angle2, &dist2, &near2);

	if (near2 > -1 && !corner) {
		angle2 = atanf((balls[near2].py - balls[index].py) 
					/ (balls[index].px - balls[near2].px));
	}

	adjust_angles(&near, &near2, &angle, &angle2, &index, &corner);	

	if (dist || dist2) {
		balls[index].angle = (angle * dist2 * dist2 + angle2 * dist * dist)
							 / (dist * dist + dist2 * dist2);
	}

	balls[index].px += v * dt * cosf(balls[index].angle);
	balls[index].py -= v * dt * sinf(balls[index].angle);

	hold_ball(&index);
}

//------------------------------//
//  follows a certain position  //
//------------------------------//
void follow(float dt, float position_x, float position_y, int index)
{
	float	v;

	balls[index].angle = atanf((balls[index].py - position_y) 
						/ (position_x - balls[index].px));

	if (position_x < balls[index].px)
		balls[index].angle += M_PI;

	start_read(&sem_velocity);
	v = VELOCITY * 4 / balls[index].r;
	end_read(&sem_velocity);

	if (distance(position_x, position_y, index) > 1) {
		balls[index].px += v * dt * cosf(balls[index].angle);
		balls[index].py -= v * dt * sinf(balls[index].angle);
	}
}

//--------------------------------------------------//
//  follows a ball while escaping from another one  //
//--------------------------------------------------//
void	follow_escape(int index, int follow, int escape, float dt)
{
	float	dist = -1, dist2 = -1, v, angle, angle2;
	int		corner = 0;
	float	dist_follow = collision_distance(index, follow);
	float	dist_escape = collision_distance(index, escape);

	dist = dist_follow;
	if (dist_follow > dist_escape * 3)
		check_corner(&index, &angle, &dist, &corner);

	check_border(&index, &angle, &dist, &follow);

	if (follow > -1 && !corner) {
		angle = atanf((balls[index].py - balls[follow].py)
				/ (balls[follow].px - balls[index].px)) + M_PI;
	}
	
	dist2 = dist_escape;
	angle2 = atanf((balls[escape].py - balls[index].py)
				/ (balls[index].px - balls[escape].px));
	
	if (dist2 > balls[index].r * 4 || dist < dist2 && follow > -1)
		dist = 0;

	start_read(&sem_velocity);
	v = VELOCITY * 4 / balls[index].r;
	end_read(&sem_velocity);

	adjust_angles(&escape, &follow, &angle2, &angle, &index, &corner);

	balls[index].angle = (angle * dist2 * dist2 + angle2 * dist * dist)
						/ (dist * dist + dist2 * dist2);

	balls[index].px += v * dt * cosf(balls[index].angle);
	balls[index].py -= v * dt * sinf(balls[index].angle);

	hold_ball(&index);
}

//----------------------------------------//
//  draws a rotated triangle on a buffer  //
//----------------------------------------//
void	rotateTriangle(BITMAP *buffer, int x, int y, 
				int color, size_t size, float angle)
{
	triangle(buffer,
			x + size * cosf(angle), y - size * sinf(angle),
			x + size * cosf(angle + M_PI / 3 * 4), 
			y - size * sinf(angle + M_PI / 3 * 4),
			x + size * cosf(angle + M_PI / 3 * 2), 
			y - size * sinf(angle + M_PI / 3 * 2),
			color);
}

//-----------------------------------------//
//  updates balls' data after a collision  //
//-----------------------------------------//
void	collision_update(int *i, int *j)
{
	start_read(&sem_end);
	if (collision_distance(*i, *j) < 0 && !END 
		|| END && overlap_distance(*i, *j) < 0)
	{
		if (!END) {
			end_read(&sem_end);
			balls[*i].amount += balls[*j].amount;
			balls[*i].r = sqrt(balls[*i].amount / M_PI);
		}
		else
			end_read(&sem_end);
			
		balls[*j].alive = FALSE;
		itos(amounts[*i], balls[*i].amount);
	}
	else
		end_read(&sem_end);
}

//---------------------------------------------------------//
//  checks if there have been collisions and updates data  //
//---------------------------------------------------------//
void	collision()
{
	for (int i = 0; i < N_BALLS - 1; i++)
	{
		for (int j = i + 1; j < N_BALLS; j++)
		{
			if (balls[i].alive && balls[j].alive)
			{
				if (balls[i].r > balls[j].r) {
					collision_update(&i, &j);
				}
				else {
					collision_update(&j, &i);
				}
			}
		}
	}
}

//--------------------------------------------------------//
//  checks if the current ball collides with another one  //
//--------------------------------------------------------//
int	check_collision(int i)
{
	for (int j = 0; j < i; j++) {
		if (collision_distance(i, j) < 0)
			return TRUE;
	}
	return FALSE;
}

//------------------------------------//
//  checks if a ball is greater than  //
//	the sum of all the others         //
//------------------------------------//
void	win(int index, float *total_amount)
{
	if (balls[index].amount > *total_amount / 2) {
		balls[index].amount += WIN_ANIMATION_SPEED * 7000 * SCALE;
		itos(amounts[index], balls[index].amount);
		balls[index].r = sqrt(balls[index].amount / M_PI);

		start_read(&sem_end);
		if (!END) {
			end_read(&sem_end);
			
			start_write(&sem_end);
			END = TRUE;
			end_write(&sem_end);
		}
		else
			end_read(&sem_end);
	}
}

//------------------------------------------//
//  initializes parameters of a given ball  //
//------------------------------------------//
void	init_ball(int index)
{
	balls[index].px = rand() % XWIN;
	balls[index].py = rand() % YWIN;
	balls[index].r = 50 * SCALE + ((float)(rand() % 3000)
					/ (float)((rand() % 3000) + 1000));
	balls[index].color = index % 15 + 1;
	while (balls[index].color == 4)
		balls[index].color = ABS(rand()) % 15 + 1;
	balls[index].alive = TRUE;
	balls[index].amount = balls[index].r * balls[index].r * M_PI;
}

//--------------------//
//  user ball's task  //
//--------------------//
void	ball_player_task()
{
	float	dt = (float)ptask_get_period(0, 1);

	while (!KILL_ALL) {
		ptask_wait_for_period();

		start_read(&sem_pause);
		if (!PAUSE)	{
			end_read(&sem_pause);

			start_write(&sem_balls);
			if (!balls[0].alive || BET)	{
				end_write(&sem_balls);
				continue;
			}
			follow(dt, mouse_x, mouse_y, 0);
			end_write(&sem_balls);
		}
		else
			end_read(&sem_pause);
	}
}

//-------------------------------------------//
//  find and assigns nearest balls' indexes  //
//-------------------------------------------//
void	find_nears(int i, int *near, int *near2)
{
	*near = nearest(i);
	*near2 = nearbyintf((float)(*near) / (float)128);
	*near = *near - (*near2 * 128);
}

//----------------------------------------------//
//  defines and applies what the ball is doing  //
//----------------------------------------------//
void	behaviour(int i, int near, int near2, float dt)
{
	if (near2 != -1)
		if (balls[near].r < balls[i].r)
			if (balls[near2].r < balls[i].r)
				follow(dt, balls[near].px, balls[near].py, i);
			else
				follow_escape(i, near, near2, dt);
		else if (balls[near2].r < balls[i].r)
			follow_escape(i, near2, near, dt);
		else
			escape(i, near, near2, dt);
	else if (balls[near].r < balls[i].r)
		follow(dt, balls[near].px, balls[near].py, i);
	else
		escape(i, near, near2, dt);
}

//-----------------------//
//  generic ball's task  //
//-----------------------//
void	ball_task()
{
	int		i = ptask_get_index(), near, near2;
	float	dt = (float)ptask_get_period(i, 1);

	while (!KILL_ALL) {
		ptask_wait_for_period();
		
		start_read(&sem_pause);
		if (!PAUSE) {
			end_read(&sem_pause);

			start_read(&sem_balls);
			if (!balls[i].alive) {
				end_read(&sem_balls);
				continue;
			}
			find_nears(i, &near, &near2);
			end_read(&sem_balls);

			start_write(&sem_balls);
			behaviour(i, near, near2, dt);
			end_write(&sem_balls);
		}
		else
			end_read(&sem_pause);
	}
}

//-------------------------------------//
//  draws all the balls on the buffer  //
//-------------------------------------//
void	draw_balls()
{
	int	i;
	
	for (i = 1; i < N_BALLS; i++) {
		if (balls[i].alive) {
			drawCircle(balls[i].px, balls[i].py, 
					balls[i].r, balls[i].color);
			textout_centre_ex(buffer, font_game, amounts[i], balls[i].px,
						 balls[i].py - 5 + 10 * (-BET), BG_COLOR, -1);
			if (BET)
				textout_centre_ex(buffer, font_game, names[i], 
				balls[i].px, balls[i].py - 20, BG_COLOR, -1);
		}
	}
	if (balls[0].alive && !BET)	{
		drawCircle(balls[0].px, balls[0].py, balls[0].r, balls[0].color);
		rotateTriangle(buffer, mouse_x, mouse_y, RED,
						30 * SCALE, balls[0].angle);
		textout_centre_ex(buffer, font_game, amounts[0], balls[0].px,
							balls[0].py - 5, BG_COLOR, -1);
	}
}

//----------------------------------------//
//  draws all the messages on the buffer  //
//----------------------------------------//
void	display_messages(float *total_amount)
{
	int	i, j;

	for (i = 0; i < N_BALLS; i++) {
		if (balls[i].amount > *total_amount / 2) {
			start_read(&sem_end);
			if (!END) {
				end_read(&sem_end);
				textout_centre_ex(buffer, font_menu, "Press " 
				"X to explode", XWIN / 2, YWIN / 10, WHITE, -1);
			}
			else if (BET) {
				end_read(&sem_end);
				textout_centre_ex(buffer, font_menu, "The "
				"winner is:", XWIN / 2, YWIN / 10, WHITE, -1);
				textout_centre_ex(buffer, font_menu, names[i],
				XWIN / 2 + 200, YWIN / 10 + 2, WHITE, -1);
				textout_centre_ex(buffer, font_game, "Press ESC "
				"for options", XWIN / 2, YWIN / 10 + 70, WHITE, -1);
			}
			else {
				end_read(&sem_end);
				if (i) {
					textout_centre_ex(buffer, font_menu, "You "
					"lost", XWIN / 2, YWIN / 10, WHITE, -1);
					textout_centre_ex(buffer, font_game, "Press "
					"ESC for options", XWIN / 2, YWIN / 10 + 70, WHITE, -1);
				}
				else {
					textout_centre_ex(buffer, font_menu, "You won",
								XWIN / 2, YWIN / 10, WHITE, -1);
					textout_centre_ex(buffer, font_game, "Press ESC "
					"for options", XWIN / 2, YWIN / 10 + 70, WHITE, -1);
				}
			}
		}
	}
}

//------------------------------------------//
//  task that draws anything on the buffer  //
//------------------------------------------//
void	display_task()
{
	int		i;
	float	total_amount;

	while (!KILL_ALL) {

		start_read(&sem_pause);
		if (!PAUSE) {
			end_read(&sem_pause);

			clear_to_color(buffer, BG_COLOR);
			start_read(&sem_balls);
			draw_balls();
			total_amount = 0;
			for (i = 0; i < N_BALLS; i++) {
				if (balls[i].alive)
					total_amount += balls[i].amount;
			}
			display_messages(&total_amount);
			end_read(&sem_balls);

			rect(buffer, 0, 0, XWIN - 1, YWIN - 1, WHITE);
			draw_sprite(screen, buffer, 0, 0);
		}
		else
			end_read(&sem_pause);

		ptask_wait_for_period();
	}
}

//------------------------------------------------//
//  task that checks collisions and updates data  //
//------------------------------------------------//
void	collision_task()
{
	while (!KILL_ALL) {

		start_read(&sem_pause);
		if (!PAUSE)
		{
			end_read(&sem_pause);

			start_write(&sem_balls);
			collision();
			end_write(&sem_balls);
		}
		else
			end_read(&sem_pause);

		ptask_wait_for_period();
	}
}

//------------------//
//  plays the game  //
//------------------//
void	init_game(int bet)
{
	balls = malloc(N_BALLS * sizeof(struct ball));
	amounts = malloc(N_BALLS * sizeof(int *));
	names = malloc(N_BALLS * sizeof(int *));

	sync_init(&sem_balls);
	sync_init(&sem_pause);
	sync_init(&sem_end);
	sync_init(&sem_velocity);

	for (int i = 0; i < N_BALLS; i++) {
		amounts[i] = malloc(11);
		names[i] = malloc(5);
		init_ball(i);
		sprintf(amounts[i], "%d", i);
		sprintf(names[i], "%d", i);
		while (check_collision(i))
			init_ball(i);
	}

	balls[0].color = RED;
	balls[0].r = 53 * SCALE;
	balls[0].amount = balls[0].r * balls[0].r * M_PI;

	for (int i = 0; i < N_BALLS; i++) {
		itos(amounts[i], balls[i].amount);
	}

	if (bet) {
		balls[0].alive = FALSE;
		BET = TRUE;
	}
}

//----------------------------------------//
//  shows balls' positions to the player  //
//  before the game starts                //
//----------------------------------------//
void	show_balls(struct timeval *t1, struct timeval *t2, float *dt)
{
	float	init_px = balls[0].px;
	float	init_py = balls[0].py;
	int		i;

	position_mouse(balls[0].px, balls[0].py);

	gettimeofday(t1, NULL);
	while (!key[KEY_SPACE]) {
		timing(t1, t2, dt);
		if (*dt > (float)1000 / (float)FPS)
		{
			gettimeofday(t1, NULL);
			clear_to_color(buffer, BG_COLOR);
			balls[0].px = init_px;
			balls[0].py = init_py;
			draw_balls();
			textout_centre_ex(buffer, font_menu, "Press SPACE to start", 
									XWIN / 2, YWIN / 2, WHITE, -1);
			rect(buffer, 0, 0, XWIN - 1, YWIN - 1, WHITE);
			follow(*dt, mouse_x, mouse_y, 0);
			draw_sprite(screen, buffer, 0, 0);
		}
	}
}

//------------------------------------//
//  create a task for each ball, the  //
//  collision and the display task    //
//------------------------------------//
void	create_tasks()
{
	int i;
	
	ptask_create_prio(ball_player_task, 1000 / FPS, 80, 1);

	for (i = 1; i < N_BALLS; i++) {
		ptask_create_prio(ball_task, 1000 / FPS, 80, 1);
	}
	ptask_create_prio(collision_task, 1000 / FPS, 80, 1);
	ptask_create_prio(display_task, 1000 / FPS, 80, 1);
}

//-------------------------------//
//  checks if there is a winner  //
//-------------------------------//
void	check_win(float *total_amount, int *x)
{
	int i;

	*total_amount = 0;
	for (int i = 0; i < N_BALLS; i++) {
		if (balls[i].alive)
			*total_amount += balls[i].amount;
	}
	for (i = 0; i < N_BALLS; i++)
		if (*x)
			win(i, total_amount);

	start_read(&sem_end);
	if (!END) {
		end_read(&sem_end);
		*x = FALSE;
	}
	else
		end_read(&sem_end);
}

//-----------------------------------------//
//  while other tasks are running, manage  //
//  keyboard input and winning status      //
//-----------------------------------------//
void	controller(int *x, float *total_amount)
{
	if (key[KEY_RIGHT]) {
		start_write(&sem_velocity);
		VELOCITY += 0.02;
		end_write(&sem_velocity);
	}
	if (key[KEY_LEFT]) {
		start_read(&sem_velocity);
		if (VELOCITY > 0.02) {
			end_read(&sem_velocity);

			start_write(&sem_velocity);
			VELOCITY -= 0.02;
			end_write(&sem_velocity);
		}
		else
			end_read(&sem_velocity);
	}
	if (key[KEY_X])
		*x = TRUE;
	
	check_win(total_amount, x);
}

//************************************************//
//  closes the program returning a certain value  //
//************************************************//
void	close_all(int return_value)
{
	destroy_font(font_menu);
	destroy_font(font_game);
	destroy_bitmap(buffer);
	allegro_exit();
	free(balls);
	exit(return_value);
}

//************************************//
//  resets parameters to resume game  //
//************************************//
void	reset_from_pause(int *x, int p)
{
	start_read(&sem_end);
	if (END) {
		end_read(&sem_end);

		start_write(&sem_velocity);
		VELOCITY = ((float)XWIN * (float)YWIN) 
				/ ((float)1366 * (float)768);
		end_write(&sem_velocity);

		start_write(&sem_end);
		END = FALSE;
		end_write(&sem_end);
	}
	else
		end_read(&sem_end);

	if(p)
		*x = FALSE;

	start_write(&sem_pause);
	PAUSE = FALSE;
	end_write(&sem_pause);
}

//*****************************************************//
//  computes how_much time passed from the last check  //
//*****************************************************//
void	timing(struct timeval *t1, struct timeval *t2, float *dt)
{
	gettimeofday(t2, NULL);
	*dt = (t2->tv_sec - t1->tv_sec) * 1000.0;
	*dt += (t2->tv_usec - t1->tv_usec) / 1000.0;
}

//***********************************//
//  frees allocated memory and exit  //
//***********************************//
void	free_and_close()
{
	start_read(&sem_balls);
	for (int i = 0; i < N_BALLS; i++) {
		free(amounts[i]);
		free(names[i]);
	}

	KILL_ALL = TRUE;
	end_read(&sem_balls);
	free(amounts);
	free(names);
	close_all(0);
}

//********************************//
//  manage play and pause status  //
//********************************//
void	game_pause_cycle(struct timeval *t1, struct timeval *t2,
					int x, float dt, float *total_amount)
{
	int p = 0;

	while (TRUE) {
		start_read(&sem_end);
		if (p == 1 && END) {
			end_read(&sem_end);			
			show_balls(t1, t2, &dt);
		}
		else
			end_read(&sem_end);

		reset_from_pause(&x, p);
		while (!key[KEY_ESC]) {
			timing(t1, t2, &dt);

			if (dt > (float)1000 / (float)FPS) {
				gettimeofday(t1, NULL);
				controller(&x, total_amount);
			}
		}
		p = paus(3);

		if (p == 1)
			restart();

		if (p == 2)
			free_and_close();
	}
}

//******************//
//  plays the game  //
//******************//
int	game(int nballs, int bet)
{
	float 			dt = 0, total_amount;
	struct timeval 	t1, t2;
	int 			x = 0, i;

	END = FALSE;
	N_BALLS = nballs;

	clear_to_color(screen, BG_COLOR);
	init_game(bet);
	show_balls(&t1, &t2, &dt);
	create_tasks();

	game_pause_cycle(&t1, &t2, x, dt, &total_amount);

	start_read(&sem_balls);
	for (int i = 0; i < N_BALLS; i++) {
		free(amounts[i]);
		free(names[i]);
	}
	end_read(&sem_balls);

	KILL_ALL = TRUE;
	free(amounts);
	free(names);

	return 0;
}

//***********************************************************//
//  checks if the arguments inserted from by user are valid  //
//***********************************************************//
int check_arguments(int argc, char **argv)
{
	if (argc < 3) {
		printf("Error: not enough arguments.\n");
		printf("Usage: sudo ./agar WIDTH HEIGHT W(optional: window mode)\n");
		return FALSE;
	}
	if (argc > 4) {
		printf("Error: too many arguments.\n");
		printf("Usage: sudo ./agar WIDTH HEIGHT W(optional: window mode)\n");
		return FALSE;
	}
	if (!atoi(argv[1])) {
		printf("Error: first argument is not an integer.\n");
		printf("Usage: sudo ./agar WIDTH HEIGHT W(optional: window mode)\n");
		return FALSE;
	}
	if (!atoi(argv[2])) {
		printf("Error: second argument is not an integer.\n");
		printf("Usage: sudo ./agar WIDTH HEIGHT W(optional: window mode)\n");
		return FALSE;
	}
	if (argc == 4) {
		if (*argv[3] != 'w' && *argv[3] != 'W') {
			printf("Error: third argument must be {w, W}.\n");
			printf("Usage: sudo ./agar WIDTH HEIGHT W(optional: window mode)\n");
			return FALSE;
		}
	}
	return TRUE;
}

//*****************************//
//  initializes the main menu  //
//*****************************//
void	initialize_menu(int *colors, int *select, int prev_select, int submenu)
{
	int i = 0;

	for (int i = 0; i < 13; i++) {
		colors[i] = WHITE;
	}
	if (submenu) {
		*select = 0;
		colors[3] = YELLOW;
	}
	colors[prev_select] = YELLOW;

	rectfill(buffer, 0, 0, XWIN, YWIN, BG_COLOR);
}

//***************************************//
//  displays the words of the main menu  //
//***************************************//
void	draw_menu_levels(int* colors)
{
	textout_centre_ex(buffer, font_menu, "Level 1", XWIN / 2, 
						YWIN / 5 + 0 * 60, colors[3], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 2", XWIN / 2, 
						YWIN / 5 + 1 * 60, colors[4], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 3", XWIN / 2, 
						YWIN / 5 + 2 * 60, colors[5], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 4", XWIN / 2, 
						YWIN / 5 + 3 * 60, colors[6], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 5", XWIN / 2, 
						YWIN / 5 + 4 * 60, colors[7], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 6", XWIN / 4 * 3, 
						YWIN / 5 + 0 * 60, colors[8], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 7", XWIN / 4 * 3, 
						YWIN / 5 + 1 * 60, colors[9], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 8", XWIN / 4 * 3, 
						YWIN / 5 + 2 * 60, colors[10], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 9", XWIN / 4 * 3, 
						YWIN / 5 + 3 * 60, colors[11], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Level 10", XWIN / 4 * 3, 
						YWIN / 5 + 4 * 60, colors[12], BG_COLOR);
}

//***************************************//
//  displays the words of the main menu  //
//***************************************//
void	draw_menu_commands(int *colors)
{	
	textout_centre_ex(buffer, font_menu, "Menu", XWIN / 15,
						YWIN / 80, YELLOW, BG_COLOR);
						
	textout_centre_ex(buffer, font_menu, "Play", XWIN / 5, 
							YWIN / 4, colors[0], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Bet", XWIN / 5, 
						YWIN / 4 + 60, colors[1], BG_COLOR);
	textout_centre_ex(buffer, font_menu, "Exit", XWIN / 5,
						YWIN / 4 + 120, colors[2], BG_COLOR);
}

//*******************************************//
//  displays the triangles of the main menu  //
//*******************************************//
void	draw_menu_triangles(int submenu, int select, int last)
{
	if (submenu) {
		rotateTriangle(buffer, XWIN / 2 - 100 + XWIN / 4 * (int)(last / 5), 
						YWIN / 5 + 60 * (last % 5) + 30, BG_COLOR, 10, 0);
		rotateTriangle(buffer, XWIN / 2 + 100 + XWIN / 4 * (int)(last / 5), 
						YWIN / 5 + 60 * (last % 5) + 30, BG_COLOR, 10, M_PI);
		rotateTriangle(buffer, XWIN / 2 - 100 + XWIN / 4 * (int)(select / 5),
						 YWIN / 5 + 60 * (select % 5) + 30, YELLOW, 10, 0);
		rotateTriangle(buffer, XWIN / 2 + 100 + XWIN / 4 * (int)(select / 5),
						 YWIN / 5 + 60 * (select % 5) + 30, YELLOW, 10, M_PI);
	}
	else {
		rotateTriangle(buffer, XWIN / 5 - 65, YWIN / 4 + 60 
						* last + 30, BG_COLOR, 10, 0);
		rotateTriangle(buffer, XWIN / 5 + 65, YWIN / 4 + 60 
						* last + 30, BG_COLOR, 10, M_PI);
		rotateTriangle(buffer, XWIN / 5 - 65, YWIN / 4 + 60 
						* select + 30, YELLOW, 10, 0);
		rotateTriangle(buffer, XWIN / 5 + 65, YWIN / 4 + 60 
						* select + 30, YELLOW, 10, M_PI);
	}
}

//********************************************//
//  updates selection in the main/pause menu  //
//********************************************//
void	update_select(int *select, int *last,
			int n, int submenu, int *colors)
{
	*select = *select % n;
	if (*select < 0)
		*select = n + *select;
	colors[*select + 3 * submenu] = YELLOW;
	colors[*last + 3 * submenu] = WHITE;
}

//**************************************//
//  displays and manages the main menu  //
//**************************************//
int menu(int n, int submenu, int prev_select)
{
	int				select = prev_select, k;
	int 			last, colors[13];
	struct timeval	t1, t2;
	float			dt;

	initialize_menu(colors, &select, prev_select, submenu);
	gettimeofday(&t1, NULL);

	while (TRUE) {
		timing(&t1, &t2, &dt);
		if (dt > (float)1000 / (float)FPS) {

			draw_menu_commands(colors);
			draw_menu_levels(colors);			
			draw_menu_triangles(submenu, select, last);
			draw_sprite(screen, buffer, 0, 0);			

			k = readkey();
			k = k >> 8;
			gettimeofday(&t1, NULL);

			if (k == KEY_DOWN) {
				last = select;
				select++;
			}
			if (k == KEY_UP) {
				last = select;
				select--;
			}
			if (k == KEY_ESC)
				if(submenu){
					select = -1;
					break;
				}
				else
					continue;

			if (k == KEY_ENTER)
				break;				
			if (k != KEY_UP && k != KEY_DOWN && k != KEY_ENTER)
				continue;
			update_select(&select, &last, n, submenu, colors);
		}
	}
	return select;
}

//*****************************************//
//  displays pause commands and triangles  //
//*****************************************//
void	draw_pause_menu(int background, int *colors, int *select, int *last)
{
	rectfill(buffer, XWIN / 2 - 200, YWIN / 4, XWIN / 2 + 200, 
							YWIN / 4 + 300, background);

	textout_centre_ex(buffer, font_menu, "Resume", 
	XWIN / 2, YWIN / 4 + 60, colors[0], background);

	textout_centre_ex(buffer, font_menu, "Restart", 
	XWIN / 2, YWIN / 4 + 120, colors[1], background);
		
	textout_centre_ex(buffer, font_menu, "Exit", XWIN / 2, 
				YWIN / 4 + 180, colors[2], background);

	rotateTriangle(buffer, XWIN / 2 - 100, YWIN / 4 + 60
						 * (*last) + 90, background, 10, 0);
	rotateTriangle(buffer, XWIN / 2 + 100, YWIN / 4 + 60
						 * (*last) + 90, background, 10, M_PI);
	rotateTriangle(buffer, XWIN / 2 - 100, YWIN / 4 + 60
						 * (*select) + 90, YELLOW, 10, 0);
	rotateTriangle(buffer, XWIN / 2 + 100, YWIN / 4 + 60
						 * (*select) + 90, YELLOW, 10, M_PI);
}

//***********************************************//
//  initializes the variables of the pause menu  //
//***********************************************//
void	init_pause(int *colors)
{
	start_write(&sem_pause);
	PAUSE = TRUE;
	end_write(&sem_pause);

	colors[0] = YELLOW;
	colors[1] = WHITE;
	colors[2] = WHITE;
}

//***************************************//
//  displays and manages the pause menu  //
//***************************************//
int paus(int n)
{
	int 			select = 0, k, last, colors[3];
	struct timeval	t1, t2;
	float 			dt;
	int 			background = makecol8(50, 50, 50);

	init_pause(colors);

	gettimeofday(&t1, NULL);
	while (TRUE) {
		timing(&t1, &t2, &dt);
		if (dt > (float)1000 / (float)FPS) {

			draw_pause_menu(background, colors, &select, &last);
			draw_sprite(screen, buffer, 0, 0);

			k = readkey();
			k = k >> 8;
			gettimeofday(&t1, NULL);

			if (k == KEY_DOWN) {
				last = select;
				select++;
			}
			if (k == KEY_UP) {
				last = select;
				select--;
			}
			if (k == KEY_ENTER)
				break;
			if (k != KEY_UP && k != KEY_DOWN && k != KEY_ENTER)
				continue;

			update_select(&select, &last, n, 0, colors);
		}
	}	
	return select;
}

//**************************************************//
//  resets game parameters after before restarting  //
//**************************************************//
void restart()
{
	for (int i = 0; i < N_BALLS; i++) {
		init_ball(i);
		sprintf(amounts[i], "%d", i);
		sprintf(names[i], "%d", i);
		while (check_collision(i))
			init_ball(i);
	}

	balls[0].color = RED;
	balls[0].r = 53 * SCALE;
	balls[0].amount = balls[0].r * balls[0].r * M_PI;

	for (int i = 0; i < N_BALLS; i++) {
		itos(amounts[i], balls[i].amount);
	}

	if (BET) {
		balls[0].alive = FALSE;
	}

	start_write(&sem_end);
	END = TRUE;
	end_write(&sem_end);

	float init_px = balls[0].px;
	float init_py = balls[0].py;
	position_mouse(balls[0].px, balls[0].py);
}

//********************************//
//  initializes global variables  //
//********************************//
void	init_variables(int argc, char **argv)
{
	BET = FALSE;
	PAUSE = 0;
	KILL_ALL = 0;

	if (!check_arguments(argc, argv))
		exit(1);

	XWIN = atoi(argv[1]);
	YWIN = atoi(argv[2]);
	if (XWIN < 200) {
		printf("Resolution too low. Please insert a higher width.\n");
		exit(1);
	}
	if (YWIN < 200) {
		printf("Resolution too low. Please insert a higher height.\n");
		exit(1);
	}
	diag = XWIN / cos(atanf((float)YWIN / (float)XWIN));
	VELOCITY = ((float)XWIN * (float)YWIN) / ((float)1366 * (float)768);
	SCALE = ((float)XWIN + (float)YWIN) / ((float)1366 + (float)768);
}

//*******************************//
//  initializes graphic library  //
//*******************************//
void	init_graphics(int argc, char **argv)
{
	allegro_init();
	ptask_init(SCHED_FIFO, GLOBAL, PRIO_INHERITANCE);
	install_keyboard();
	install_mouse();
	srand(time(0));
	set_color_depth(8);

	buffer = create_bitmap(XWIN, YWIN);
	font_menu = load_font("./font/score_comic.pcx", NULL, NULL);
	font_game = load_font("./font/arial.pcx", NULL, NULL);

	if (argc == 4 && (*argv[3] == 'w' || *argv[3] == 'W'))
		set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
	else
		set_gfx_mode(GFX_AUTODETECT, XWIN, YWIN, 0, 0);
}

//*******************************************//
//  main function: where the program starts  //
//*******************************************//
int main(int argc, char **argv)
{
	int 	select = 0, select1;

	init_variables(argc, argv);
	init_graphics(argc, argv);

	while (TRUE) {
		select = menu(3, 0, select);
		if (select == 2 || select == -1) {
			close_all(0);
		}
		select1 = menu(10, 1, select);
		if (select1 != -1)
			break;
	}

	while (TRUE) {
		if (KILL_ALL)
			KILL_ALL = FALSE;
		if (select1 < 3)
			game(select1 * 2 + 3 + select, select);
		if (select1 > 2 && select1 < 8)
			game((select1 - 3) * 5 + 10 + select, select);
		if (select1 > 7)
			game((select1 - 8) * 10 + 37 + select, select);
	}

	close_all(0);
}