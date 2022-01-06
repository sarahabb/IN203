# Examen machine 2022 - IN203

J'ai effectué cet examen sur une machine de l'ENSTA sous Linux.

## Mesure du temps

Pour small_lena_gray.png (et taux=10%) :
- Temps encodage : 245.662s
- Temps sélection : 0.00616867s
- Temps reconstitution : 28.1454s


Pour tiny_lena_gray.png (et taux=10%) :
- Temps encodage : 1.48832s
- Temps sélection : 0.000318428s
- Temps reconstitution : 0.158553s



## Parallélisation OpenMP

On a vu juste avant que les fonctions prenant le plus de temps sont l'encodage, puis la reconstitution.

En parallélisant d'abord seulement l'encodage de l'image avec la ligne `# pragma omp parallel for` devant les 4 boucles for de la fonction `discretTransformFourier` en parallélisant les pixels. On a les temps et accélérations suivants :

Pour small_lena_gray.png (et taux=10%) :

nb threads   | temps encodage (s)   | accélération
-------------|----------------------|----------
1            | 245.662              | 1
2            | 126.915              | 1.5
3            | 86.5239              | 2.839
4            | 67.7675              | 3.625


On parallélise ensuite les coefficients complexes (sélectionnés durant la compression) de la reconstitution dans la fonction `inversePartialDiscretTransformFourier` avec la commande `# pragma omp parallel for` devant les boucles de la foncion

On obtient pour small_lena_gray.png (et taux=10%) :

nb threads   | temps restitution (s)| accélération
-------------|----------------------|----------
1            | 28.1454              | 1
2            | 14.515               | 1.939
3            | 9.82165              | 2.866
4            | 7.53308              | 3.734


On remarque pour les deux parallélisations une accélération qui augmente avec le nombre de threads, on voit donc que ces parallélisations sont très efficaces pour diminuer le temps de calcul de l'encodage et de la reconstitution de l'image.

## Première parallélisation MPI

Dans le main on réserve la partie de reconstitution de de sauvegarde de l'image au processus 0 avec un `if (rank==0){}`.

Ensuite, on réalise une partition de l'image en créant un nombre de lignes par processus `nj_loc=nj/nbp` dans la fonction discretTransformFourier

Ainsi chaque processus s'occupe d'une partie des lignes et réalise la transformatino de Fourier d'une partie de l'image. 

On récupère ensuite l'image entière transformée à l'aide de la commande `MPI_Allgather`.

Je n'ai pas réussi à obtenir une image correcte, j'ai l'impression d'avoir un problème dans le `MPI_Allgather` qui fait que pour l'affichage, le processus 0 ne considère que sa transformation à lui (quand j'exécute avec 2 processus j'ai la moitié de l'image correcte, avec 3 un tiers). J'ai essayé de recréer un X différent de celui modifié dans la boucle pour que le Allgather soit correct mais j'avais des segmentation fault que je n'ai pas réussi à enlever.


## Deuxième parallélisation MPI

Je n'ai pas eu le temps de la faire mais au vu des problèmes de la question précédente je voulais essayer de faire la concaténation depuis le main avec un `MPI_Allreduce` des fréquences.
