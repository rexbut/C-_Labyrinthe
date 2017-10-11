#include "Entity.h"

#include <stdlib.h>
#include <math.h>
#include <ctime>
#include <algorithm>

using namespace std;

Entity::Entity (int x, int y, Labyrinthe* l, const char* modele) :
	Mover (120, 80, l, modele), _health (MAX_HEALTH),
	_lastDamage(clock()), _lastRegeneration(clock()) {
}

/*
 * Affligé un dégat
 */
void Entity::damage(int damage) {
	_lastDamage = clock();
	_health = max(0, _health - damage);

	// Si c'est le chasseur
	if (this == _l->_guards[0]) {
		message ("Vie : %d", getHealth());
		if (isDead()) {
			partie_terminee(false);
		}
	// Si c'est un garde
	} else {
		if (isDead()) {
			rester_au_sol();
		} else {
			tomber();
		}
	}
}

/*
 * Régénération de la vie
 */
void Entity::regeneration(void) {
	if (isDead() || _health >= MAX_HEALTH) return;

	if (double(clock() - _lastDamage) / CLOCKS_PER_SEC >= TIME_REGENERATION) {
		if (double(clock() - _lastRegeneration) / CLOCKS_PER_SEC >= TIME_REGENERATION) {
			_lastRegeneration = clock();
			_health = min(MAX_HEALTH, _health + 1);

			if (this == _l->_guards[0]) {
				message ("Vie : %d", getHealth());
			}
		}
	}
}

/*
 * Permet de savoir s'il y a une entité à une coordonné
 */
char Entity::hasEntity(int i, int j) {
	for (int cpt=0; cpt < _l->_nguards; cpt++) {
		Entity* entity = (Entity*)_l->_guards[cpt];

		if (this != entity && !entity->isDead()) {
			if ((int)(entity->_x/Environnement::scale) == i &&
					(int)(entity->_y/Environnement::scale) == j) {

				return 1;
			}
		}
	}
	return EMPTY;
}

