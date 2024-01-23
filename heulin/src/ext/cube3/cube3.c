/****************************************************************************/
/* CUBE3.C : exemple d'animation d'un cube en fil de fer                    */
/*           Programme par Christophe Heulin - 1997                         */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include "graphics.h"


typedef struct
{
  int x,y,z;
} point3D;


typedef struct
{
  int x,y;
} point2D;


point3D Sommet[8];   /* les sommets du cube          */
point3D Point3D[8];  /* les sommets apres rotation   */
point2D Point2D[8];  /* les sommets apres projection */


int Nb_points = 8;

int Xoff = 160;
int Yoff = 100;
int Zoff = 600;


float Sin[360],Cos[360];   /* Tableaux precalcules de sinus et cosinus */
float matrice[3][3];       /* Matrice de rotation 3*3 */



/****************************************************************************/
/* Init_Sinus() : precalcul les tables de sinus et cosinus                  */
/****************************************************************************/

void Init_Sinus(void)
{
  int i;

  for(i=0;i<360;i++)
  {
    Sin[i]=sin(i * 3.1415927 / 180);
    Cos[i]=cos(i * 3.1415927 / 180);
  }
}


/****************************************************************************/
/* Rotation() : effectue la rotation des points Sommet -> Point3D           */
/****************************************************************************/

void Rotation(int Xa, int Ya, int Za)
{
  int i;

  /* Calcul de la matrice de rotation 3*3 */

  matrice[0][0] = Cos[Za]*Cos[Ya];
  matrice[1][0] = Sin[Za]*Cos[Ya];
  matrice[2][0] = -Sin[Ya];

  matrice[0][1] = Cos[Za]*Sin[Ya]*Sin[Xa] - Sin[Za]*Cos[Xa];
  matrice[1][1] = Sin[Za]*Sin[Ya]*Sin[Xa] + Cos[Xa]*Cos[Za];
  matrice[2][1] = Sin[Xa]*Cos[Ya];

  matrice[0][2] = Cos[Za]*Sin[Ya]*Cos[Xa] + Sin[Za]*Sin[Xa];
  matrice[1][2] = Sin[Za]*Sin[Ya]*Cos[Xa] - Cos[Za]*Sin[Xa];
  matrice[2][2] = Cos[Xa]*Cos[Ya];


  /* Rotation des sommets de l'objet */

  for(i=0;i<Nb_points;i++)
  {
    Point3D[i].x =   matrice[0][0]*Sommet[i].x
		   + matrice[1][0]*Sommet[i].y
		   + matrice[2][0]*Sommet[i].z;

    Point3D[i].y =   matrice[0][1]*Sommet[i].x
		   + matrice[1][1]*Sommet[i].y
		   + matrice[2][1]*Sommet[i].z;

    Point3D[i].z =   matrice[0][2]*Sommet[i].x
		   + matrice[1][2]*Sommet[i].y
		   + matrice[2][2]*Sommet[i].z;
  }

}


/****************************************************************************/
/* Projection() : projette en perspective les points aprŠs rotation.        */
/****************************************************************************/

void Projection(void)
{
  int i;

  for(i=0;i<Nb_points;i++)
  {
    Point2D[i].x=(Point3D[i].x<<8)/(Point3D[i].z+Zoff)+Xoff;
    Point2D[i].y=(Point3D[i].y<<8)/(Point3D[i].z+Zoff)+Yoff;
  }
}


/****************************************************************************/
/* Initialiser() : initialise les coordonnees des sommets du cube           */
/****************************************************************************/

void Initialiser(void)
{
  Sommet[0].x = -100;  Sommet[0].y = -100;  Sommet[0].z = -100;
  Sommet[1].x =  100;  Sommet[1].y = -100;  Sommet[1].z = -100;
  Sommet[2].x =  100;  Sommet[2].y =  100;  Sommet[2].z = -100;
  Sommet[3].x = -100;  Sommet[3].y =  100;  Sommet[3].z = -100;
  Sommet[4].x =  100;  Sommet[4].y = -100;  Sommet[4].z =  100;
  Sommet[5].x = -100;  Sommet[5].y = -100;  Sommet[5].z =  100;
  Sommet[6].x = -100;  Sommet[6].y =  100;  Sommet[6].z =  100;
  Sommet[7].x =  100;  Sommet[7].y =  100;  Sommet[7].z =  100;
}



void ligne(int a, int b, int couleur)
{
  Line(Point2D[a].x,Point2D[a].y,Point2D[b].x,Point2D[b].y,couleur);
}



void FilDeFer(int couleur)
{
 /* On affiche la face avant */
 ligne(0,1,couleur); ligne(1,2,couleur);
 ligne(2,3,couleur); ligne(3,0,couleur);

 /* Puis la face arriere */
 ligne(4,5,couleur); ligne(5,6,couleur);
 ligne(6,7,couleur); ligne(7,4,couleur);

 /* Et enfin les aretes restantes */
 ligne(0,5,couleur);
 ligne(1,4,couleur);
 ligne(2,7,couleur);
 ligne(3,6,couleur);
}


/****************************************************************************/
/* Afficher() : dessine la scene a l'ecran                                  */
/****************************************************************************/

void Afficher(int couleur)
{
  int i;

  /* On efface le contenu de l'ecran virtuel */
  ClearBuffer();

  /* On dessine dedans */
  FilDeFer(couleur);

  /* On affiche le tout a l'ecran, en synchonisant */
  WaitVbl();
  ShowBuffer();
}


/****************************************************************************/
/***  MAIN                                                                ***/
/****************************************************************************/

void main()
{
  int xa,ya,za;

  /* Initialisations */
  Init_Sinus();
  Initialiser();
  xa=ya=za=0;

  /* Passage en mode graphique */
  MyInitGraph();

  /* Animation de note cube jusqu'a pression d'une touche */
  while(!kbhit())
  {
    Rotation(xa,ya,za);
    Projection();
    Afficher(100);
    xa=(xa+1)%360;
    ya=(ya+3)%360;
    za=(za+1)%360;
  }

  /* retour mode texte */
  MyCloseGraph();
}


