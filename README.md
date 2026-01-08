debut de dev d'un module kernel

pour build 
make à la racine


pour lancer 
c'est pas très clean pour l'instant c'est plutot un POC qu'autre chose
donc faut le faire dans l'ordre :D

sudo insmod ./kernel/intercept.ko
./user/cli

dans une autre fenetre lancer le
./test/rw

dans la fenetre de la cli faire les test sur le programme rw
et le fd3 parce que ya que le hook de write de fait 


bien quitter avec d puis q
pareil parce que c'est pas encore clean et ca fait plenter la machine sinon :D

