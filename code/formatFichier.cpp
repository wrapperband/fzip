#include "formatFichier.h"

/* *************** Flotant 16 bits *************** */

float decode16bFloat(Flotant16b x) {
	/* Retourne un flotant 32 bits (natif) à partir d'un flotant 16 bits */
	return x.mantisse * std::pow(2, x.exp + DECALAGE_EXPOSANT) / TAILLE_MANTISSE;
}

/* *************** Fonctions de mise en paquets *************** */

Pack_Entete packer_entete(const ImageFractale& image) {
	Pack_Entete retour;
		retour.largeur = image.getLargeur();
		retour.hauteur = image.getHauteur();
		retour.couleur = image.isCouleur();
		retour.transparence = image.isTransparent();
	return retour;
}

Pack_IFS packer_ifs(const IFS& ifs, unsigned char moyenne) {
	Pack_IFS retour;
		retour.decoupeGros = ifs.decoupeGros;
		retour.decoupePetit = ifs.decoupePetit;
		retour.moyenne = moyenne;
	return retour;
}

Pack_Correspondance packer_correspondance(const Correspondance& correspondance) {
	Pack_Correspondance retour;
		retour.bloc = correspondance.bloc;
		retour.rotation = correspondance.transformation.rotation;
		retour.a = correspondance.transformation.droite.a;
		retour.b = correspondance.transformation.droite.b;
	return retour;
}

/* *************** Fonctions d'extraction des paquets *************** */

unsigned char unpack_moyenne(const Pack_IFS& paquet) {
	return paquet.moyenne;
}

IFS unpack_IFS(const Pack_IFS& paquet) {
	IFS retour;
		retour.decoupeGros = paquet.decoupeGros;
		retour.decoupePetit = paquet.decoupePetit;
	return retour;
}

Correspondance unpack_correspondance(const Pack_Correspondance& paquet) {
	Correspondance retour;
		retour.bloc = paquet.bloc;
		retour.transformation.rotation = paquet.rotation;
		retour.transformation.droite.b = paquet.b;
		retour.transformation.droite.a = decode16bFloat(paquet.a);
	return retour;
}
