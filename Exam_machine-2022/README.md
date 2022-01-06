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

