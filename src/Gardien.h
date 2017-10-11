#ifndef GARDIEN_H
#define GARDIEN_H

#include "Entity.h"
#include "Labyrinthe.h"
#include <ctime>

#define MIN_VIEW	10	// Distance de vision minimum
#define MAX_VIEW	80 	// Distance de vision maximum

#define MIN_ANGLE	1 	// Permet de mettre une probabilité
#define MAX_ANGLE	10 	// que le gardien rate sa cible

#define SPEED		0.5 // Vitesse de déplacement
#define TIME_FIRE	0.5 // Délais entre deux tirs

#define DEFENSE		2048 // A environ plus ou moins 256

struct Vector {
	double	_x, _y;
	bool value;
};

class Gardien : public Entity {
private:
	static int _role;

	bool 	_defense;  	// Mode défense
	bool	_hasfb;		// Permet de savoir son fb a explosé
	clock_t	_lastfb;    // Permet de savoir quand il a tiré son fb

	void role(void);
	void updateDefense (void);
	void updateAttaque (void);
	void updatePatrouille (void);

	bool moveAngle();
	bool canViewChasseur();
	void impact(int bx, int by);

public:

	Gardien (Labyrinthe* l, const char* modele);

	void update (void);
	bool move (double dx, double dy){return false;};
	void fire (int angle_vertical);
	bool process_fireball (float dx, float dy);

	void damage(int health);

	/*
	 * Retourne la distance du gardien au trésor
	 */
	int getDistance(){
		return ((Labyrinthe*) _l)->distance(
				_x / Environnement::scale,
				_y / Environnement::scale);
	}

	// Permet de savoir si le gardien est en mode défense
	bool isDefense() {
		return _defense;
	}
};

#endif
