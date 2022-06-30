Per compilar els programes haurem de fer servir el Makefile.

Els programes s'executen amb les comandes

./mainSave

./mainRecc <mode execució> <paràmetre 1> <paràmetre 2>

Els modes són descrits a continuació:

<mode execució>:
    - 1 : Conta el nombre de pel·lícules vistes per un usuari. <paràmetre 1> := id d'usuari.
    - 2 : Conta el nombre de usuaris que han vist una pel·lícula. <paràmetre 1> := id de la pel·lícula.
    - 3 : Prediu el rating que faria el usuari amb id donat a la pel·lícula amb la id donada. <paràmetre 1>:= id d'usuari. <paràmetre 2> := id de la pel·lícula.
    - 4 : Recomana una pel·lícula a l'usuari amb id donat (no vista per l'usuari). <paràmetre 1> := id d'usuari.

Exemples d'execució i resultats esperats:


./mainRecc 1 2207774
>>> The number of movies seen by the user 2207774 is 57

./mainRecc 2 1
>>> The number of users that have seen the movie 1 is 547

./mainRecc 3 2207774 1
>>> U:2207774 - M:1 - The forecasted rating was 4.101167

./mainRecc 4 2207774
>>> The recommended movie for user 2207774 is 1418.
