/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2014, 2015 Adrien Boussicault
 *
 *    This Library is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This Library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this Library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "automate.h"
#include "table.h"
#include "ensemble.h"
#include "outils.h"
#include "fifo.h"

#include <search.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h> 

#include <assert.h>

#include <math.h>


void action_get_max_etat( const intptr_t element, void* data ){
	int * max = (int*) data;
	if( *max < element ) *max = element;
}

int get_max_etat( const Automate* automate ){
	int max = INT_MIN;

	pour_tout_element( automate->etats, action_get_max_etat, &max );

	return max;
}

void action_get_min_etat( const intptr_t element, void* data ){
	int * min = (int*) data;
	if( *min > element ) *min = element;
}

int get_min_etat( const Automate* automate ){
	int min = INT_MAX;

	pour_tout_element( automate->etats, action_get_min_etat, &min );

	return min;
}


int comparer_cle(const Cle *a, const Cle *b) {
	if( a->origine < b->origine )
		return -1;
	if( a->origine > b->origine )
		return 1;
	if( a->lettre < b->lettre )
		return -1;
	if( a->lettre > b->lettre )
		return 1;
	return 0;
}

void print_cle( const Cle * a){
	printf( "(%d, %c)" , a->origine, (char) (a->lettre) );
}

void supprimer_cle( Cle* cle ){
	xfree( cle );
}

void initialiser_cle( Cle* cle, int origine, char lettre ){
	cle->origine = origine;
	cle->lettre = (int) lettre;
}

Cle * creer_cle( int origine, char lettre ){
	Cle * result = xmalloc( sizeof(Cle) );
	initialiser_cle( result, origine, lettre );
	return result;
}

Cle * copier_cle( const Cle* cle ){
	return creer_cle( cle->origine, cle->lettre );
}

Automate * creer_automate(){
	Automate * automate = xmalloc( sizeof(Automate) );
	automate->etats = creer_ensemble( NULL, NULL, NULL );
	automate->alphabet = creer_ensemble( NULL, NULL, NULL );
	automate->transitions = creer_table(
		( int(*)(const intptr_t, const intptr_t) ) comparer_cle , 
		( intptr_t (*)( const intptr_t ) ) copier_cle,
		( void(*)(intptr_t) ) supprimer_cle
	);
	automate->initiaux = creer_ensemble( NULL, NULL, NULL );
	automate->finaux = creer_ensemble( NULL, NULL, NULL );
	automate->vide = creer_ensemble( NULL, NULL, NULL ); 
	return automate;
}

Automate * translater_automate_entier( const Automate* automate, int translation ){
	Automate * res = creer_automate();

	Ensemble_iterateur it;
	for( 
		it = premier_iterateur_ensemble( get_etats( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat( res, get_element( it ) + translation );
	}

	for( 
		it = premier_iterateur_ensemble( get_initiaux( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat_initial( res, get_element( it ) + translation );
	}

	for( 
		it = premier_iterateur_ensemble( get_finaux( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_etat_final( res, get_element( it ) + translation );
	}

	// On ajoute les lettres
	for(
		it = premier_iterateur_ensemble( get_alphabet( automate ) );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		ajouter_lettre( res, (char) get_element( it ) );
	}

	Table_iterateur it1;
	Ensemble_iterateur it2;
	for(
		it1 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it1 );
		it1 = iterateur_suivant_table( it1 )
	){
		Cle * cle = (Cle*) get_cle( it1 );
		Ensemble * fins = (Ensemble*) get_valeur( it1 );
		for(
			it2 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int fin = get_element( it2 );
			ajouter_transition(
				res, cle->origine + translation, cle->lettre, fin + translation
			);
		}
	};

	return res;
}


void liberer_automate( Automate * automate ){
	assert( automate );
	liberer_ensemble( automate->vide );
	liberer_ensemble( automate->finaux );
	liberer_ensemble( automate->initiaux );
	pour_toute_valeur_table(
		automate->transitions, ( void(*)(intptr_t) ) liberer_ensemble
	);
	liberer_table( automate->transitions );
	liberer_ensemble( automate->alphabet );
	liberer_ensemble( automate->etats );
	xfree(automate);
}

const Ensemble * get_etats( const Automate* automate ){
	return automate->etats;
}

const Ensemble * get_initiaux( const Automate* automate ){
	return automate->initiaux;
}

const Ensemble * get_finaux( const Automate* automate ){
	return automate->finaux;
}

const Ensemble * get_alphabet( const Automate* automate ){
	return automate->alphabet;
}

void ajouter_etat( Automate * automate, int etat ){
	ajouter_element( automate->etats, etat );
}

void ajouter_lettre( Automate * automate, char lettre ){
	ajouter_element( automate->alphabet, lettre );
}

void ajouter_transition(
	Automate * automate, int origine, char lettre, int fin
){
	ajouter_etat( automate, origine );
	ajouter_etat( automate, fin );
	ajouter_lettre( automate, lettre );

	Cle cle;
	initialiser_cle( &cle, origine, lettre );
	Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
	Ensemble * ens;
	if( iterateur_est_vide( it ) ){
		ens = creer_ensemble( NULL, NULL, NULL );
		add_table( automate->transitions, (intptr_t) &cle, (intptr_t) ens );
	}else{
		ens = (Ensemble*) get_valeur( it );
	}
	ajouter_element( ens, fin );
}

void ajouter_etat_final(
	Automate * automate, int etat_final
){
	ajouter_etat( automate, etat_final );
	ajouter_element( automate->finaux, etat_final );
}

void ajouter_etat_initial(
	Automate * automate, int etat_initial
){
	ajouter_etat( automate, etat_initial );
	ajouter_element( automate->initiaux, etat_initial );
}

const Ensemble * voisins( const Automate* automate, int origine, char lettre ){
	Cle cle;
	initialiser_cle( &cle, origine, lettre );
	Table_iterateur it = trouver_table( automate->transitions, (intptr_t) &cle );
	if( ! iterateur_est_vide( it ) ){
		return (Ensemble*) get_valeur( it );
	}else{
		return automate->vide;
	}
}

Ensemble * delta1(
	const Automate* automate, int origine, char lettre
){
	Ensemble * res = creer_ensemble( NULL, NULL, NULL );
	ajouter_elements( res, voisins( automate, origine, lettre ) );
	return res; 
}

Ensemble * delta(
	const Automate* automate, const Ensemble * etats_courants, char lettre
){
	Ensemble * res = creer_ensemble( NULL, NULL, NULL );

	Ensemble_iterateur it;
	for( 
		it = premier_iterateur_ensemble( etats_courants );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		const Ensemble * fins = voisins(
			automate, get_element( it ), lettre
		);
		ajouter_elements( res, fins );
	}

	return res;
}

Ensemble * delta_star(
	const Automate* automate, const Ensemble * etats_courants, const char* mot
){
	int len = strlen( mot );
	int i;
	Ensemble * old = copier_ensemble( etats_courants );
	Ensemble * new = old;
	for( i=0; i<len; i++ ){
		new = delta( automate, old, *(mot+i) );
		liberer_ensemble( old );
		old = new;
	}
	return new;
}

void pour_toute_transition(
	const Automate* automate,
	void (* action )( int origine, char lettre, int fin, void* data ),
	void* data
){
	Table_iterateur it1;
	Ensemble_iterateur it2;
	for(
		it1 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it1 );
		it1 = iterateur_suivant_table( it1 )
	){
		Cle * cle = (Cle*) get_cle( it1 );
		Ensemble * fins = (Ensemble*) get_valeur( it1 );
		for(
			it2 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it2 );
			it2 = iterateur_suivant_ensemble( it2 )
		){
			int fin = get_element( it2 );
			action( cle->origine, cle->lettre, fin, data );
		}
	};
}

Automate* copier_automate( const Automate* automate ){
	Automate * res = creer_automate();
	Ensemble_iterateur it1;
	// On ajoute les états de l'automate
	for(
		it1 = premier_iterateur_ensemble( get_etats( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat( res, get_element( it1 ) );
	}
	// On ajoute les états initiaux
	for(
		it1 = premier_iterateur_ensemble( get_initiaux( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat_initial( res, get_element( it1 ) );
	}
	// On ajoute les états finaux
	for(
		it1 = premier_iterateur_ensemble( get_finaux( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_etat_final( res, get_element( it1 ) );
	}
	// On ajoute les lettres
	for(
		it1 = premier_iterateur_ensemble( get_alphabet( automate ) );
		! iterateur_ensemble_est_vide( it1 );
		it1 = iterateur_suivant_ensemble( it1 )
	){
		ajouter_lettre( res, (char) get_element( it1 ) );
	}
	// On ajoute les transitions
	Table_iterateur it2;
	for(
		it2 = premier_iterateur_table( automate->transitions );
		! iterateur_est_vide( it2 );
		it2 = iterateur_suivant_table( it2 )
	){
		Cle * cle = (Cle*) get_cle( it2 );
		Ensemble * fins = (Ensemble*) get_valeur( it2 );
		for(
			it1 = premier_iterateur_ensemble( fins );
			! iterateur_ensemble_est_vide( it1 );
			it1 = iterateur_suivant_ensemble( it1 )
		){
			int fin = get_element( it1 );
			ajouter_transition( res, cle->origine, cle->lettre, fin );
		}
	}
	return res;
}

Automate * translater_automate(
	const Automate * automate, const Automate * automate_a_eviter
){
	if(
		taille_ensemble( get_etats(automate) ) == 0 ||
		taille_ensemble( get_etats(automate_a_eviter) ) == 0
	){
		return copier_automate( automate );
	}
	
	int translation = 
		get_max_etat( automate_a_eviter ) - get_min_etat( automate ) + 1; 

	return translater_automate_entier( automate, translation );
	
}

int est_une_transition_de_l_automate(
	const Automate* automate,
	int origine, char lettre, int fin
){
	return est_dans_l_ensemble( voisins( automate, origine, lettre ), fin );
}

int est_un_etat_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_etats( automate ), etat );
}

int est_un_etat_initial_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_initiaux( automate ), etat );
}

int est_un_etat_final_de_l_automate( const Automate* automate, int etat ){
	return est_dans_l_ensemble( get_finaux( automate ), etat );
}

int est_une_lettre_de_l_automate( const Automate* automate, char lettre ){
	return est_dans_l_ensemble( get_alphabet( automate ), lettre );
}

void print_ensemble_2( const intptr_t ens ){
	print_ensemble( (Ensemble*) ens, NULL );
}

void print_lettre( intptr_t c ){
	printf("%c", (char) c );
}

void print_automate( const Automate * automate ){
	printf("- Etats : ");
	print_ensemble( get_etats( automate ), NULL );
	printf("\n- Initiaux : ");
	print_ensemble( get_initiaux( automate ), NULL );
	printf("\n- Finaux : ");
	print_ensemble( get_finaux( automate ), NULL );
	printf("\n- Alphabet : ");
	print_ensemble( get_alphabet( automate ), print_lettre );
	printf("\n- Transitions : ");
	print_table( 
		automate->transitions,
		( void (*)( const intptr_t ) ) print_cle, 
		( void (*)( const intptr_t ) ) print_ensemble_2,
		""
	);
	printf("\n");
}

int le_mot_est_reconnu( const Automate* automate, const char* mot ){
	Ensemble * arrivee = delta_star( automate, get_initiaux(automate) , mot ); 
	
	int result = 0;

	Ensemble_iterateur it;
	for(
		it = premier_iterateur_ensemble( arrivee );
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		if( est_un_etat_final_de_l_automate( automate, get_element(it) ) ){
			result = 1;
			break;
		}
	}
	liberer_ensemble( arrivee );
	return result;
}

Automate * mot_to_automate( const char * mot ){
	Automate * automate = creer_automate();
	int i = 0;
	int size = strlen( mot );
	for( i=0; i < size; i++ ){
		ajouter_transition( automate, i, mot[i], i+1 );
	}
	ajouter_etat_initial( automate, 0 );
	ajouter_etat_final( automate, size );
	return automate;
}

void action_creer_union_des_automates ( int origine, char lettre, int fin, void * data ){
	ajouter_transition((Automate*)data, origine, lettre, fin);
}

Automate * creer_union_des_automates(const Automate * automate_1, const Automate * automate_2){
	//On évite les doublons
	Automate * automate_2_trans = translater_automate(automate_2, automate_1);
	Automate * automate_resultat = creer_automate();
	//On ajoute les états, les initiaux et les finaux des deux automates
	transferer_elements_et_libere(automate_resultat->etats,creer_union_ensemble(get_etats(automate_1), get_etats(automate_2_trans)));
	transferer_elements_et_libere(automate_resultat->initiaux,creer_union_ensemble(get_initiaux(automate_1), get_initiaux(automate_2_trans)));
	transferer_elements_et_libere(automate_resultat->finaux,creer_union_ensemble(get_finaux(automate_1), get_finaux(automate_2_trans)));
	//On ajoute les transitions des deux automates
	pour_toute_transition(automate_1, action_creer_union_des_automates, automate_resultat);
	pour_toute_transition(automate_2_trans, action_creer_union_des_automates, automate_resultat);
	liberer_automate(automate_2_trans);
	return automate_resultat;
}

Ensemble* etats_accessibles( const Automate * automate, int etat ){
	Ensemble * etats = creer_ensemble( NULL, NULL, NULL );
	ajouter_element(etats, etat);
	
	Ensemble_iterateur it_etats;
	for(
		it_etats = premier_iterateur_ensemble( etats);
		! iterateur_ensemble_est_vide( it_etats );
		it_etats = iterateur_suivant_ensemble( it_etats )
	){
		Ensemble_iterateur it_lettres;
		for(
			it_lettres = premier_iterateur_ensemble(get_alphabet(automate));
			! iterateur_ensemble_est_vide(it_lettres);
			it_lettres = iterateur_suivant_ensemble( it_lettres)
		){
			//Pour chaque état et chaque lettre, on récupère les voisins et on réitère l'opération sur ces-derniers
			transferer_elements_et_libere( etats, delta(automate, etats, (char)get_element(it_lettres))); 
		}
	}
	return etats;
}

Ensemble* accessibles( const Automate * automate ){
	Ensemble * etats = creer_ensemble( NULL, NULL, NULL );
	
	Ensemble_iterateur it;
	for(
		it = premier_iterateur_ensemble( get_initiaux(automate ));
		! iterateur_ensemble_est_vide( it );
		it = iterateur_suivant_ensemble( it )
	){
		//On cherche les états accessibles depuis les initiaux
		transferer_elements_et_libere( etats, etats_accessibles(automate, (int)get_element(it))); 
	}
	return etats;
}

void action_automate_accessible( int origine, char lettre, int fin, void * data ){
	//Pour savoir si une transition sera dans l'automate accessible, il faut que son origine et sa fin soient accessibles
	if(est_un_etat_de_l_automate((Automate*)data, origine)
		&& est_un_etat_de_l_automate((Automate*)data, fin))
		ajouter_transition((Automate*)data, origine, lettre, fin);
}

Automate *automate_accessible( const Automate * automate ){
	Automate * automate_resultat = creer_automate();
	//L'alphabet et les initiaux ne changent pas
	transferer_elements_et_libere(automate_resultat->alphabet, copier_ensemble(get_alphabet(automate)));
	transferer_elements_et_libere(automate_resultat->initiaux, copier_ensemble(get_initiaux(automate)));
	//On veut seulement les états accessibles
	transferer_elements_et_libere(automate_resultat->etats, accessibles(automate));
	//On conserve uniquement les transitions qui ne concernent que les états accessibles
	pour_toute_transition(automate, action_automate_accessible, automate_resultat);
	//On ajoute les finaux accessibles
	transferer_elements_et_libere(automate_resultat->finaux, creer_intersection_ensemble(get_etats(automate_resultat), get_finaux(automate)));
	return automate_resultat;
}

void action_miroir( int origine, char lettre, int fin, void * data ){
	ajouter_transition((Automate *) data, fin, lettre, origine);
}

Automate *miroir( const Automate * automate){
	Automate * automate_resultat = creer_automate();
	//Les états et l'alphabet ne changent pas
	transferer_elements_et_libere(automate_resultat->etats, copier_ensemble(get_etats(automate)));
	transferer_elements_et_libere(automate_resultat->alphabet, copier_ensemble(get_alphabet(automate)));
	//Les initiaux et les finaux s'inversent
	transferer_elements_et_libere(automate_resultat->initiaux, copier_ensemble(get_finaux(automate)));
	transferer_elements_et_libere(automate_resultat->finaux, copier_ensemble(get_initiaux(automate)));
	//On inverse chaque transition
	pour_toute_transition(automate, action_miroir, automate_resultat);
	return automate_resultat;
}

//Cette structure permettra d'associer chaque état du nouvel automate
//	aux états auxquels il correspond dans les automates 1 et 2
typedef struct Couple Couple;
struct Couple{
	int etat_automate_1;
	int etat_automate_2;
};

//Cette strucure permet d'enrichir le paramètre data des actions
typedef struct My_data * My_data;
struct My_data{
	Couple * md_couples;
	Automate * md_automate;
};

void action_creer_automate_du_melange_copier_transitions_automate_1( int origine, char lettre, int fin, void * data ){
	Ensemble_iterateur it_etats_origine;
	for(
		it_etats_origine = premier_iterateur_ensemble( get_etats(((My_data)data)->md_automate));
		! iterateur_ensemble_est_vide( it_etats_origine );
		it_etats_origine = iterateur_suivant_ensemble( it_etats_origine )
	){
		//Pour chaque état de l'automate résultat, on regarde si l'état qui lui correspond dans l'automate 1 est notre origine
		if(((My_data)data)->md_couples[get_element(it_etats_origine)].etat_automate_1==origine){
			Ensemble_iterateur it_etats_fin;
			for(
				it_etats_fin = premier_iterateur_ensemble( get_etats(((My_data)data)->md_automate));
				! iterateur_ensemble_est_vide( it_etats_fin );
				it_etats_fin = iterateur_suivant_ensemble( it_etats_fin )
			){
				//On parcours une nouvelle fois les états de l'automate résultat, où on cherche la fin
				//en s'assurant que l'état correspondant de l'automate 2 n'a pas changé
				//puisque dans cette fonction on ne traite que les transitions de l'automate 1
				if(((My_data)data)->md_couples[get_element(it_etats_fin)].etat_automate_1==fin
				  && ((My_data)data)->md_couples[get_element(it_etats_origine)].etat_automate_2
				  ==((My_data)data)->md_couples[get_element(it_etats_fin)].etat_automate_2){
					ajouter_transition(((My_data)data)->md_automate,get_element(it_etats_origine),lettre,get_element(it_etats_fin));
				}
			}
		}
	}
}

void action_creer_automate_du_melange_copier_transitions_automate_2( int origine, char lettre, int fin, void * data ){
	Ensemble_iterateur it_etats_origine;
	for(
		it_etats_origine = premier_iterateur_ensemble( get_etats(((My_data)data)->md_automate));
		! iterateur_ensemble_est_vide( it_etats_origine );
		it_etats_origine = iterateur_suivant_ensemble( it_etats_origine )
	){
		//Pour chaque état de l'automate résultat, on regarde si l'état qui lui correspond dans l'automate 2 est notre origine
		if(((My_data)data)->md_couples[get_element(it_etats_origine)].etat_automate_2==origine){
			Ensemble_iterateur it_etats_fin;
			for(
				it_etats_fin = premier_iterateur_ensemble( get_etats(((My_data)data)->md_automate));
				! iterateur_ensemble_est_vide( it_etats_fin );
				it_etats_fin = iterateur_suivant_ensemble( it_etats_fin )
			){
				//On parcours une nouvelle fois les états de l'automate résultat, où on cherche la fin
				//en s'assurant que l'état correspondant de l'automate 1 n'a pas changé
				//puisque dans cette fonction on ne traite que les transitions de l'automate 2
				if(((My_data)data)->md_couples[get_element(it_etats_fin)].etat_automate_2==fin
				  && ((My_data)data)->md_couples[get_element(it_etats_origine)].etat_automate_1
				  ==((My_data)data)->md_couples[get_element(it_etats_fin)].etat_automate_1){
					ajouter_transition(((My_data)data)->md_automate,get_element(it_etats_origine),lettre,get_element(it_etats_fin));
				}
			}
		}
	}
}

Automate * creer_automate_du_melange(const Automate* automate_1,  const Automate* automate_2){
	//On évite les doublons
	Automate * automate_2_trans = translater_automate(automate_2, automate_1);
	Automate * automate_resultat = creer_automate();
	//Le tableau couples contiendra toutes les correspondances entre les nouveaux états et ceux auxquels ils correspondent
	Couple * couples = malloc(sizeof(Couple)*taille_ensemble(get_etats(automate_1))*taille_ensemble(get_etats(automate_2_trans)));
	int nb_etats = 0;
	//Etats + Initiaux + Finaux
	Couple tmp;
	Ensemble_iterateur it_etats_automate_1;
	for(
		it_etats_automate_1 = premier_iterateur_ensemble( get_etats(automate_1 ));
		! iterateur_ensemble_est_vide( it_etats_automate_1 );
		it_etats_automate_1 = iterateur_suivant_ensemble( it_etats_automate_1 )
	){
		Ensemble_iterateur it_etats_automate_2;
		for(
			it_etats_automate_2 = premier_iterateur_ensemble( get_etats(automate_2_trans ));
			! iterateur_ensemble_est_vide( it_etats_automate_2 );
			it_etats_automate_2 = iterateur_suivant_ensemble( it_etats_automate_2 )
		){
			//Pour chaque couple d'état possible entre les automates 1 et 2, on ajoute un état
			tmp.etat_automate_1 = get_element(it_etats_automate_1);
			tmp.etat_automate_2 = get_element(it_etats_automate_2);
			//Si les deux étaient initiaux/finaux notre état sera initial/final
			if(est_un_etat_initial_de_l_automate(automate_1, tmp.etat_automate_1) 
			  && est_un_etat_initial_de_l_automate(automate_2_trans, tmp.etat_automate_2)){
				ajouter_etat_initial(automate_resultat, nb_etats);
			}
			else if(est_un_etat_final_de_l_automate(automate_1, tmp.etat_automate_1)
			  && est_un_etat_final_de_l_automate(automate_2_trans, tmp.etat_automate_2)){
				ajouter_etat_final(automate_resultat, nb_etats);
			}
			else{
				ajouter_etat(automate_resultat, nb_etats);
			}
			//On conserve sa correspondance avec le couple auxquel il correspond
			couples[nb_etats++]=tmp;
		}
	}
	//L'alphabet est l'union des deux alphabets
	transferer_elements_et_libere(automate_resultat->alphabet, creer_union_ensemble(get_alphabet(automate_1), get_alphabet(automate_2_trans)));
	// Transitions
	//On utilise la structure My_data pour avoir non seulement nos états mais aussi leur couple correspondant
	My_data md = malloc(sizeof(struct My_data));
	md->md_couples = couples;
	md->md_automate = automate_resultat;
	//Pour chaque transition des automates 1 et 2, on va chercher où elle intervient dans notre automate résultat
	// Le traitement, bien que similaire, reste légèrement différent selon si on veut copier la transition depuis l'automate 1 ou le 2
	// Cela semble pourtant relativement "sale" vu qu'il y a beaucoup de duplication de code, comment aurions-nous pu faire ?
	pour_toute_transition(automate_1,action_creer_automate_du_melange_copier_transitions_automate_1,md);
	pour_toute_transition(automate_2_trans,action_creer_automate_du_melange_copier_transitions_automate_2,md);
	free(couples);	
	free(md);
	liberer_automate(automate_2_trans);	
	return automate_resultat;
}

