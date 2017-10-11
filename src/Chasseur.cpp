#include "Chasseur.h"

#include <stdlib.h>
#include <regex>
#include <limits.h>

using namespace std;

/*
 *	Constructeur du Chasseur
 */
Chasseur::Chasseur (Labyrinthe* l) : Entity (100, 80, l, 0) {
	_hunter_fire = new Sound ("sons/hunter_fire.wav");
	_hunter_hit = new Sound ("sons/hunter_hit.wav");
	if (_wall_hit == 0)
		_wall_hit = new Sound ("sons/hit_wall.wav");
}

/*
 *	Fait bouger la boule de feu
 */
bool Chasseur::process_fireball (float dx, float dy) {
	int bx = (int)((_fb-> get_x() + dx) / Environnement::scale);
	int by = (int)((_fb->get_y() + dy) / Environnement::scale);

	for (int cpt=1; cpt < _l->_nguards; cpt++) {
		Entity* guard = (Entity*)_l->_guards[cpt];

		if (!guard->isDead()) {
			int gx = (int)(guard->_x/Environnement::scale);
			int gy = (int)(guard->_y/Environnement::scale);

			if (gx >= bx-1 && gx <= bx+1 && gy >= by-1 && gy <= by+1) {
				impact(bx, by);
				return false;
			}
		}
	}

	if (EMPTY == _l->data(bx, by)) {
		// il y a la place.
		return true;
	}

	// calculer la distance entre le chasseur et le lieu de l'explosion.
	float x = (_x - _fb->get_x()) / Environnement::scale;
	float y = (_y - _fb->get_y()) / Environnement::scale;
	float dist2 = x*x + y*y;

	float dmax2 = (_l->width())*(_l->width())+(_l->height())*(_l->height());
	_wall_hit->play(1.-dist2/dmax2);

	impact(bx, by);
	return false;
}

/*
 * Rayon d'impact de la boule de feu
 */
void Chasseur::impact(int bx, int by) {
	for (int cpt=0; cpt < _l->_nguards; cpt++) {
		Entity* guard = (Entity*)_l->_guards[cpt];
		int gx = (int)(guard->_x/Environnement::scale);
		int gy = (int)(guard->_y/Environnement::scale);


		if (gx >= bx-IMPACT && gx <= bx+IMPACT
				&& gy >= by-IMPACT && gy <= by+IMPACT) {
			guard->damage(5);
		}
	}
}

/*
 *	Tire sur un ennemi.
 */
void Chasseur::fire (int angle_vertical) {
	_hunter_fire->play();
	_fb->init(_x, _y, 10., angle_vertical, _angle);
}

/*
 *	Tente un deplacement.
 */
bool Chasseur::move_aux (double dx, double dy) {
	int x = (int)((_x + dx) / Environnement::scale);
	int y = (int)((_y + dy) / Environnement::scale);

	char data = _l->data(x, y);
	if (EMPTY == data && hasEntity(x, y) == EMPTY) {
		_x += dx;
		_y += dy;
		return true;
	} else if (!isDead() && _l->_treasor._x == x && _l->_treasor._y == y) {
		partie_terminee(true);
	} if (data == 'x') {
		if (moveBox(x, y, dx, dy)) {
			_x += dx;
			_y += dy;
			return true;
		}
	}
	return false;
}

/*
 * Déplace une box
 */
bool Chasseur::moveBox(int x, int y, double vx, double vy) {
	// Si le vecteur est assez grand
	if (std::abs(vx) > 0.9 || std::abs(vy) > 0.9) {
		int dx = round(x + vx);
		int dy = round(y + vy);

		return ((Labyrinthe*) _l)->moveBox(x, y, dx, dy);
	}
	return false;
}
