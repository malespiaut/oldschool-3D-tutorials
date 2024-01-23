/****************************************************************************/
/* OBJET.C : Animation d'un objet contenu dans un fichier ASC               */
/*           Affichage de l'objet en fil de fer sans elimination des        */
/*           des parties cach‚es - Clipping int‚gr‚ au tracage de droite    */
/*           Programme par Christophe Heulin - 1997                         */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include "graphics.h"


typedef struct
{
  double x,y,z;
} point3D;


typedef struct
{
  int a,b,c;
  int ab,bc,ac;
} face;


point3D Sommet[1000];   /* les sommets de l'objet       */
point3D Point3D[1000];  /* les sommets apres rotation   */
point   Point2D[1000];  /* les sommets apres projection */
face    Facette[1000];  /* les facettes de l'objet      */

int Xoff =  160;
int Yoff =  100;
int Zoff =  5000;

int Nb_points = 0;
int Nb_faces  = 0;

double Sin[360],Cos[360];   /* Tableaux precalcules de sinus et cosinus */
double matrice[3][3];       /* Matrice de rotation 3*3 */



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
    Point2D[i].x=((int)Point3D[i].x<<8)/(Point3D[i].z+Zoff)+Xoff;
    Point2D[i].y=((int)Point3D[i].y<<8)/(Point3D[i].z+Zoff)+Yoff;
  }
}


/****************************************************************************/
/* ligne() : appel la fonction de trace de ligne pour l'arete specifiee     */
/****************************************************************************/

void ligne(int a, int b, int couleur)
{
  Line(Point2D[a].x,Point2D[a].y,Point2D[b].x,Point2D[b].y,couleur);
}


/****************************************************************************/
/* Afficher() : dessine la scene a l'ecran                                  */
/****************************************************************************/

void Afficher(int couleur)
{
  int i;

  ClearBuffer();

  for (i=0;i<Nb_faces;i++)
  {
    if(Facette[i].ab) ligne(Facette[i].a,Facette[i].b,couleur);
    if(Facette[i].bc) ligne(Facette[i].b,Facette[i].c,couleur);
    if(Facette[i].ac) ligne(Facette[i].c,Facette[i].a,couleur);
  }

  WaitVbl();
  ShowBuffer();
}


/****************************************************************************/
/* ChargerASC() : charge en memoire un objet au format .asc                 */
/*                (format ascii de 3DS)                                     */
/*                La memoire n'est pas allouee dynamiquement dans un soucis */
/*                de lisibilit‚ (programme a but pedagogique                */
/****************************************************************************/

void ChargerASC(char *nom)
{
  FILE  *fichier;
  char  chaine[200];
  char  *fin;
  int   i,j;
  char  temp[50];
  float x,y,z;
  int   a,b,c;

  int decalage=0;

  if ((fichier = fopen(nom,"rt"))==NULL)
  {
    perror("Impossible d'ouvrir le fichier en lecture");
    exit(-2);
  }

  do
  {
    // On lit le fichier contenant les informations sur l'objet
    fin=fgets(chaine,100,fichier);
    if (!strncmp(chaine,"Vertex",6))
    {
      if (strncmp(chaine,"Vertex list",11))
      {
	// Lecture des coordonn‚es d'un point
	i=0;

	while(chaine[i]!='X') i++;
	i+=2;
	while(chaine[i]==' ') i++;
	sscanf(chaine+i,"%f",&x);

	while(chaine[i]!='Y') i++;
	i+=2;
	while(chaine[i]==' ') i++;
	sscanf(chaine+i,"%f",&y);

	while(chaine[i]!='Z') i++;
	i+=2;
	while(chaine[i]==' ') i++;
	sscanf(chaine+i,"%f",&z);

	Sommet[Nb_points].x=x;
	Sommet[Nb_points].y=y;
	Sommet[Nb_points].z=z;

	Nb_points++;
      }
    }
    else
    {
      if (!strncmp(chaine,"Face",4))
      {
	if (strncmp(chaine,"Face list",9))
	{
	  // Lecture d'une facette
	  i=j=0;
	  while(chaine[i]!='A') i++;
	  i+=2;
	  j=i;
	  while(chaine[j]!=' ') j++;
	  strncpy(temp,chaine+i,j-i);
	  temp[j-i]=0;
	  Facette[Nb_faces].a=atoi(temp)+decalage;

	  while(chaine[i]!='B') i++;
	  i+=2;
	  j=i;
	  while(chaine[j]!=' ') j++;
	  strncpy(temp,chaine+i,j-i);
	  temp[j-i]=0;
	  Facette[Nb_faces].b=atoi(temp)+decalage;

	  while(chaine[i]!='C') i++;
	  i+=2;
	  j=i;
	  while(chaine[j]!=' ') j++;
	  strncpy(temp,chaine+i,j-i);
	  temp[j-i]=0;
	  Facette[Nb_faces].c=atoi(temp)+decalage;

	  // Lecture des arˆtes visibles
	  while(chaine[i]!='A') i++;

	  strncpy(temp,chaine+i+3,1);
	  temp[j-i]=0;
	  Facette[Nb_faces].ab=atoi(temp);

	  strncpy(temp,chaine+i+8,1);
	  temp[j-i]=0;
	  Facette[Nb_faces].bc=atoi(temp);

	  strncpy(temp,chaine+i+13,1);
	  temp[j-i]=0;
	  Facette[Nb_faces].ac=atoi(temp);

	  Nb_faces++;
	}
      }
      else
	if (!strncmp(chaine,"Named object",12))
	  decalage=Nb_points;
    }
  } while(fin!=NULL);


  fclose(fichier);
}


/****************************************************************************/
/***  MAIN                                                                ***/
/****************************************************************************/

void main(int argc, char **argv)
{
  int xa,ya,za;

  if(argc<2)
  {
    printf("Syntaxe : %s <fichier.asc>\n",argv[0]);
    exit(-1);
  }

  /* Initialisations */
  Init_Sinus();
  ChargerASC(argv[1]);
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


