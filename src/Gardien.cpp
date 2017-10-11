#include "Gardien.h"

#include <stdlib.h>
#include <limits.h>
#include <math.h>
#include <ctime>
#include <algorithm>

using namespace std;

int Gardien::_role = 0; // Permet d'executer une fois la fonction role()

/*
 * Constructeur du Gardien
 */
Gardien::Gardien(Labyrinthe* l, const char* modele) : Entity (120, 80, l, modele) {
	_defense = false; // Par défaut tous les gardiens sont en mode patrouille
	_angle = (rand() % 360);
	_hasfb = false;
	_lastfb = clock();
}

/*
 * Permet de faire avancer le gardien par rapport à son angle de vision
 */
bool Gardien::moveAngle() {
	float radian = _angle * (2 * M_PI)/360;

	double dx = -sin(radian) * SPEED;
	double dy = cos(radian) * SPEED;
	int x = (int)((_x + dx) / Environnement::scale);
	int y = (int)((_y + dy) / Environnement::scale);

	// Aucun block
	if (EMPTY == _l->data(x, y)) {
		// Aucune entity
		if (hasEntity(x, y) == EMPTY) {
			_x += dx;
			_y += dy;
			return true;
		}
	}
	return false;
}

/*
 *
 */
void Gardien::update() {
	role();

	// Vérifie si le joueur est mort
	if (isDead()) {
		return;
	}

	// Vérifie si il peut voir le chasseur
	if (canViewChasseur()) {
		updateAttaque();
	}

	// Si il y a collision on actualise l'angle
	if (!moveAngle()) {
		if (isDefense()) {
			updateDefense();
		} else {
			updatePatrouille();
		}
	}
}

/*
 * Executer une fois par actualisation de tous les gardiens
 */
void Gardien::role() {
	if (Gardien::_role < _l->_nguards-1) {
		Gardien::_role++;
		return;
	}
	Gardien::_role=1;

	int point = 0;
	int defense = 0;

	for (int cpt=1; cpt < _l->_nguards; cpt++) {
		Gardien* guard = (Gardien*) _l->_guards[cpt];
		if (!guard->isDead()) {
			// Si le garde est a plus de 256 blocs il rapport 0 point
			point += 256 - min(256, guard->getDistance());

			if (guard->isDefense()) {
				defense++;
			}
		}
	}

	if (point <= DEFENSE - 256) {
		// Pas assez de defense
		defense++;
	} else if (point >= DEFENSE + 256) {
		// Trop de defense
		defense--;
	}

	//cout << "defense : " << defense << " ;point : " << point <<  endl;

	for (int cpt=1; cpt < _l->_nguards; cpt++) {
		Gardien* guard = (Gardien*) _l->_guards[cpt];
		if (!guard->isDead()) {
			if (defense <= 0) {
				guard->_defense = false;
			} else {
				guard->_defense = true;
				defense--;
			}
		}
	}

	// Regeneration
	for (int cpt=0; cpt < _l->_nguards; cpt++) {
		((Entity*) _l->_guards[cpt])->regeneration();
	}
}

/*
 * Permet de savoir si le gardien peut voir le joueur.
 * En simulant l'avancement d'une boule de feu et en vérifiant si elle
 * peut arriver jusqu'au joueur sans collision
 */
bool Gardien::canViewChasseur() {
	Mover* chasseur = _l->_guards[0];

	// La distance réel de entre le chasseur et le garde
	int distance = sqrt(pow(_x - chasseur->_x, 2) + pow(_y - chasseur->_y, 2));

	// La distance de vision du garde
	int min = (MIN_VIEW + (MAX_VIEW - MIN_VIEW) * (((double)_health) / MAX_HEALTH)) * Environnement::scale;

	if (distance <= min) {
		int xa = chasseur->_x / Environnement::scale;
		int ya = chasseur->_y / Environnement::scale;
		int xb = _x / Environnement::scale;
		int yb = _y / Environnement::scale;

		float x = xb;
		float y = yb;
		int rx = xb;
		int ry = yb;

		float angle = atan2(-(xa - xb), ya - yb);
		float xv = -sin(angle);
		float yv = cos(angle);

		while (rx != xa || ry != ya) {
			if (EMPTY != _l->data(rx, ry) || hasEntity(rx, ry) != EMPTY) {
				return false;
			}

			x += xv;
			y += yv;
			rx = round(x);
			ry = round(y);
		}
		return true;
	}
	return false;
}

/*
 * Mise à jour des gardes Patrouille
 */
void Gardien::updatePatrouille() {
	_angle = (rand() % 360); // N'importe quelle direction
}

/*
 * Mise à jour des gardes Attaque
 */
void Gardien::updateAttaque() {
	Mover* chasseur = _l->_guards[0];

	double x = chasseur->_x - _x;
	double y = chasseur->_y - _y;

	_angle = 180 * atan2(-x, y) / M_PI;

	if(!_hasfb) {
		fire(0);
	}
}

/*
 * Mise à jour des gardes Defense
 */
void Gardien::updateDefense() {
	struct Prob {
		int	_x, _y;
		int _value;
	};

	int x = _x/Environnement::scale;
	int y = _y/Environnement::scale;

	int minValue = ((Labyrinthe*) _l)->distance(x, y);

	// Si est proche du tresor
	if (minValue <= 10) {
		_angle = (rand() % 360);
	} else {
		int probLength = 0;
		int total = 0;

		// Valeur min et nombre de prob
		for (int c=x-1; c <= x+1 && c>=0 && c < _l->width(); c++) {
			for (int l=y-1; l <= y+1 && l>=0 && l < _l->height(); l++) {
				if ((l != y || c != x)) {
					minValue = min(((Labyrinthe*) _l)->distance(c, l), minValue);
					probLength++;
				}
			}
		}

		Prob probs[probLength];
		probLength = 0;
		for (int c=-1; c <= 1 && c+x>=0 && c+x < _l->width(); c++) {
			for (int l=-1; l <= +1 && l+y>=0 && l+y < _l->height(); l++) {
				if ((l != 0 || c != 0)) {
					int value = 1;
					if (((Labyrinthe*) _l)->distance(c+x, l+y) == minValue) {
						value = 20;
					}
					total += value;

					probs[probLength]._x = c;
					probs[probLength]._y = l;
					probs[probLength]._value = value;
					probLength++;
				}
			}
		}

		int random = rand() % total;
		int value = 0;
		probLength = -1;
		do {
			probLength++;
			value += probs[probLength]._value;
		} while (random > value);

		_angle = 180 * atan2(-probs[probLength]._x, probs[probLength]._y) / M_PI;
	}
}

/*
 *	Fait bouger la boule de feu
 */
bool Gardien::process_fireball (float dx, float dy) {
	int bx = (int)((_fb -> get_x () + dx) / Environnement::scale);
	int by = (int)((_fb -> get_y () + dy) / Environnement::scale);

	for (int cpt=0; cpt < _l->_nguards; cpt++) {
		Entity* guard = (Entity*)_l->_guards[cpt];
		if (this != guard && !guard->isDead()) {
			int gx = (int)(guard->_x/Environnement::scale);
			int gy = (int)(guard->_y/Environnement::scale);

			if (gx == bx && gy == by) {
				impact(bx, by);
				return false;
			}
		}
	}

	if (EMPTY == _l->data(bx, by)) {
		// il y a la place.
		return true;
	}

	impact(bx, by);
	return false;
}

/*
 * Rayon d'impact de la boule de feu
 */
void Gardien::impact(int bx, int by) {
	for (int cpt=0; cpt < _l->_nguards; cpt++) {
		Entity* guard = (Entity*)_l->_guards[cpt];
		int gx = (int)(guard->_x/Environnement::scale);
		int gy = (int)(guard->_y/Environnement::scale);


		if (this != guard &&
				gx >= bx-IMPACT && gx <= bx+IMPACT &&
				gy >= by-IMPACT && gy <= by+IMPACT) {
			guard->damage(5);
		}
	}
	_hasfb = false;
}

/*
 *	Tire sur un ennemi.
 */
void Gardien::fire (int angle_vertical) {
	// Delais entre 2 tirs
	if (double(clock() - _lastfb) / CLOCKS_PER_SEC < TIME_FIRE) return;

	// Rajoute une probabilité que le gardien rate sa cible
	int min = MAX_ANGLE + (MIN_ANGLE-MAX_ANGLE) * (((double)_health) / MAX_HEALTH);
	int random = 0;
	if (min != 0) {
		random = (min/2)-(rand() % min);
	}

	_fb -> init (_x, _y, 10., angle_vertical, -_angle + random);
	_hasfb = true;
	_lastfb = clock();
}
