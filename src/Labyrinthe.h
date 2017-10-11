#ifndef LABYRINTHE_H
#define LABYRINTHE_H

#include "Environnement.h"

#define MOVER	2

class Labyrinthe : public Environnement {
private:
	char**	_data;		// Liste des positions des murs
	int** _distance; 	// Liste des distances au trésor

	int     _width;
	int     _height;

	char* randomGuard();

	int getBox(int x, int y);
	~Labyrinthe();
public:
	Labyrinthe (char*);

	// retourne la largeur du labyrinthe.
	int width () {
        return _width;
	}

	// retourne la longueur du labyrinthe.
	int height () {
        return _height;
	}

	// retourne la case (i, j).
	char data(int i, int j);
	char hasBox(int i, int j);

	bool moveBox(int x, int y, int dx, int dy);
	void algoDistance();
	// retourne la distance (i, j).
	int distance(int i, int j) {
		return _distance[j][i];
	}
};

#endif
