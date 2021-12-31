

# TP2 de ABBANA BENNANI Sarah

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



### Permutation des boucles

*Expliquer comment est compilé le code (ligne de make ou de gcc) : on aura besoin de savoir l'optim, les paramètres, etc. Par exemple :*

`make TestProduct.exe && ./TestProduct.exe 1024`


  ordre           | time    | MFlops  | MFlops(n=2048) 
------------------|---------|---------|----------------
i,j,k (origine)   | 2.73764 | 782.476 |                
j,i,k             |  |  |    
i,k,j             |  |  |    
k,i,j             |  |  |    
j,k,i             |  |  |    
k,j,i             |  |  |    


*Discussion des résultats*



### OMP sur la meilleure boucle 

`make TestProduct.exe && OMP_NUM_THREADS=8 ./TestProduct.exe 1024`

  OMP_NUM         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
1                 |  |
2                 |  |
3                 |  |
4                 |  |
5                 |  |
6                 |  |
7                 |  |
8                 |  |




### Produit par blocs

`make TestProduct.exe && ./TestProduct.exe 1024`

  szBlock         | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
------------------|---------|----------------|----------------|---------------
origine (=max)    |  |
32                |  |
64                |  |
128               |  |
256               |  |
512               |  | 
1024              |  |




### Bloc + OMP



  szBlock      | OMP_NUM | MFlops  | MFlops(n=2048) | MFlops(n=512)  | MFlops(n=4096)
---------------|---------|---------|------------------------------------------------
A.nbCols       |  1      |         | 
512            |  8      |         | 







# Tips 

```
	env 
	OMP_NUM_THREADS=4 ./dot_product.exe
```

```
    $ for i in $(seq 1 4); do elap=$(OMP_NUM_THREADS=$i ./TestProductOmp.exe|grep "Temps CPU"|cut -d " " -f 7); echo -e "$i\t$elap"; done > timers.out
```
