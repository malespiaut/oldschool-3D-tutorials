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
  /* Allocation de l'ecran virtuel */
  screen=(unsigned char*)calloc(320*200,sizeof(unsigned char));

  /* passe en 320*200 256 couleurs */
  _setvideomode(19);
}


/****************************************************************************/
/* MyCloseGraph() : retour en mode texte et liberation de l'ecran virtuel   */
/****************************************************************************/

void MyCloseGraph()
{
  /* libere l'ecran virtuel */
  free(screen);

  /* repasse en mode texte 80*25 */
  _setvideomode(3);
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
	  PutPixel(x,y,color);
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
	  PutPixel(x,y,color);
	}
    }
}


/****************************************************************************/
/* Hline() : trace une ligne horizontale                                    */
/****************************************************************************/

void Hline(int x1,int x2,int y, int color)
{
  int x,offset;

  if(x1>x2)
    {
      x=x1;
      x1=x2;
      x2=x;
    }

  offset=y*320+x1;

  for(x=x1;x<=x2;x++)
    screen[offset++]=color;

}


/****************************************************************************/
/* InitSegment() : fonction privee. Initialise le remplissage de polygone   */
/****************************************************************************/

void InitSegment(int x1,int y1,int x2,int y2)
{
  int temp,y;
  long x,m;

  if(y2!=y1)
  {
    if(y2<y1)
    {
      temp=y1;
      y1=y2;
      y2=temp;

      temp=x1;
      x1=x2;
      x2=temp;
    }
    x=(long)x1<<8;

    m=((long)(x2-x1)<<8) / ((long)(y2-y1));

    x+=m;
    y1++;

    for(y=y1;y<=y2;y++)
    {
      if((y>=0) & (y<640))
	if(startx[y]==-16000)
	  startx[y]=x>>8;
	else
	  endx[y]=x>>8;
      x+=m;
    }
  }
}


/****************************************************************************/
/* FillPoly() : remplissage d'un polygone definit par 'numvertex' sommets   */
/****************************************************************************/

void FillPoly(point *vertexlist, int numvertex,int color)
{
  int i;
  point *curpt,*nextpt;


  curpt  = vertexlist;
  nextpt = vertexlist+1;

  for(i=0;i<200;i++)
    startx[i]=endx[i]=-16000;

  for(i=1;i<numvertex;i++)
  {
    InitSegment(curpt->x,curpt->y,nextpt->x,nextpt->y);
    curpt+=1;
    nextpt+=1;
  }

  nextpt = vertexlist;
  InitSegment(curpt->x,curpt->y,nextpt->x,nextpt->y);

  for(i=0;i<200;i++)
    if (startx[i]!= -16000)
    {
      if(endx[i]==-16000)
	endx[i]=startx[i];

      Hline(startx[i],endx[i],i,color);
    }
}
