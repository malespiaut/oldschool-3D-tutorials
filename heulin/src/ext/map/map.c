/****************************************************************************/
/* MAP.C : Animation d'un objet contenu dans un fichier ASC                 */
/*         Affichage de l'objet avec textures                               */
/*         Programme par Christophe Heulin - 1997                           */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <conio.h>
#include "graphics.h"


typedef struct
{
  double x,y,z;
} point3D;


typedef struct
{
  int a,b,c;
  unsigned char couleur;
  double z;
} face;


point3D  Sommet[1000];   /* les sommets de l'objet       */
point3D  Point3D[1000];  /* les sommets apres rotation   */
pointMap Point2D[1000];  /* les sommets apres projection */

face     Facette[1000];  /* les facettes de l'objet      */
int      tri[1000];      /* tableau trie des facettes    */

int     Nb_points = 0;
int     Nb_faces  = 0;
int     Nb_visibles;


int Xoff =  160;
int Yoff =  100;
int Zoff =  500;

double Sin[360],Cos[360];   /* Tableaux precalcules de sinus et cosinus */
double matrice[3][3];       /* Matrice de rotation 3*3 */


char *textureimage;
rgb  palette[256];


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
/* Init_Palette() : initialise les couleurs utilis‚es                       */
/****************************************************************************/

void Init_Palette(void)
{
  int i;
  long couleur;

  outp(0x03C8,0);

  for(i=0;i<256;i++)
  {
    outp(0x03C9,palette[i].rouge);
    outp(0x03C9,palette[i].vert);
    outp(0x03C9,palette[i].bleu);
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
/* Visible() : determine si une face est cach‚e ou non.                     */
/****************************************************************************/

int Visible(face f)
{
  long a1,a2,b1,b2;

  a1 = Point2D[f.a].x - Point2D[f.b].x;
  b1 = Point2D[f.a].y - Point2D[f.b].y;
  a2 = Point2D[f.c].x - Point2D[f.b].x;
  b2 = Point2D[f.c].y - Point2D[f.b].y;

  if ((a1*b2-b1*a2)<0) return 0;
  return 1;
}


/****************************************************************************/
/* Quick_Sort() : tri le tableau des facettes visibles                      */
/****************************************************************************/

void Quick_Sort(int deb, int fin)
{
  int   i=deb;
  int   j=fin;
  double milieu=Facette[tri[(deb+fin)/2]].z;
  int   temp;

  while(i<=j)
  {
    while(Facette[tri[i]].z > milieu) i++;
    while(Facette[tri[j]].z < milieu) j--;

    if(i<=j)
    {
      temp=tri[i];
      tri[i]=tri[j];
      tri[j]=temp;
      i++; j--;
    }
  }

  if(i<fin) Quick_Sort(i,fin);
  if(deb<j) Quick_Sort(deb,j);
}



/****************************************************************************/
/* TrierFacettes() : classe les facettes en fonctions de la profondeur.     */
/****************************************************************************/

void TrierFacettes(void)
{
  int i;

  Nb_visibles=0;

  for(i=0;i<Nb_faces;i++)
    if(visible(Facette[i]))
    {
      Facette[i].z =  Point3D[Facette[i].a].z
		    + Point3D[Facette[i].b].z
		    + Point3D[Facette[i].c].z;

      tri[Nb_visibles]=i;
      Nb_visibles++;
    }

  Quick_Sort(0,Nb_visibles-1);
}



/****************************************************************************/
/* Afficher() : dessine la scene a l'ecran                                  */
/****************************************************************************/

void Afficher(void)
{
  pointMap points[3];
  int i;

  TrierFacettes();

  ClearBuffer();

  for(i=0;i<Nb_visibles;i++)
  {
    points[0] = Point2D[Facette[tri[i]].a];
    points[1] = Point2D[Facette[tri[i]].b];
    points[2] = Point2D[Facette[tri[i]].c];

    FillPolyTexture(points,3,textureimage);
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
  float x,y,z,u,v;
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
	sscanf(chaine+i,"%f",&x);

	while(chaine[i]!='Y') i++;
	i+=2;
	sscanf(chaine+i,"%f",&y);

	while(chaine[i]!='Z') i++;
	i+=2;
	sscanf(chaine+i,"%f",&z);

	while(chaine[i]!='U') i++;
	i+=2;
	sscanf(chaine+i,"%f",&u);

	while(chaine[i]!='V') i++;
	i+=2;
	sscanf(chaine+i,"%f",&v);

	Sommet[Nb_points].x=x;
	Sommet[Nb_points].y=y;
	Sommet[Nb_points].z=z;

	Point2D[Nb_points].u=u*320;   /* (U * largeur_texture) */
	Point2D[Nb_points].v=v*200;   /* (V * hauteur_texture) */

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

	  /* la couleur est tiree au hasard */
	  Facette[Nb_faces].couleur = (rand()%256)+1;

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
  char car;

  /* Initialisations */
  Init_Sinus();

  ChargerASC("jupiter.asc");
  textureimage=(unsigned char *)malloc(sizeof(unsigned char*)*320*200);
  LoadPic("jupiter.raw",textureimage,palette);

  xa=ya=za=0;

  /* Passage en mode graphique */
  MyInitGraph();
  Init_Palette();

  /* Animation de note cube jusqu'a pression d'une touche */
  while(1)
  {
    Rotation(xa,ya,za);
    Projection();
    Afficher();
    xa=(xa+1)%360;
    ya=(ya+2)%360;
    za=(za+3)%360;

    if(kbhit())
    {
      car=getch();
      if(car=='-') Zoff+=100;
      if(car=='+' & Zoff>300) Zoff-=100;
      if( car==27) break;
    }
  }

  /* retour mode texte */
  MyCloseGraph();
  free(textureimage);
}


