#include "Labyrinthe.h"
#include "Chasseur.h"
#include "Gardien.h"

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <regex>
#include <limits.h>
#include <queue>

using namespace std;

Sound*	Chasseur::_hunter_fire;	// bruit de l'arme du chasseur.
Sound*	Chasseur::_hunter_hit;	// cri du chasseur touch�.
Sound*	Chasseur::_wall_hit;	// on a tap� un mur.

Environnement* Environnement::init (char* filename) {
	return new Labyrinthe (filename);
}

/*
 * Constructeur du labyrinthe
 */
Labyrinthe::Labyrinthe (char* filename) {
	ifstream file(filename, ios::in); // Chargement du fichier

	// Véricifation du fichier labyrinthe
	if (!file) {
		cerr << "[Labyrinthe] Erreur : Impossible de trouver le file du labyrinthe : '" << filename << "'" << endl;
		exit(0);
	}

	regex regexTexture("^([a-z])(?:[\\s\\t]+)([a-z\\.]+)"); // Expression régulière de la texture
	srand (time(NULL));

	_width = 0;
	_height = 0;
	_nwall = 0;
	_npicts = 0;
	_nboxes = 0;
	_nguards = 0;

	int nbTexture = 0; // Nombre texture
	bool lab = false; // Si on fini de lire les textures et que l'on est dans le labyrinthe
	
	// Boucle qui permet de savoir la taille du labyrinthe
	for(string line; getline(file, line);) {
		// Ligne de Texture
		if (regex_search(line, regexTexture)) {
			nbTexture++; // Increment le nombre de texture

		// Autre ligne
		} else {
			int width = 0;
			for(char& c : line) {
				if (c == '#') {
					break; // Si il y a un commentaire
				} else if (c=='+') {
					lab = true; // Signale que l'on commence à lire le labyrinthe
				}
				width++;
			}
			
			if (lab) {
				_width = max(width, _width);
				_height++;
			}
		}
	}

	// Message d'information
	cout << "[Labyrinthe] Width : " << _width << endl;
	cout << "[Labyrinthe] Height : " << _height << endl;
	cout << "[Labyrinthe] Nombre de texture : " << nbTexture << endl;

	// Intitialisation des tableaux
	int* textures = new int['z'];
	_data = (char**) malloc(sizeof(*_data)*_height);
	if (_data == NULL) {
		cerr << "[Labyrinthe] Erreur : Impossible d'allouer de la mémoire" << endl;
		exit(0);
	}

	for (int l = 0; l < _height; l++) {
		_data[l] = (char*) malloc(sizeof(**_data)*_width);
		if (_data[l] == NULL) {
			cerr << "[Labyrinthe] Erreur : Impossible d'allouer de la mémoire" << endl;
			exit(0);
		}

		for (int c = 0; c < _width; c++) {
			_data[l][c] = EMPTY; // Initialise tous les cases à 0
		}
	}

	// Initialise les textures
	int nbTresor = 0; // Pour vérifier qu'il y a uniquement un trésor
	int numLigne = 0;
	smatch matches;
	lab = false;
	
	// Relecture le fichier
	file.clear();
	file.seekg(0,ios::beg);

	// Permet de charger les textures et le labyrinthe
	for(string line; getline(file, line);) {
		// Ligne de Texture
		if (regex_search(line, matches, regexTexture)) {
			std::string name = matches[1];
			std::string value = matches[2];

			char tmp[strlen(texture_dir) + value.length() + 1];
			sprintf (tmp, "%s/%s", texture_dir, value.c_str());
			textures[(int) name.c_str()[0]] = wall_texture (tmp);

		// Ligne du labyrinthe
		} else {
			int numColonne = 0;
			for(char& c : line) {
				// Commentaire
				if (c == '#') {
					break;
				// Debut du labyrinthe
				} else if (c=='+') {
					lab = true;
					_nwall++;
				// Un coffre
				} else if (c=='x') {
					_nboxes++;
				// Un garde
				} else if (c=='G') {
					_nguards++;
				// Un tresor
				} else if (c=='T') {
					nbTresor++;
				// Une affiche
				} else if (c >= 'a' && c <= 'z') {
					_npicts++;
				}

				if (lab) {
					if (c == ' ' || c== '\n') {
						_data[numLigne][numColonne] = EMPTY;
					} else {
						_data[numLigne][numColonne] = c;
					}
				}
				numColonne++;
			}
			if (lab) {
				numLigne++;
			}
		}
	}
	file.close();
	
	// Vérifie le nombre de trésor
	if (nbTresor != 1) {
		cerr << "[Labyrinthe] Erreur : Dans le nombre de tresor" << endl;
		exit(0);
	}

	cout << "[Labyrinthe] _nboxes : " << _nboxes << endl;
	cout << "[Labyrinthe] _nguards : " << _nguards << endl;
	
	// Intialisation des objects
	_walls = new Wall[_nwall*4];
	_picts = new Wall[_npicts*2];
	_boxes = new Box[_nboxes];
	_guards = new Mover*[_nguards+1];
	_nwall = 0;
	_npicts = 0;
	_nboxes = 0;
	_nguards = 1; // Le garde 0 = Chasseur

	bool angle = false;
	
	// Permet de créer les murs verticaux
	for (int numLigne=0; numLigne < _height; numLigne++) {
		for (int numColonne=0; numColonne < _width; numColonne++) {
			char c = _data[numLigne][numColonne];
			// Un angle
			if (c=='+') {
				// Si c'est la fin d'un mur
				if (angle) {
					// Si il y a un mur après
					if (numColonne+1 < _width) {
						char next = _data[numLigne][numColonne+1];
						angle = next == '+' || next == '-' || (next >= 'a' && next <= 'z'  && next != 'x');
					} else {
						angle = false;
					}

					// Si fin du mur
					if (!angle) {
						_walls[_nwall]._x2 = numColonne;
						_walls[_nwall]._y2 = numLigne;
						_walls[_nwall]._ntex = 0;
						_nwall++;
					}
				// Si c'est le début d'un mur
				} else {
					angle = true;
					_walls[_nwall]._x1 = numColonne;
					_walls[_nwall]._y1 = numLigne;
				}
			// Un coffre
			} else if (c=='x') {
				_boxes[_nboxes]._x = numColonne;
				_boxes[_nboxes]._y = numLigne;
				_nboxes++;
			// Un tresor
			} else if (c=='T') {
				_treasor._x = numColonne;
				_treasor._y = numLigne;
			// Un garde
			} else if (c=='C') {
				Chasseur *chasseur = new Chasseur(this);
				chasseur->_x = numColonne*10;
				chasseur->_y = numLigne*10;
				_guards[0] = chasseur;
			} else if (c=='G') {
				Gardien *guard = new Gardien(this, Labyrinthe::randomGuard());
				guard->_x=numColonne*scale;
				guard->_y=numLigne*scale;
				_guards[_nguards] = guard;
				_nguards++;
			// Une affiche
			} else if (c >= 'a' && c <= 'z' && c != 'x' && numColonne+1 < _width) {
				char next = _data[numLigne][numColonne+1];
				if (next == '+' || next == '-') {
					_picts[_npicts]._x1 = numColonne;
					_picts[_npicts]._y1 = numLigne;
					_picts[_npicts]._x2 = numColonne+2;
					_picts[_npicts]._y2 = numLigne;
					_picts[_npicts]._ntex = textures[(int) c];
					_npicts++;
				}
			} else if (angle && c==0) {
				cerr << "[Labyrinthe] Erreur : Format" << endl;
				exit(0);
			}
		}

		if (angle) {
			cerr << "[Labyrinthe] Erreur : Format" << endl;
			exit(0);
		}
	}

	// Permet de créer les murs horizontaux
	for (int numColonne=0; numColonne < _width; numColonne++) {
		for (int numLigne=0; numLigne < _height; numLigne++) {
			char c = _data[numLigne][numColonne];

			// Un angle
			if (c == '+') {
				// Si c'est la fin d'un mur
				if (angle) {
					// Si il y a un mur après
					if (numLigne+1 < _height) {
						char next = _data[numLigne+1][numColonne];
						angle = next == '+' || next == '|' || (next >= 'a' && next <= 'z' && next != 'x');
					} else {
						angle = false;
					}

					// Si fin du mur
					if (!angle) {
						_walls[_nwall]._x2 = numColonne;
						_walls[_nwall]._y2 = numLigne;
						_walls[_nwall]._ntex = 0;
						_nwall++;
					}
				// Si c'est le début d'un mur
				} else {
					angle = true;
					_walls[_nwall]._x1 = numColonne;
					_walls[_nwall]._y1 = numLigne;
				}
			// Une affiche
			} else if (c >= 'a' && c <= 'z' && c != 'x' && numLigne+1 < _height) {
				char next = _data[numLigne][numColonne+1];
					if (next == '+' || next == '|') {
					_picts[_npicts]._x1 = numColonne;
					_picts[_npicts]._y1 = numLigne;
					_picts[_npicts]._x2 = numColonne;
					_picts[_npicts]._y2 = numLigne+2;
					_picts[_npicts]._ntex = textures[(int) c];
					_npicts++;
					}
			} else if (angle && c==0) {
				cerr << "[Labyrinthe] Erreur : Format" << endl;
				exit(0);
			}
		}

		if (angle) {
			cerr << "[Labyrinthe] Erreur : Format" << endl;
			exit(0);
		}
	}
	
	// Mise à jour des datas
	for (int l=0; l < _height; l++) {
		for (int c=0; c < _width; c++) {
			char data = _data[l][c];
			if (data == 'C') {
				_data[l][c] = EMPTY;
			} else if (data == 'G') {
				_data[l][c] = EMPTY;
			} else if (data == 'x') {
				_data[l][c] = EMPTY;
			} else if (data != 0) {
				_data[l][c] = 1;
			}
		}
	}

	// Affiche le labyrinthe
	cout << "[Labyrinthe] Affichage : " << endl;
	for (int l=0; l < _height; l++) {
		for (int c=0; c < _width; c++) {
			if(_data[l][c] == EMPTY) {
				cout << " ";
			} else {
				cout << "1";
			}
		}
		cout << endl;
	}
	cout << "[Labyrinthe] Fin affichage : " << endl;

	// Intitialisation des tableaux pour l'algo de distance
	_distance = (int**) malloc(sizeof(*_distance)*_height);
	if (_distance == NULL) {
		cerr << "[Labyrinthe] Erreur : Impossible d'allouer de la mémoire" << endl;
		exit(0);
	}
	for (int l = 0; l < _height; l++) {
		_distance[l] = (int*) malloc(sizeof(**_distance)*_width);
		if (_data[l] == NULL) {
			cerr << "[Labyrinthe] Erreur : Impossible d'allouer de la mémoire" << endl;
			exit(0);
		}
	}

	Labyrinthe::algoDistance();
}

Labyrinthe::~Labyrinthe() {
	cout << "[Labyrinthe] Debut Delete " << endl;
	for (int l = 0; l < _height; l++) {
		free(_data[l]);
		free(_distance[l]);
	}
	free(_data);
	free(_distance);

	cout << "[Labyrinthe] Fin Delete " << endl;
}

char* Labyrinthe::randomGuard() {
	int random = rand() % 7;

	if (random <= 1) {
		return (char*) "drfreak";
	} else if (random <= 2) {
		return (char*) "Marvin";
	} else if (random <= 3) {
		return (char*) "Potator";
	} else if (random <= 4) {
		return (char*) "Serpent";
	} else if (random <= 5) {
		return (char*) "Samourai";
	} else if (random <= 6) {
		return (char*) "Lezard";
	} else {
		return (char*) "garde";
	}
}

/*
 * Permet de généré un
 */
void Labyrinthe::algoDistance() {
	// Intitialisation des tableaux
	for (int l = 0; l < _height; l++) {
		for (int c = 0; c < _width; c++) {
			_distance[l][c] = INT_MAX;
		}
	}

	struct Loc {
		int	_x, _y;
		int _value;
	};

	queue<Loc> locations;
	Loc treasor {_treasor._x, _treasor._y, 0};
	locations.push(treasor);

	while (!locations.empty()) {
		Loc location = locations.front();
		if (_distance[location._y][location._x] > location._value) {
			_distance[location._y][location._x] = location._value;

			for (int x=location._x-1; x <= location._x+1 && x>=0 && x < _width; x++) {
				for (int y=location._y-1; y <= location._y+1 && y>=0 && y < _height; y++) {
					char d = data(x, y);
					if ((x != location._x || y != location._y) && d == EMPTY) {
						Loc next {x, y, location._value+1};
						locations.push(next);
					}
				}
			}
		}
		locations.pop();
	}

	/*
	// Debug
	cout << "[Labyrinthe] Distance : " << endl;
	for (int l=0; l < _height; l++) {
		for (int c=0; c < _width; c++) {
			cout << _distance[l][c] << " ";
		}
		cout << endl;
	}
	*/
}

/*
 * Permet de savoir s'il y a un mur ou une box à une coordonné
 */
char Labyrinthe::data(int i, int j) {
	char data = _data [j][i];
	if (data == EMPTY) {
		return hasBox(i, j);
	}
	return data;
}

/*
 * Permet de savoir s'il y a une box à une coordonné
 */
char Labyrinthe::hasBox(int i, int j) {
	for (int cpt=0; cpt < _nboxes; cpt++) {
		Box box = _boxes[cpt];
		if (box._x == i && box._y == j) {
			return 'x';
		}
	}
	return EMPTY;
}

/*
 * Retourne le numéro d'un box à une coordonné ou -1 s'il y en a pas
 */
int Labyrinthe::getBox(int x, int y) {
	for (int cpt=0; cpt < _nboxes; cpt++) {
		Box box = _boxes[cpt];
		if (box._x == x && box._y == y) {
			return cpt;
		}
	}
	return -1;
}

/*
 * Permet de déplacer une box
 */
bool Labyrinthe::moveBox(int x, int y, int dx, int dy) {
	// La destination est vide
	if (EMPTY == data(dx, dy)) {
		int box = getBox(x, y);
		if (box != -1) {
			_boxes[box]._x = dx;
			_boxes[box]._y = dy;

			algoDistance();
			reconfigure();
			return true;
		}
	}
	return false;
}
