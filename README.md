# Projet de Programmation Système

## Structure

Notre archive de rendu prend la forme suivante:  
- Un `Makefile` à la racine du projet
- Un dossier `include` avec les header nécessaires pour utiliser notre librairie d'ordonnanceur
- Un dossier `src` avec les .c pour les définitions des méthodes
- Un dossier `test` pour le quicksort sur lequel on a mesuré les performances de nos ordonnanceurs
- Un dossier `demo` avec des exemples d’applications de nos ordonnanceurs
- Un dossier `benchmark` pour le script python utilisé pour mesurer les performances de nos scheduler et en faire des courbes
- Un dossier `build` pour stocker tous les .o créés lors de la compilation
- Un dossier `out` avec les exécutables générés après compilation

## Compilation

### Benchmark

Compilation des exécutables de benchmark utilisants `quicksort`:

```bash
make bench
```

### Demos

Compilation de mandelbrot (dépendance `gtk`):

```bash
make demo_mandelbrot
```

Compilation de voronoï (dépendance `raylib`):
```bash
make demo_voronoi
```

Compilation de la version image statique de voronoï (avec `java`):
```bash
make demo_voronoi_viewer
```
Il est aussi nécessaire de compiler le calculateur du diagramme `c`:
```bash
make demo_static_voronoi_stealing
```
ou
```bash
make demo_static_voronoi_lifo
```

## Utilisation

Tous les exécutables se trouvent dans le dossier `out`.  
Les benchmarks des schedulers ont pour préfixe `bench`, et les demos `demo`.

### Démos

Afin de générer l'image asssociée au diagramme de Voronoï, il faut d'abord le calculer avec le code `c`:
```bash
out/demo_static_voronoi_stealing
```
ou
```bash
out/demo_static_voronoi_lifo
```

Puis:
```bash
java demo/VoronoiViewer
```
L'image sera générée dans le dossier `out`

## Script benchmark

Un programme python dans le dossier `./benchmark` exécutera l'ensemble des schedulers de `0` à `n` threads à `i` itérations (le benchmark sera lancé `i` fois par thread afin de faire une moyenne des résultats).  
Un fichier .csv et un graphique par scheduler seront générés avec les résultats par nombre de threads.  

### Utilisation


`python3 bench.py`
```
  -t T        Run with t threads
  -s          Run in serial (wont work with graphs)
  -g          Generate image graphs
  -i I        Number of iterations for each benchmark
  -D          Debug mode
```

### Dépendance

Le script dépend de `matplotlib` afin de générer les graphiques.  
La démo non statique de Voronoï dépend de [raylib](https://www.raylib.com/).

## Contributeurs


| Nom       | Prénom | Numéro étudiant |
|:----------|:-------|:----------------|
| PICKERN   | Yann   | 22012875        |
| RODRIGUEZ | Lucas  | 22002335        |
