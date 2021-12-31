

# TP3 de ABBANA BENNANI Sarah

`pandoc -s --toc tp2.md --css=./github-pandoc.css -o tp2.html`




## Produit scalaire 


dotproduct.cpp pour samples=1024 sur une machine de l'ENSTA :

OMP_NUM    | temps produits scalaires | accélération
-----------|--------------|----------
séquentiel | 0.712463  | 1
1          | 0.712114  | 1.00049
2          | 0.711694  | 1.00108
3          | 0.711775  | 1.00097
4          | 0.711866  | 1.00084
8          | 0.711962  | 1.00070


dotproduct_thread.cpp pour samples=1024 sur une machine de l'ENSTA :

OMP_NUM    | temps produits scalaires | accélération
-----------|--------------|----------
séquentiel | 1.35959  | 1
1          | 1.40736  | 0.96606
2          | 1.40669  | 0.96652
3          | 1.40527  | 0.96663
4          | 1.35303  | 1.00084
8          | 1.40835  | 0.96538

On constate qu'il n'y que très peu d'accélération et parfois même le programme est plus lent. Cela s'explique parce que ce programme est memory-bound (pour chaque donnée lue en mémoire, il n'y a qu'une opération effectuée), donc le paralléliser ne l'accélère pas.



## Produit matrice-matrice

1]

dimension 1023 : tps = 20.8912s (MFlops : 102.493)

dimension 1024 : tps = 41.2854s (MFlops : 52.0155)

dimension 1025 : tps = 21.0724s (MFlops : 102.209)


Cette variation lorsqu'on prend une dimension de 1024 s'explique par le stockage en adresse mémoire cache des variables. En effet, pour une matrice de dimension 1024, on va faire des sauts de 8x1024 octets et donc souvent retomber sur la même adresse cache dans notre boucle interne. Il y aura donc beaucoup de transert entre mémoire caché et mémoire vive, ce qui ralentit beaucoup le calcul.

### Permutation des boucles

*Expliquer comment est compilé le code (ligne de make ou de gcc) : on aura besoin de savoir l'optim, les paramètres, etc. Par exemple :*

`make TestProduct.exe && ./TestProduct.exe 1024`


L'ordre k, j, i est le plus rapide avec un temps d'éxecution de 20.78s pour une dimension 1024. Cette efficacité s'explique car on réutilise parmi toutes les permutations, un maximum de valeurs déjà stockées dans le cache et on profite donc de la mémoire entrelacée.

*Discussion des résultats*



### OMP sur la meilleure boucle 

Pour une dimension de 1024

`make TestProduct.exe && OMP_NUM_THREADS=8 ./TestProduct.exe 1024`

  OMP_NUM         | Temps mat-mat (s)
------------------|---------
1                 | 21.7149 |
2                 | 10.9578 |
3                 | 7.5483 |
4                 | 5.8104 |
8                 | 6.29554 |


Le résultat peut encore être amélioré parce qu'on n'a pas encore optimisé les transferts de mémoire, qui existent encore sur la même partie de la matrice, le programme est donc toujours memory-bound.

### Produit par blocs

Pour une dimension de 1024 

`make TestProduct.exe && ./TestProduct.exe 1024`

  szBlock         | tps (s)
------------------|---------
16   		  | 1.82049 |
32                | 1.56394 |
64                | 1.35832 |
128               | 1.27438 |
256               | 1.32087 |
512               | 1.63743 | 

On obtient le temps minimal pour une taille de blocs optimale de 128.

Le temps est bien plus court pour le produit par bloc que pour le produit "scalaire", cela s'explique car la taille des blocs est assez petite pour optimiser le stockage en mémoire cache. Le programme est donc cpu-bound, c'est ce que l'on cherche.


### Bloc + OMP


On parallélise en ajoutant la commande `# pragma omp parallel for` dans la fonction `oprator*` juste avant les boucles for.

Pour une dimension de 1024 et une taille de blocs de 128, on obtient :

OMP_NUM    | temps (s) | accélération
-----------|--------------|----------
séquentiel | 1.27438  | 1
1          | 1.47383  | 0.86467
2          | 1.17234  | 1.08704
3          | 0.96998  | 1.31382
4          | 0.91212  | 1.39716
8          | 0.89023  | 1.43151

Cette fois l'accélération augmente puisqu'on a utilisé le produit par bloc.


# Tips 

```
	env 
	OMP_NUM_THREADS=4 ./dot_product.exe
```

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
