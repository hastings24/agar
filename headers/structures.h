#ifndef STRUCTURE_H
#define STRUCTURE_H

//ball structure
struct ball{
	float	px, py, r, amount, angle;
	int		color, alive;
};

//syncronization structure
struct sync {
	int		nbr, nbw, nr, nw;
	sem_t	priv_r, priv_w, m;
};

#endif