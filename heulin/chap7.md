**Les Textures ou Mapping**

Pour rendre les objets encore plus impressionnants, il est intéressant
de pouvoir ajouter des textures. Ce n\'est pas forcément très simple au
premier abord, mais si vous avez compris comment faire du Gouraud, ça
ira beaucoup mieux.

Le but du mapping est bien sur de \"coller\" une image, que l\'on
appelle texture, sur un objet. Encore une fois il y a plusieurs
techniques possibles, certaines étant spécialement conçues pour la 3D,
et intégrant la correction de perspective. La méthode présentée ici est
beaucoup plus naïve, mais donne tout de même des résultats. Les sources
sont disponibles dans [map.zip](map.zip).

![](map2.gif){height="200" width="320"}

Une sphère mappée

**[1/ Préparer les données]{.underline}**

L\'idée directrice est d\'associer à chaque sommet de l\'objet une
coordonnée (x,y) dans la texture à appliquer. Dans la (bonne)
littérature, ces coordonnées sont nommées U et V. Le plus dur étant de
calculer ces coordonnées sur un objet complexe, nous allons utiliser les
données fournies par 3D Studio. Le modeleur de 3DS permet en effet
d\'appliquer des textures (encore heureux, mais encore faut-il savoir
s\'en servir), et il va fournir les valeurs de U et V dans le fichier au
format .ASC.

La première chose à faire est donc de modifier notre chargeur d\'objet
afin de prendre en compte ces nouvelles données. Pour les stocker, nous
devons modifier notre structure de données. Voici celle qui sera
utilisée :

    typedef struct
    {
      int x,y;
      int u,v;
    } pointMap;

Dans notre chargeur, il faut rajouter juste quelques petites
instructions, mais c\'est exactement la même chose que pour lire les
coordonnées x,y et z.

            while(chaine[i]!='U') i++;
            i+=2;
            sscanf(chaine+i,"%f",&u);

            while(chaine[i]!='V') i++;
            i+=2;
            sscanf(chaine+i,"%f",&v);

            Point2D[Nb_points].u=u*320;   /* (U * largeur_texture) */
            Point2D[Nb_points].v=v*200;   /* (V * hauteur_texture) */

Il faut bien veiller à multiplier U et V par la largeur et la hauteur de
la texture, car 3DS lui les normalise (entre 0 et 1). Personnellement,
je place ces valeurs dans le tableau Point2D, déclaré comme suit :

    pointMap Point2D[1000]; /* les sommets après projection */

tout simplement parce que ces données sont invariables, et qu\'ainsi je
vais simplifier mon code, mais c\'est une question de goût.

Pour remplir nos polygones, nous utilisions en plus de **startx** et
**endx**, les tableaux **startcol** et **endcol**. Pour le mapping, nous
n\'allons pas utiliser les 2 derniers tableaux, sauf si vous voulez
combiner Textures et Gouraud, mais là je vous laisse faire (je ne vais
pas vous mâcher tout le travail non plus). A la place nous allons avoir
une jolie petite structure ce qui évitera de se promener avec 4 tableaux
différents :

    struct {
      int   x1,y1;
      int   x2,y2;
    } texturepoint[200];

x1 et y1 seront les coordonnées du point de départ dans la texture (pour
la ligne traitée), et x2 et y2 les coordonnées de fin.

![](texture.gif){height="200" width="350"}

En principe avec ce petit dessin vous avez déjà du comprendre le
principe du mapping.

**[2/ Initialiser le remplissage de polygone]{.underline}**

Pour remplir un polygone, il faut interpoler les coordonnées x entre les
sommets, et ce pour chaque ligne y. Quand nous sommes passé au Gouraud,
nous avons en plus interpolé (en même temps) les couleurs de début et de
fin de chaque ligne horizontale. Pour les textures, il faudra
**interpoler les coordonnées U et V** entre les sommets. Le principe est
donc maintenu, et ne constitue donc rien de nouveau.

La fonction initialisant le remplissage de polygone (appelée pour chaque
arête de l\'objet) va donc simplement être adaptée. Le texte en gras
représente ce qu\'il y a de nouveau par rapport au remplissage classique
:

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

Vu sous cet angle, ca semble bien simple n\'est-ce pas ? Mais pourquoi
n\'y avez-vous pas pensé plus tôt ? Bon ce n\'est pas grave, ça me donne
l\'impression d\'être utile :o)\
Je ne vais pas m\'attarder en commentaires, je crois que tout a déjà été
dit dans les chapitres précédents.

**[3/ Appliquer la texture]{.underline}**

C\'est certainement la partie que vous pourrez le plus optimiser, mais
je vous rassure le code que je fournit, même s\'il ne contient pas une
goutte d\'assembleur, est relativement efficace.

Maintenant que nous connaissons les coordonnées U et V de départ et de
fin pour chaque ligne, il ne nous reste plus qu\'à dessiner ces lignes.
Encore une fois le mot magique en animation intervient :
**interpolation** ! Ça risque de vous décevoir du fait de la grande
simplicité, mais il existe de meilleure méthode. M\'enfin celle-ci
marche, alors si vous débutez elle suffira largement.

En fait, pour chaque point de la scanline (ligne horizontale traitée) il
nous suffit de retrouver le point correspondant dans la texture. On
l\'obtient à partir des variables **xpos** et **ypos**, qui sont en fait
les valeurs interpolées de U et V de début et de fin.

Pour simplifier les choses, on suppose que la texture est rectangulaire
(c\'est généralement le cas, mais enfin il y a peut-être des détraqués
près à tout parmi vous).

Assez parlé, voilà le code :

    void HlineTexture(int x1,int u1,int v1,int x2,int u2,int v2, 
                      int y,char *texture)
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

Le calcul de **src** peut vous choquer, alors je vais vous expliquer :
dans mon cas je suppose que la **texture est en 320\*200**. Donc le
point à accéder se trouve à l\'emplacement (V\*320+U), soit
(ypos/256)\*320+(xpos\*256). Pour calculer le terme (ypos/256)\*320 je
pose un masque pour avoir V\*256 sans la partie fractionnaire, et
ensuite j\'ajoute ypos\>\>2, soit en fait V\*64. Donc V\*64\*V\*256+U =
V\*320\*U.

En pratique, je vous conseille d\'utiliser des textures de 256\*256,
surtout si vous faites de l\'assembleur. Ça vous permettra d\'optimiser
pas mal votre code. A cela une explication bien simple : 256\*256 =
655536 = taille d\'un segment.

Pour faire proprement, le calcul de src devrait etre :

-   

Si vous utilisez une texture 256\*256, vous pourrez mettre à la place :

-   

Surtout, n\'oubliez pas de changer les valeurs lors du chargement de
fichier ASC, lorsqu\'on multiplie U et V par la largeur et la hauteur de
la texture.

**[4/ La fonction de remplissage]{.underline}**

Il n\'y a rien de particulier qui change dans cette fonction, mais je
n\'ai pas résisté au plaisir de vous couvrir encore un peu plus de code.
Il faut dire qu\'en général les programmeurs sont très rébarbatifs à
l\'idée de distribuer leurs sources librement. Encore heureux que le
projet GNU existe\...

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

**[5/ Un exemple]{.underline}**

Fidèle à mon habitude, je vous ai préparé un petit exemple. J\'ai essayé
de faire mieux qu\'un cube avec des images sur les bords, et j\'ai donc
tenté d\'appliquer une texture sur une sphère ([map.zip](map.zip)). Pour
ça j\'ai fait appel à 3D Studio (non non je l\'ai pas chez moi, je suis
pas un pirate ;-).

C\'est alors que je me suis rendu compte que j\'étais mieux à programmer
qu\'à utiliser ce logiciel, parce que je n\'ai pas réussi à mettre la
texture comme je voulais. Ce crétin ne rejoint pas les extrémités de la
texture. Je vous jure ! Le fichier jupiter.asc de l\'exemple le prouve.
Essayez de le charger avec 3DS, et faites une animation (genre faire
tourner la sphère sur elle-même). Eh bien oui, il y a une bande noire !
Cela se traduit donc forcément donc mon programme, mais le bug ne vient
pas de moi.

![](map1.gif){height="200" width="320"}

*La petite planète qui tourne, qui tourne\...*

------------------------------------------------------------------------

[\[Sommaire\]](Plan3D.html) [\[Précédent\]](chap6.html) \[Suivant\]
[\[Home Page\]](../accueil.html){target="Contenu"}

Page réalisée par [Christophe Heulin.](../cv.html){target="Contenu"}\
Renseignements, critiques, informations : <heulin@multimania.com>\
\
