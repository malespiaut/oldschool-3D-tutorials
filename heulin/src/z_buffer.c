
void Z_Buffer()
{
  int x,y;

  for(y=0;y<YMAX;y++)
    for(x=0;x<XMAX;x++)
      PutPixel(x,y,COULEUR_DU_FOND);    /* usuellement on met 0           */
                                        /* consiste juste a vider l'ecran */

  for(chaque polygone)
    for(chaque point du polygone projete)
      double z = profondeur du pixel de coord (x,y) a l'ecran       	
      if(z>LireZ(x,y))     /* retourne la valeur du buffer Z en (x,y)	
        {
	  EcrireZ(x,y,z);
	  PutPixel(x,y,couleur);
	}
 
}
