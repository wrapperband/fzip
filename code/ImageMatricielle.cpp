#include "ImageMatricielle.h"

/* *************** Constructeur / Destructeur *************** */

ImageMatricielle::ImageMatricielle(unsigned int x, unsigned int y) : mLargeur(x), mHauteur(y) {
	/* Créée une nouvelle image de dimensions données
	 * Les pixels de l'image ne sont pas initialisés
	 */
	mImage = new unsigned int* [mLargeur];
	for(int i=0 ; i<mLargeur ; i++) {
		mImage[i] = new unsigned int[mHauteur];
	}
}

ImageMatricielle::ImageMatricielle(const char* fichier, int couche) {
	/* Ouvre un fichier '.png'
	 * Entrées :
	    - fichier : l'adresse du fichier
		- couche : la couche à lire (de 0 à 3)
	 * Sortie : si l'ouverture échoue, l'image prend les dimensions 0x0
	 */
	bool erreur = false;
	std::vector<unsigned char> png;
	std::vector<unsigned char> img;

	lodepng::load_file(png, fichier	);
	unsigned error = lodepng::decode(img, mLargeur, mHauteur, png);

	if(error) {
		std::cerr << fichier << " -> png decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
		mLargeur = mHauteur = 0;
		erreur = true;
	}

	mImage = new unsigned int* [mLargeur];
	for(int i=0 ; i<mLargeur ; i++) {
		mImage[i] = new unsigned int[mHauteur];
		for(int j=0 ; j<mHauteur ; j++) {
			mImage[i][j] = img[ (i*mHauteur + j)*4 + couche ];
		}
	}

	if(!erreur) std::cout << "image lue : " << fichier << " (" << mLargeur << "x" << mHauteur << "px) : couche " << couche << std::endl;
}

ImageMatricielle::~ImageMatricielle() {
	/* Suppression de l'image */
	for(int i=0 ; i<mLargeur ; i++) {
		delete[] mImage[i];
	}
	delete[] mImage;
}

/* *************** Setters / Getters *************** */

unsigned int* ImageMatricielle::operator[](int i) {
	/* Retourne la ligne de l'image correspondante */
	return mImage[i];
}

unsigned int ImageMatricielle::getHauteur() const { return mHauteur; }
unsigned int ImageMatricielle::getLargeur() const { return mLargeur; }

unsigned int ImageMatricielle::moyenne() const {
	int somme = 0;
	for(int i=0 ; i<mLargeur ; i++) {
		for(int j=0 ; j<mHauteur ; j++) {
			somme += mImage[i][j];
		}
	}
	return somme/(mLargeur * mHauteur);
}

void ImageMatricielle::adapterMoyenne(int val) {
	int decalage = val - moyenne();
	for(int i=0 ; i<mLargeur ; i++) {
		for(int j=0 ; j<mHauteur ; j++) {
			mImage[i][j] += decalage;
		}
	}
}

/* *************** Compression *************** */

std::vector<ImagePart> ImageMatricielle::decouper(int taille) {
	/* Découpe l'image en sous-images
	 * Les sous parties sont des carrés de côté "taille", le dépassement est ignoré
	 */
	std::vector<ImagePart> liste;
	for(int i=0 ; i*taille<mLargeur ; i++) {
		for(int j=0 ; j*taille<mHauteur ; j++) {
			liste.push_back( ImagePart(this, i*taille, j*taille, taille) );////////////////////////////////////////////// !!!!!!
		}
	}
	return liste;
}

IFS ImageMatricielle::chercherIFS(unsigned int taillePetit, unsigned int tailleGros) {
	/* Recherche l'ifs pour l'image
	 * Entrées :
	 *  - taillePetit : la taille des blocs du petit pavages
	 *  - tailleGros : taille des gros blocs, doit être plus grand que taillePetit
	 */
	int tDebut = time(0);
	if(taillePetit > tailleGros) {
		std::cout << "Le pavage n'est pas de la bonne dimension" << std::endl;
		IFS retour;
			retour.correspondances = std::vector<Source>();
			retour.decoupeGros = tailleGros;
			retour.decoupePetit = taillePetit;
		return retour;
	}
	std::cout << "Compression initiée" << std::endl;

	std::cout << " - Création des pavages";
	std::vector<ImagePart> pavagePetit = decouper(taillePetit);
	std::vector<ImagePart> pavageGros = decouper(tailleGros);
	std::cout << " (" << pavagePetit.size() << ":" << pavageGros.size() << " blocs)" << std::endl;

	chargement(" - Recherche des correspondances", 0, pavagePetit.size());
	std::vector<Source> correspondances;
	for(int i=0 ; i<pavagePetit.size() ; i++) {
		chargement(" - Recherche des correspondances", i, pavagePetit.size());
		correspondances.push_back( pavagePetit[i].chercherMeilleur(pavageGros) );
	}
	std::cout << "\r\033[K - " << pavagePetit.size() << " correspondances en " << time(0) - tDebut << "s" << std::endl;

	IFS retour;
		retour.correspondances = correspondances;
		retour.decoupeGros = tailleGros;
		retour.decoupePetit = taillePetit;
	return retour;
}

ImageMatricielle ImageMatricielle::appliquerIFS(const IFS& ifs) {
	/* Applique l'IFS et en retourne le résultat */
	ImageMatricielle sortie(getLargeur(), getHauteur());
	std::vector<ImagePart> decoupeEntree = decouper(ifs.decoupeGros);
	std::vector<ImagePart> decoupeSortie = sortie.decouper(ifs.decoupePetit);

	for(int i=0 ; i<ifs.correspondances.size() ; i++) {
		int j = ifs.correspondances[i].bloc;
		Transformation transfo = ifs.correspondances[i].transformation;
		decoupeEntree[j].transformer(decoupeSortie[i], transfo);
	}
	return sortie;
}

/* *************** Enregistrement *************** */

void ImageMatricielle::sauvegarder(const char* fichier) const {
	/* Enregistre l'image au format png au nom donné
	 *  /!\ Comme on ne connais qu'une couche, l'image est enregistrée en niveaux de gris
	 */
	std::vector<unsigned char> pixels;
	for(int i=0 ; i<mLargeur ; i++) {
		for(int j=0 ; j<mHauteur ; j++) {
			pixels.push_back((unsigned char)(mImage[i][j]));
			pixels.push_back((unsigned char)(mImage[i][j]));
			pixels.push_back((unsigned char)(mImage[i][j]));
			pixels.push_back(255);
		}
	}

	std::vector<unsigned char> png;
	unsigned error = lodepng::encode(png, pixels, mLargeur, mHauteur);
	if(!error) lodepng::save_file(png, fichier);

	if(error) std::cout << fichier << " -> encoder error " << error << ": "<< lodepng_error_text(error) << std::endl;
}
