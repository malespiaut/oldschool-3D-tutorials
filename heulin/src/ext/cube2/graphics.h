/****************************************************************************/
/* GRAPHICS.H : module de gestion des primitives graphiques pour Watcom C   */
/*              Programme par Christophe Heulin - 1997                      */
/****************************************************************************/

#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_


void WaitVbl(void);
void ClearBuffer(void);
void ShowBuffer(void);
void MyInitGraph();
void MyCloseGraph();
void PutPixel(int x, int y, int couleur);
void Line(int x1,int y1, int x2,int y2, int couleur);


#endif