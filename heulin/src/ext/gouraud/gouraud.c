/****************************************************************************/
/*                                                                          */
/* GOURAUD.C : Animation d'un objet contenu dans un fichier ASC             */
/*             Faces cachees et rendu en utilisant la methode de Gouraud    */
/*             Programme par Christophe Heulin - 1997                       */
/*                                                                          */
/****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>
#include "graphics.h"


typedef struct
{
  int   x,y,z;

  float nx,ny,nz;          /* 'Vecteur normal' du sommet      */
  float nx2,ny2,nz2;       /* 'Vecteur normal' aprŠs rotation */
  float norme;
} point3D;


typedef struct
{
  int a,b,c;
  float z;

  /* On stocke les normales */
  float nx,ny,nz;              /* Vecteur normal de la face     */
  float nx2,ny2,nz2;           /* Vecteur normal aprŠs rotation */
  float norme;                 /* norme (invariable)            */
} face;


point3D Sommet[3000];   /* les sommets de l'objet       */
point3D Point3D[3000];  /* les sommets apres rotation   */
point   Point2D[3000];  /* les sommets apres projection */

face    Facette[5000];  /* les facettes de l'objet      */
int     tri[5000];      /* tableau trie des facettes    */

int     Nb_points = 0;
int     Nb_faces  = 0;
int     Nb_visibles;


int Xoff =  160;
int Yoff =  100;
int Zoff =  5000;

float Sin[360],Cos[360];   /* Tableaux precalcules de sinus et cosinus */
float matrice[3][3];       /* Matrice de rotation 3*3 */

int   lumiere[3];           /* le vecteur lumiere */
long  normelumiere;         /* et sa norme        */


/****************************************************************************/
/* InitSinus() : precalcul les tables de sinus et cosinus                   */
/****************************************************************************/

void InitSinus(void)
{
  int i;

  for(i=0;i<360;i++)
  {
    Sin[i]=sin(i * 3.1415927 / 180);
    Cos[i]=cos(i * 3.1415927 / 180);
  }
}


/****************************************************************************/
/* InitPalette() : initialise les couleurs utilis‚es                        */
/****************************************************************************/

void InitPalette(void)
{
  int i;
  long couleur=0x3F003F;

  SetColor(66,0);

  for(i=1;i<34;i++)
  {
    SetColor(i,couleur);
    couleur-=1;
  }

  for(i=34;i<65;i++)
  {
    SetColor(i,couleur);
    couleur-=0x010001;
  }

  SetColor(i,0x1F0000);
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

    Point3D[i].nx2 =   matrice[0][0]*Sommet[i].nx
		     + matrice[1][0]*Sommet[i].ny
		     + matrice[2][0]*Sommet[i].nz;

    Point3D[i].ny2 =   matrice[0][1]*Sommet[i].nx
		     + matrice[1][1]*Sommet[i].ny
		     + matrice[2][1]*Sommet[i].nz;

    Point3D[i].nz2 =   matrice[0][2]*Sommet[i].nx
		     + matrice[1][2]*Sommet[i].ny
		     + matrice[2][2]*Sommet[i].nz;
  }

  for(i=0;i<Nb_faces;i++)
  {
    Facette[i].nx2=  matrice[0][0]*Facette[i].nx
		   + matrice[1][0]*Facette[i].ny
		   + matrice[2][0]*Facette[i].nz;

    Facette[i].ny2=  matrice[0][1]*Facette[i].nx
		   + matrice[1][1]*Facette[i].ny
		   + matrice[2][1]*Facette[i].nz;

    Facette[i].nz2=  matrice[0][2]*Facette[i].nx
		   + matrice[1][2]*Facette[i].ny
		   + matrice[2][2]*Facette[i].nz;
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
/* Calcul_Intensite() : determine la couleur de la facette                  */
/****************************************************************************/

int Calcul_Intensite(int i)
{
  int res;

  res= - (  Point3D[i].nx2*lumiere[0]
	  + Point3D[i].ny2*lumiere[1]
	  + Point3D[i].nz2*lumiere[2]) * 64
	 / (normelumiere*Sommet[i].norme);

  if (res>0) return 65;
  else if(res!=0) return res+66;
  else return 65;
}


/****************************************************************************/
/* Remplir() : appelle la fonction de remplissage de polygone               */
/****************************************************************************/

void Remplir(int i)
{
  point p[3];

  p[0] = Point2D[Facette[i].a];
  p[1] = Point2D[Facette[i].b];
  p[2] = Point2D[Facette[i].c];

  p[0].couleur = Calcul_Intensite(Facette[i].a);
  p[1].couleur = Calcul_Intensite(Facette[i].b);
  p[2].couleur = Calcul_Intensite(Facette[i].c);

  FillPolyGouraud(p,3);
}


/****************************************************************************/
/* Afficher() : dessine la scene a l'ecran                                  */
/****************************************************************************/

void Afficher(void)
{
  int i;

  TrierFacettes();

  ClearBuffer();

  for(i=0;i<Nb_visibles;i++)
    Remplir(tri[i]);

/*  WaitVbl();*/
  ShowBuffer();
}


/****************************************************************************/
/* FixerLumiere() : initialise le vecteur lumineux. Attention !             */
/*                  on inverse le sens du vecteur pour calculer             */
/*                  le produit scalaire avec la normale des facettes        */
/****************************************************************************/

void FixerLumiere(int x,int y,int z)
{
  lumiere[0] = -x;
  lumiere[1] = -y;
  lumiere[2] = -z;

  normelumiere = sqrt( lumiere[0]*lumiere[0]
		      +lumiere[1]*lumiere[1]
		      +lumiere[2]*lumiere[2]);
}


/****************************************************************************/
/* CalculerNormales() :                                                     */
/****************************************************************************/

void CalculerNormales()
{
  double a[3],b[3];
  double nx,ny,nz;
  int    nb;
  int    i,j;

  for(i=0;i<Nb_faces;i++)
  {
    a[0] = (Sommet[Facette[i].a].x-Sommet[Facette[i].b].x);
    a[1] = (Sommet[Facette[i].a].y-Sommet[Facette[i].b].y);
    a[2] = (Sommet[Facette[i].a].z-Sommet[Facette[i].b].z);

    b[0] = (Sommet[Facette[i].a].x-Sommet[Facette[i].c].x);
    b[1] = (Sommet[Facette[i].a].y-Sommet[Facette[i].c].y);
    b[2] = (Sommet[Facette[i].a].z-Sommet[Facette[i].c].z);


    /* produit vectoriel => normale … la face */

    Facette[i].nx = a[1]*b[2]-a[2]*b[1];
    Facette[i].ny = a[2]*b[0]-a[0]*b[2];
    Facette[i].nz = a[0]*b[1]-a[1]*b[0];

    Facette[i].norme = sqrt(  Facette[i].nx*Facette[i].nx
			    + Facette[i].ny*Facette[i].ny
			    + Facette[i].nz*Facette[i].nz );
  }

  /* Calcul de la pseudo-normale pour chaque sommet */
  /* qui sera utilis‚e pour le Gouraud (et le Pong) */
  /* Consiste a faire la moyenne des normales des   */
  /* facettes qui contiennent le sommet trait‚      */

  for(i=0;i<Nb_points;i++)
  {
    nx=ny=nz=0;
    nb=0;

    for(j=0;j<Nb_faces;j++)
    {
      if (Facette[j].a==i || Facette[j].b==i || Facette[j].c==i)
      {
	nx+=Facette[j].nx;
	ny+=Facette[j].ny;
	nz+=Facette[j].nz;

	nb++;
      }
    }

    Sommet[i].nx=nx/nb;
    Sommet[i].ny=ny/nb;
    Sommet[i].nz=nz/nb;
    Sommet[i].norme = sqrt(  Sommet[i].nx*Sommet[i].nx
			   + Sommet[i].ny*Sommet[i].ny
			   + Sommet[i].nz*Sommet[i].nz );
  }
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
	sscanf(chaine+i,"%f",&x);

	while(chaine[i]!='Y') i++;
	i+=2;
	sscanf(chaine+i,"%f",&y);

	while(chaine[i]!='Z') i++;
	i+=2;
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

  if(argc<2)
  {
    printf("Syntaxe : %s <fichier.asc>\n",argv[0]);
    exit(-1);
  }

  /* Initialisations */
  InitSinus();
  FixerLumiere(10,10,10);

  ChargerASC(argv[1]);
  CalculerNormales();

  xa=ya=za=0;

  /* Passage en mode graphique */
  MyInitGraph();
  SetOverscan(66);
  InitPalette();

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
      if(car=='+') Zoff-=100;
      if( car==27) break;
    }
  }

  /* retour mode texte */
  MyCloseGraph();
}


