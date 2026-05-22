# Déquantification d'une série temporelle

Projet de L2 Informatique -- reconstruction d'une série temporelle à partir de sa version sous-quantifiée et de l'histogramme des couples de valeurs successives.

## Principe

À partir d'une série `xQ` (sous-quantifiée d'1 bit) et d'un histogramme `P` des couples `(x(n-1), x(n))` de la série originale, on construit un arbre binaire représentant toutes les séries possibles et on l'élague à l'aide de `P`. Les feuilles vertes de l'arbre sont les séries déquantifiées valides.

L'algorithme et son déroulement sont visualisés en temps réel dans une fenêtre SDL2 (construction, élagage des branches mortes, mise en évidence des solutions).

## Dépendances

- **gcc** : compilateur C
- **SDL2** : bibliothèque pour l'affichage graphique
- **make** : pour la compilation

## Installation et compilation

Un script `script.sh` est fourni à la racine. Il :

1. installe `gcc` et `libsdl2-dev` s'ils ne sont pas déjà présents
2. lance le `Makefile` pour compiler le projet

```bash
chmod +x script.sh
./script.sh
```

## Exécution du programme / Tests

```bash
./prog <fichier.dat> ou bien ./prog_test
```

où `<fichier.dat>` contient la série quantifiée à déquantifier.  
./prog_test pour lancer les tests et voir leurs résultats.

## Contrôles dans la fenêtre SDL

- **Clic droit + glisser** ou **flèches** : naviguer dans l'arbre
- **Molette** : scroll vertical
- **Ctrl + molette** ou **+ / -** : zoom / dézoom
- **R** : reset de la vue, **0** : reset du zoom
- **PAUSE/REPRENDRE** : suspendre ou reprendre la construction
- **RELANCER** : relancer la construction sur la même série
- **QUITTER** : fermer le programme

## Options de configuration

Deux macros permettent d'adapter le comportement du programme à la compilation :

- **`NO_DISPLAY`** (dans `display.h`) : désactive complètement l'affichage SDL2. Utile pour tester l'algorithme sur de grosses séries sans le coût graphique. Décommenter la ligne `#define NO_DISPLAY` dans `display.h` pour l'activer.

- **`NO_ELAGAGE`** (dans `tree.h`) : désactive l'élagage récursif des branches mortes. Les nœuds invalides restent visibles en rouge dans l'arbre au lieu d'être supprimés. Décommenter la ligne `#define NO_ELAGAGE` dans `tree.h` pour l'activer.


Deux constantes permettent aussi de régler la vitesse de l'animation graphique :

- **`DELAY_NODE_MS`** (dans `display.h`) : délai en ms après la création d'un nœud, contrôle la vitesse de la construction visuelle.
  ```c
  //delai en ms apres creation d'un noeud (vitesse de l'animation)
  #define DELAY_NODE_MS 50
  ```

- **`DELAY_ELAGAGE_MS`** (dans `tree.c`) : délai en ms entre chaque étape de l'élagage, pour bien voir disparaître les branches mortes.
  ```c
  //delai en ms entre chaque etape de l'elagage (pour voir l'animation)
  #define DELAY_ELAGAGE_MS 30
  ```

Augmenter ces valeurs ralentit l'animation pour mieux la suivre, les diminuer (voire mettre `0`) accélère le rendu.

## Auteurs
- Mahleb Chakib Mohamed Tahar
- Rzaigui Adem
- Kerrad Arezki 
- Projet L2F1 -- Université Paris Cité -- encadré par Mr Gael Mahé.
