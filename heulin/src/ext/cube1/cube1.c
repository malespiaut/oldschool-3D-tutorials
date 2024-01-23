#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <graph.h>


const Xoff = 320;
const Yoff = 240;
const Zoff = 500;


typedef struct
{
  double x,y,z;
} Point;


Point cube1[8];


Point cube2[8]= { -100,-100,-100,
		   100,-100,-100,
		   100, 100,-100,
		  -100, 100,-100,
		   100,-100, 100,
		  -100,-100, 100,
		  -100, 100, 100,
		   100, 100, 100 };

void Initialiser(void)
{
  cube1[0].x = -100;  cube1[0].y = -100;  cube1[0].z = -100;
  cube1[1].x =  100;  cube1[1].y = -100;  cube1[1].z = -100;
  cube1[2].x =  100;  cube1[2].y =  100;  cube1[2].z = -100;
  cube1[3].x = -100;  cube1[3].y =  100;  cube1[3].z = -100;
  cube1[4].x =  100;  cube1[4].y = -100;  cube1[4].z =  100;
  cube1[5].x = -100;  cube1[5].y = -100;  cube1[5].z =  100;
  cube1[6].x = -100;  cube1[6].y =  100;  cube1[6].z =  100;
  cube1[7].x =  100;  cube1[7].y =  100;  cube1[7].z =  100;
}



void Sommet1(void)
{
  int i;

  _setcolor(15);

  for (i=0;i<8;i++)
  {
    _setpixel((cube1[i].x*256)/(cube1[i].z+Zoff)+Xoff,
	      (cube1[i].y*256)/(cube1[i].z+Zoff)+Yoff);
  }
}


void Sommet2(void)
{
  int i;

  _setcolor(15);

  for (i=0;i<8;i++)
  {
    _setpixel((cube2[i].x*256)/(cube2[i].z+Zoff)+Xoff,
	      (cube2[i].y*256)/(cube2[i].z+Zoff)+Yoff);
  }
}



void ligne(int a, int b)
{
 /* Cette fonction trace une ligne entre les 2 points projetes a et b */
 /* On effectue une projection orthogonale */

 _moveto((cube2[a].x*256)/(cube2[a].z+Zoff)+Xoff,
	 (cube2[a].y*256)/(cube2[a].z+Zoff)+Yoff);

 _lineto((cube2[b].x*256)/(cube2[b].z+Zoff)+Xoff,
	 (cube2[b].y*256)/(cube2[b].z+Zoff)+Yoff);
}



void FilDeFer(void)
{
 _setcolor(15);

 /* On affiche la face avant */
 ligne(0,1); ligne(1,2); ligne(2,3); ligne(3,0);

 /* Puis la face arriere */
 ligne(4,5); ligne(5,6); ligne(6,7); ligne(7,4);

 /* Et enfin les aretes restantes */
 ligne(0,5);
 ligne(1,4);
 ligne(2,7);
 ligne(3,6);
}


void MyInitGraph()
{
  /* passe en 640*480 16 couleurs */
  _setvideomode(18);
}


void MyCloseGraph()
{
  /* repasse en mode texte 80*25 */
  _setvideomode(3);
}


void main()
{
  MyInitGraph();

  Initialiser();

  Sommet1();
  Sommet2();
  FilDeFer();

  while(!kbhit());

  MyCloseGraph();
}

