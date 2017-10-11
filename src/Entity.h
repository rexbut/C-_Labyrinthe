#ifndef ENTITY_H
#define ENTITY_H

#include "Mover.h"
#include <ctime>

#define IMPACT		1	// Rayon de l'impact
#define MAX_HEALTH	20  // Vie maximum
#define TIME_REGENERATION	2 // Temps de régénération (en seconde)

class Entity : public Mover {
protected :
	int _health;
	clock_t _lastDamage;
	clock_t _lastRegeneration;

public:
	Entity (int x, int y, Labyrinthe* l, const char* modele);

	bool isDead() {
		return _health <= 0;
	}

	int getHealth() {
		return _health;
	}

	void regeneration(void);
	void damage(int damage);

	char hasEntity(int i, int j);
};

#endif
