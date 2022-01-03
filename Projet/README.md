# Projet Epidémiologie

Le but de ce projet est de paralléliser un programme qui stimule la co-circulation d'un virus et d'un second agent pathogène en interaction dans un population humaine.

On utilise MPI et OpenMP pour faire du parallélisme respectivement distribué et en mémoire partagée, avec un affichage asynchrone.

## Configuration et compilation

J'ai effectué ce projet sous MacOS et les programmes réalisés sont, dans l'ordre des étapes :

- simulation_timed.cpp
- simulation_sync_affiche_mpi.cpp
- simulation_async_affiche_mpi.cpp
- simulation_async_omp.cpp
- simulation_async_mpi.cpp
- simulation_async_mpi_omp.cpp

Je n'ai malheureusement pas réussi à configurer mon ordinateur pour faire fonctionner les codes utilisant OpenMP. Ayant pourtant installé les librairies que je pensais nécessaire, j'ai tout de même une erreur à la compilation car la librairie OpenMP et les commandes #pragma omp parallel ne sont pas reconnues, je n'ai donc pas pu effectuer de multi-threading ni interpréter les résultats associés. Les codes sont tout de même écrits dans ce que je pense être la bonne démarche, je les développerai plus loin.

Pour compiler, on utilise la commande : `MPI=yes make all` ; `MPI=yes` est nécessaire lorsque l'on utilise MPI. Le MakeFile est configuré pour inclure le fichier `Make_osx.inc`.

## Démarches et résultats

### (2.1) Mesure du temps -- simulation_timed.cpp

Pour mesurer les différents temps voulus on utilise la librairie `chrono`.

Pour 100 000 individus, on obtient en moyenne :
- temps total = 0.052s
- temps simulation = 0.024s
- temps affichage = 0.028s

On constate qu'un peu plus de la moitié du temps de calcul est dédié à l'affichage avec ce programme séquentiel.

### (2.2) Parallélisation affichage contre simulation -- simulation_sync_affiche_mpi.cpp

On sépare la fonction `simulation` entre les deux processus, le premier pour l'affichage et le deuxième pour la simulation, en vérifiant qu'en ligne de commande on n'invoque que deux processus exactement.

On obtient cette fois ci pour 100 000 individus un temps passé par pas de temps pour la simulation de 0.034s, ce qui donne une accélération de 0.824 (=temps_sequentiel/temps_parallèle).

On remarque de plus que le temps d'affichage est équivalent au temps de simulation, cela s'explique car la programmation est synchrone, donc le processus le plus rapide doit attendre l'autre processus pour terminer. L'optimisation du temps de simulation n'est donc pas maximale avec cette méthode. 

Néanmoins, cette fois-ci le temps total ne vaut pas la somme des temps des deux processus donc on gagne en temps total d'exécution par rapport à la programmation séquentielle.

### (2.3) Parallélisation affichage asynchrone contre simulation -- simulation_async_affiche_mpi.cpp

On utilise ici la fonction `MPI_Iprobe` avec un signal (`go`) pour faire savoir quand le processus 0 attend un message du processus 1, ceci pour rendre asynchrone l'affichage. 

On obtient cette fois, toujours pour 100 000 individus :
- temps simulation = 0.026s
- temps affichage = 0.080s

On constate une diminution du temps passé pour la simulation par rapport au temps mesuré précédemment. Ceci a lieu car le processus 0 afficheur ne demande des données que lorsqu’il est prêt, donc le processus 1 calculateur peut continuer et ne pas perdre de temps en attendant ce moment, contrairement au cas précédent où il devait se synchroniser avec l’affichage, ce qui est cohérent avec la démarche précédente.

Cependant on remarque à l’exécution que le processus 0 d’affichage intervient moins souvent (environ 3 fois moins) donc il y a une légère baisse de fréquence d’affichage que l’on peut vaguement remarquer, mais ce n’est pas gênant visuellement. 



### (2.4) Parallélisation OpenMP -- simulation_async_omp.cpp

On utilise `#pragma omp parallel for` sur la boucle `for(auto& personne : population)`, qui est celle qui prend le plus de temps, en faisant une réduction sur les variables à sommer dans la boucle for.

Je n'ai pas pu obtenir de résultats en temps pour cette étape ayant des problèmes d'exécution avec OpenMP ...

On peut néanmoins remarquer que dans cette exécution on ne pourra pas obtenir le même résultat qu'avec la simulation séquentielle d'origine, car avec OpenMP l'ordre de traitement des individus n'est pas forcément l'ordre indiqué par le vecteur population. Ainsi, la génération aléatoire ne sera pas la même via la méthode `individu.estContaminé()`.


### (2.5) Parallélisation MPI de la simulation -- simulation_async_mpi.cpp

On utilise `MPI_Comm_split` pour créer un nouveau communicateur basé sur deux couleurs : 0 pour le processus 0 et 1 pour les processus de rang supérieurs. Ceci permet d'effectuer une parallélisation sur les individus. 

Encore une fois, on ne peut garantir le même résultat qu'avec la simulation d'origine.

On obtient :

pour 100 000 individus


### (2.5.1) Parallélisation finale -- simulation_async_mpi_omp.cpp
