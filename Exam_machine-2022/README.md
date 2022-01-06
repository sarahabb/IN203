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

En parallélisant d'abord seulement l'encode de l'image avec la ligne `# pragma omp parallel for` devant les 4 boucles for de la fonction `discretTransformFourier`, on a les temps et accélérations suivants :

Pour small_lena_gray.png (et taux=10%) :

nb threads   | temps encodage (s)   | accélération
-------------|----------------------|----------
1            | 245.662              | 1
2            | 126.915              | 1.5
3            | 86.5239              | 2.839
4            | 67.7675              | 3.625


On parallélise ensuite la reconstitution dans la fonction ìnversePartialDiscretTransformFourier`
