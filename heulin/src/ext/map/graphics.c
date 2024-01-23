/****************************************************************************/
/* GRAPHICS.C : module de gestion des primitives graphiques pour Watcom C   */
/*              Programme par Christophe Heulin - 1997                      */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <graph.h>
#include <limits.h>
#include "graphics.h"

const int INFINI = INT_MIN;

unsigned char *screen;

int startx[200];
int endx[200];

int startcol[200];
int endcol[200];

struct
{
  int   x1,y1;
  int   x2,y2;
} tpoint[200];


int miny,maxy;


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
    startx[i]=endx[i]=INFINI;
}


/****************************************************************************/
/* MyCloseGraph() : retour en mode texte et liberation de l'ecran virtuel   */
/****************************************************************************/

void MyCloseGraph()
{
  /* repasse en mode texte 80*25 */
  _setvideomode(3);

  /* libere l'ecran virtuel */
  /* free(screen);*/

}


/****************************************************************************/
/* PutPixel() : dessine un pixel dans l'ecran virtuel                       */
/****************************************************************************/

void PutPixel(int x, int y, int color)
{
  screen[y*320+x]=color;
}


/****************************************************************************/
/* SetColor() : modifie la couleur passee en argument                       */
/****************************************************************************/

void SetColor(int i, long color)
{
  _remappalette(i,color);
}


/****************************************************************************/
/* SetOverscan() : fixe la couleur du contour de l'ecran                    */
/****************************************************************************/

void SetOverscan(unsigned char color)
{
  inp(0x3DA);
  outp(0x3C0,0x11+32);
  outp(0x3C0,color);
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


/****************************************************************************/
/***    Partie pour le remplissage de polygones                           ***/
/****************************************************************************/

/****************************************************************************/
/* Hline() : trace une ligne horizontale                                    */
/*           Le clipping est integre                                        */
/****************************************************************************/

void Hline(int x1,int x2,int y, int color)
{
  int x,offset;

  if (y<0 || y>=Ymax) return;

  if(x1>x2)
    {
      x=x1;
      x1=x2;
      x2=x;
    }

  offset=y*320+x1;

  for(x=x1;x<=x2;x++)
    {
      if(x>=0 && x<Xmax)
	screen[offset]=color;
      offset++;
    }
}


/****************************************************************************/
/* InitSegment() : fonction privee. Initialise le remplissage de polygone   */
/****************************************************************************/

void InitSegment(point p1, point p2)
{
  point temp;
  long x,pas;
  int  y;

  if(p2.y!=p1.y)
  {
    if(p2.y<p1.y)
    {
      temp = p1;
      p1   = p2;
      p2   = temp;
    }

    x  = p1.x<<8;
    pas= ((p2.x-p1.x)<<8)/(p2.y-p1.y);

    x+=pas;
    p1.y++;

    if(p1.y<miny) miny=p1.y;
    if(p2.y>maxy) maxy=p2.y;

    for(y=p1.y;y<=p2.y;y++)
    {
      if((y>=0) && (y<Ymax))
	if(startx[y]==INFINI)
	  startx[y]=x>>8;
	else
	  endx[y]=x>>8;
      x+=pas;
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

  miny=Ymax;
  maxy=0;

  curpt  = vertexlist;
  nextpt = vertexlist+1;

  for(i=1;i<numvertex;i++)
  {
    InitSegment(*curpt,*nextpt);
    curpt++;
    nextpt++;
  }

  nextpt = vertexlist;
  InitSegment(*curpt,*nextpt);

  if(miny<0) miny=0;
  if(maxy>=Ymax) maxy=Ymax-1;

  for(i=miny;i<=maxy;i++)
    {
      if(endx[i]==INFINI)
	endx[i]=startx[i];

      Hline(startx[i],endx[i],i,color);
      startx[i]=endx[i]=INFINI;
    }
}


/****************************************************************************/
/***     Partie pour le remplissage de polygones en Gouraud               ***/
/****************************************************************************/

/****************************************************************************/
/* HlineGouraud() : trace une ligne horizontale en interpolant la couleur   */
/*                  entre les points de debut et de fin de la ligne         */
/****************************************************************************/

void HlineGouraud(int x1, int coul1, int x2, int coul2, int y)
{
  long difx,pas;
  long nbcoul,couleur;
  long x;
  long indice;

  if(x2<x1)
  {
    x=x1;
    x1=x2;
    x2=x;

    x=coul1;
    coul1=coul2;
    coul2=x;
  }

  difx=x2-x1+1;

  nbcoul  = coul2-coul1+1;
  couleur = coul1<<8;
  pas     = (nbcoul<<8)/difx;

  indice=y*320+x1;

  for(x=x1;x<=x2;x++)
  {
    if(x>=0 && x<Xmax)
      screen[indice]=(couleur>>8);
    indice++;
    couleur+=pas;
  }
}


/****************************************************************************/
/* InitSegmentGouraud() : Initialise le remplissage de polygone en gouraud  */
/****************************************************************************/

void InitSegmentGouraud(point p1, point p2)
{
  int temp,y;
  long x,pas;
  long coul,pacoul;

  if(p2.y!=p1.y)
  {
    if(p2.y<p1.y)
    {
      temp=p1.y;
      p1.y=p2.y;
      p2.y=temp;

      temp=p1.x;
      p1.x=p2.x;
      p2.x=temp;

      temp=p1.couleur;
      p1.couleur=p2.couleur;
      p2.couleur=temp;
    }

    x      = (long)p1.x<<8;
    pas    = ((long)(p2.x-p1.x)<<8)/(long)(p2.y-p1.y);

    coul   = p1.couleur<<8;
    pacoul = ((long)(p2.couleur-p1.couleur)<<8)/(long)(p2.y-p1.y);

    x+=pas;
    p1.y++;
    coul+=pacoul;

    if(p1.y<miny) miny=p1.y;
    if(p2.y>maxy) maxy=p2.y;

    for(y=p1.y;y<=p2.y;y++)
    {
      if((y>=0) && (y<200))
	if(startx[y]==INFINI)
	{
	  startx[y]   = x >> 8;
	  startcol[y] = coul >> 8;
	}
	else
	{
	  endx[y]   = x >> 8;
	  endcol[y] = coul >> 8;
	}

      x    += pas;
      coul += pacoul;
    }
  }
}


/****************************************************************************/
/* FillPolyGouraud() : remplit un polygone en interpolant les couleurs      */
/****************************************************************************/

void FillPolyGouraud(point *vertexlist, int numvertex)
{
  int i;
  point *curpt,*nextpt;

  miny=Ymax;
  maxy=0;

  curpt  = vertexlist;
  nextpt = vertexlist+1;

  for(i=1;i<numvertex;i++)
  {
    InitSegmentGouraud(*curpt,*nextpt);
    curpt++;
    nextpt++;
  }

  nextpt = vertexlist;
  InitSegmentGouraud(*curpt,*nextpt);

  if(miny<0) miny=0;
  if(maxy>=Ymax) maxy=Ymax-1;

  for(i=miny;i<=maxy;i++)
    {
      if(endx[i]==INFINI)
	endx[i]=startx[i];

      HlineGouraud(startx[i],startcol[i],endx[i],endcol[i],i);;

      startx[i]=endx[i]=INFINI;
    }
}


/****************************************************************************/
/***     Partie pour le remplissage de polygones avec Texture             ***/
/****************************************************************************/

void LoadPic(char *name,char *buffer,rgb *palette)
{
  FILE *fp;

  fp = fopen (name,"rb");
  fread(palette,1,768,fp);     /* Load the palette */
  fread(buffer,1,64000,fp);    /* Load the picture */
  fclose(fp);
}


/****************************************************************************/
/* HlineTexture() : trace une ligne horizontale a partir d'une texture      */
/*                  avec une simple interpolation lineaire                  */
/****************************************************************************/

void HlineTexture(int x1,int u1,int v1,int x2,int u2,int v2,int y,char *texture)
{
  long longueur;
  long deltax,deltay;
  long xincr,yincr;
  long xpos,ypos;
  int  indice;
  long src;
  int  x,temp;


  if(x1>x2)
  {
    temp=x1; x1=x2; x2=temp;
    temp=u1; u1=u2; u2=temp;
    temp=v1; v1=v2; v2=temp;
  }

  longueur=x2-x1+1;
  if(longueur>0)
  {
    deltax = u2-u1+1;
    deltay = v2-v1+1;

    indice = y*Xmax+x1;

    src    = v1*320+u1;    /* 320 = largeur texture */

    xincr=((long)(deltax)<<8)/(long)longueur;
    yincr=((long)(deltay)<<8)/(long)longueur;

    xpos=u1<<8;
    ypos=v1<<8;

    for(x=x1;x<=x2;x++)
    {
      src = (xpos>>8) + (ypos & 0xFF00) + ((ypos & 0xFF00)>>2);
      screen[indice++]=texture[src];
      xpos+=xincr;
      ypos+=yincr;
    }
  }
}


/****************************************************************************/
/* InitSegmentTexture() : Initialise le remplissage de polygone avec texture*/
/****************************************************************************/

void InitSegmentTexture(pointMap p1,pointMap p2)
{
  int  y;
  pointMap temp;
  long x,m;
  long u,ustep,v,vstep;

  if(p2.y!=p1.y)
  {
    if(p2.y<p1.y)
    {
      temp=p1; p1=p2; p2=temp;
    }

    x = (long)p1.x<<8;
    m = ((long)(p2.x-p1.x)<<8)/((long)(p2.y-p1.y));

    u     = (long)p1.u<<8;
    ustep = ((long)(p2.u-p1.u)<<8)/((long)(p2.y-p1.y));

    v     = (long)p1.v<<8;
    vstep = ((long)(p2.v-p1.v)<<8)/((long)(p2.y-p1.y));

    x+=m;
    p1.y++;
    u+=ustep;v+=vstep;     /* important ! */

    if(p1.y<miny) miny=p1.y;
    if(p2.y>maxy) maxy=p2.y;

    for(y=p1.y;y<=p2.y;y++)
    {
      if((y>=0)&(y<Ymax))
	if(startx[y]==INFINI)
	{
	  startx[y]    = x>>8;
	  tpoint[y].x1 = u>>8;
	  tpoint[y].y1 = v>>8;
	}
	else
	{
	  endx[y]      = x>>8;
	  tpoint[y].x2 = u>>8;
	  tpoint[y].y2 = v>>8;
	}
      x += m;
      u += ustep;
      v += vstep;
    }
  }
}


/****************************************************************************/
/* FillPolyTexture() : remplit un polygone en appliquant une texture        */
/****************************************************************************/

void FillPolyTexture(pointMap *vertexlist, int numvertex, char *texture)
{
  pointMap *curpt,*nextpt;
  int i;

  miny=200;
  maxy=0;

  if(miny<0)    miny=0;
  if(maxy>=200) maxy=199;

  curpt  = vertexlist;
  nextpt = vertexlist+1;

  for(i=1;i<numvertex;i++)
  {
    InitSegmentTexture(*curpt,*nextpt);
    curpt++;
    nextpt++;
  }

  nextpt = vertexlist;
  InitSegmentTexture(*curpt,*nextpt);

  for(i=miny;i<=maxy;i++)
  {
    if(endx[i]==INFINI)
      endx[i]=startx[i];

     HlineTexture(startx[i],tpoint[i].x1,tpoint[i].y1,
		  endx[i],  tpoint[i].x2,tpoint[i].y2,
		  i,texture);

    startx[i]=endx[i]=INFINI;
  }
}

