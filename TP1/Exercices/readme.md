# Réponses aux questions

1. Envoi bloquant/non bloquant :


	MPI_SEND -> bloquant : rendra la main seulement après s’être assuré que modifier le buffer d’application est sûr (i.e. n’altère pas de données)
	
	MPI_ISEND -> non bloquant : rendent la main immédiatement, on ne peut pas deviner le moment où l’exécution va se passer (dangereux)

	Non bloquant dangereux donc solution : faire un test MPI_Test ou MPI_Waitall




2. Circulation d'un jeton 

On est en envoi bloquant donc chaque processus doit attendre que le précédent soit exécuté pour s’exécuter, d’où l’ordre. 
L’affichage de 0 se fait en dernier car dépend du nbp-1 qui dépend de tous ceux d’avant !

Dans la deuxième version, l’envoi et la récéption sont indépendants pour chaque processus donc toutes les tâches d’envoi se pproduisent simultanément. L'affichage se fait donc par ordre d'arrivée.


3. Calcul de pi 


Pour nbp = 16

Version bloquante : 390.892ms.
Version non-bloquante : 359.04ms.
