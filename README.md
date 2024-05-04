# Projet de Programmation Système

## Compilation

Compilation des exécutables de benchmark utilisants `quicksort`:

```bash
make bench
```

Compilation de mandelbrot (dépendance `gtk`):

```bash
make demo_mandelbrot
```

Compilation de voronoï (dépendance `raylib`):
```bash
make demo_voronoi
```

## Utilisation

Tous les exécutables se trouvent dans le dossier `out`.  
Les benchmarks des schedulers ont pour préfixe `bench`, et les demos `demo`.

## Script benchmark

Un programme python dans le dossier `./benchmark` exécutera l'ensemble des schedulers de `0` à `n` threads à `i` itérations (le benchmark sera lancé `i` fois par thread afin de faire une moyenne des résultats).  
Un fichier .csv et un graphique par scheduler seront générés avec les résultats par nombre de threads.  

### Utilisation

`python3 bench.py`
```
  -t T        Run with t threads
  -s          Run in serial
  -g          Generate image graphs
  -i I        Number of iterations for each benchmark
```

### Dépendance

Le script dépend de `matplotlib` afin de générer les graphiques.

## Contributeurs


| Nom       | Prénom | Numéro étudiant |
|:----------|:-------|:----------------|
| PICKERN   | Yann   | 22012875        |
| RODRIGUEZ | Lucas  | 22002335        |
