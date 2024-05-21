# Multithreaded Physics Engine

## What is it?

A 2D physics engine which visualises and deterministically resolves the collisions of thousands of rigid bodies using a custom-written Verlet integration library.

## How does it work?

This uses Verlet integration to more accurately model the movement of rigid bodies in a dynamic system.

In order to handle multithreading, a thread pool is used, parallelising the work of both broad-phase and narrow-phase collision resolution across workers.

Currently, the means of spatial partitioning is an AABB-based uniform collision grid. However, due to the dynamics of a collision system, I have plans of upgrading to a Sphere (Circle) Tree.

## What are the next steps?

At the moment, the engine simulates in 2D only. But, with greater processing power access (via university), I have intentions of implementing a 3D engine.

## Instructions

To run this locally, `cd` into the directory of the repository, then call `mkdir build`. Next, `cd` into the `build` directory, and call these commands:

```
cmake ..
cmake --build .
./multiThreadedPhysicsEngine
```
