/*
 *   Ce fichier fait partie d'un projet de programmation donné en Licence 3 
 *   à l'Université de Bordeaux
 *
 *   Copyright (C) 2015 Adrien Boussicault
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
#include "outils.h"

int test_automate_accessible(){
	int result = 1;

	{
		Automate * auto1 = creer_automate();
		Automate * auto2 = creer_automate();

		ajouter_transition( auto1, 0, 'a', 1 );
		ajouter_transition( auto1, 1, 'a', 1 );
		ajouter_transition( auto1, 1, 'b', 1 );
		ajouter_transition( auto2, 0, 'a', 0 );
		ajouter_transition( auto2, 0, 'b', 1 );
		ajouter_transition( auto2, 1, 'a', 0 );
		ajouter_transition( auto2, 1, 'b', 1 );


		ajouter_etat_initial( auto1, 0);
		ajouter_etat_initial( auto2, 0);
		ajouter_etat_final( auto1, 1);
		ajouter_etat_final( auto2, 1);		

		Automate * aut = creer_union_des_automates( auto1, auto2 );
		
		print_automate(aut);
		
		TEST(
			1
			&& aut
			&& le_mot_est_reconnu( aut, "a" )
			&& le_mot_est_reconnu( aut, "aa" )
			&& le_mot_est_reconnu( aut, "bb" )
			&& le_mot_est_reconnu( aut, "b" )
			&& le_mot_est_reconnu( aut, "bbaabb" )
			&& !le_mot_est_reconnu( aut, "baaa" )
			, result
		);
		liberer_automate( aut );
		liberer_automate( auto1 );
		liberer_automate( auto2 );
	}

	return result;
}


int main(){

	if( ! test_automate_accessible() ){ return 1; };

	return 0;
	
}
