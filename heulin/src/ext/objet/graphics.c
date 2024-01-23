/****************************************************************************/
/* GRAPHICS.C : module de gestion des primitives graphiques pour Watcom C   */
/*              Programme par Christophe Heulin - 1997                      */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <graph.h>
#include "graphics.h"


unsigned char *screen;

int startx[200];
int endx[200];



/****************************************************************************/
/* WaitVbl() : attend le retour du faisceau vertical.                       */
/****************************************************************************/

void WaitVbl(void)
{
  while (!(inp(0x3DA) & 0x08));
}


/****************************************************************************/
/* ClearBuffer() : vide le contenu de l'ecran virtuel                       */
/****************************************************************************/

void ClearBuffer(void)
{
  memset(screen,0,320*200);
}


/****************************************************************************/
/* ShowBuffer() : transfert l'ecran virtuel vers la zone d'ecran            */
/****************************************************************************/

void ShowBuffer(void)
{
  memcpy((unsigned char *)(0xA000 << 4), screen, 320*200);
}


/****************************************************************************/
/* MyInitGraph() : ouverture de l'ecran graphique et alloc ecran virtuel    */
/****************************************************************************/

void MyInitGraph()
{
  int i;

  /* Allocation de l'ecran virtuel */
  screen=(unsigned char*)calloc(320*200,sizeof(unsigned char));

  /* passe en 320*200 256 couleurs */
  _setvideomode(19);

  /* pour le remplissage de polygones */
  for(i=0;i<200;i++)
    startx[i]=endx[i]=-16000;
}


/****************************************************************************/
/* MyCloseGraph() : retour en mode texte et liberation de l'ecran virtuel   */
/****************************************************************************/

void MyCloseGraph()
{
  /* repasse en mode texte 80*25 */
  _setvideomode(3);
  
  /* libere l'ecran virtuel */
  /* free(screen); */
}


/****************************************************************************/
/* PutPixel() : dessine un pixel dans l'ecran virtuel                       */
/****************************************************************************/

void PutPixel(int x, int y, int color)
{
  screen[y*320+x]=color;
}


/****************************************************************************/
/* Line() : trace une ligne selon l'algorithme de Lucas                     */
/*          version avec clipping                                           */
/****************************************************************************/

void Line(int x1,int y1, int x2,int y2, int color)
{
  int x,y;
  int Dx,Dy;
  int xincr,yincr;
  int erreur;
  int i;


  /* On initialise nos variables */
  Dx = abs(x2-x1);
  Dy = abs(y2-y1);

  if(x1<x2)
    xincr = 1;
  else
    xincr = -1;

  if(y1<y2)
    yincr = 1;
  else
    yincr = -1;


  /* Trace de ligne */
  x = x1;
  y = y1;

  if(Dx>Dy)
    {
      erreur = Dx/2;     /* c'est plus esthetique comme ca */
      for(i=0;i<Dx;i++)
	{
	  x += xincr;
	  erreur += Dy;
	  if(erreur>Dx)
	    {
	      erreur -= Dx;
	      y += yincr;
	    }
	  /* clipping */
	  if(x>=0 && x<Xmax && y>=0 && y<Ymax) PutPixel(x,y,color);
	}

    }
  else
    {
      erreur = Dy/2;     /* c'est plus esthetique comme ca */
      for(i=0;i<Dy;i++)
	{
	  y += yincr;
	  erreur += Dx;
	  if(erreur>Dy)
	    {
	      erreur -= Dy;
	      x += xincr;
	    }
	  /* clipping */
	  if(x>=0 && x<Xmax && y>=0 && y<Ymax) PutPixel(x,y,color);
	}
    }
}

