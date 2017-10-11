#ifndef CHASSEUR_H
#define CHASSEUR_H

#include <stdio.h>

#include "Entity.h"
#include "Labyrinthe.h"
#include "Sound.h"

class Chasseur : public Entity {
private:
	// accepte ou non un deplacement.
	bool move_aux (double dx, double dy);

	bool moveBox (int x, int y, double vx, double vy);

	void impact(int bx, int by);
public:
	/*
	 *	Le son...
	 */
	static Sound*	_hunter_fire;	// bruit de l'arme du chasseur.
	static Sound*	_hunter_hit;	// cri du chasseur touché.
	static Sound*	_wall_hit;		// on a tapé un mur.

	Chasseur (Labyrinthe* l);

	bool move (double dx, double dy) {
		return move_aux (dx, dy) || move_aux (dx, 0.0) || move_aux (0.0, dy);
	}
	void update (void) {};
	bool process_fireball (float dx, float dy);
	void fire (int angle_vertical);
	void damage(int health);
};

#endif
