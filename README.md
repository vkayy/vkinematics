# Multithreaded Physics Engine

A 2D physics engine which deterministically resolves and visualises the collisions of thousands of rigid bodies using a custom-written Verlet Integration library.

## How does it work?

This engine uses Verlet integration to more accurately model the movement of rigid bodies in a dynamic system.

In order to handle multithreading, a thread pool is used, parallelising the work of both the broad phase and narrow phases of collision resolution across workers.

Currently, the broad phase involves spatial partitioning into a uniform collision grid, whilst the narrow phase is AABB-based. The linear structure of a uniform grid enables an elegant means of multithreading, in contrast to the non-linear quadtree or circle tree structures, hence the choice.

## What are the next steps?

At the moment, the engine simulates in 2D only. But, with greater processing power access (via university), I have intentions of implementing a 3D engine.

## How do I use this?

First of all, for sake of rendering, SFML has to be installed as it is a dependency.

For Linux, use your preferred package manager:
```
sudo apt-get install libsfml-dev
```

For MacOS, use Homebrew:
```
brew install sfml
```

For Windows, download from the website:

https://www.sfml-dev.org/download.php

Next, assuming you've downloaded the repository, to run the engine, `cd` into the directory of the repository, then call `mkdir build`. At this stage, `cd` into the `build` directory, and call these commands:

```
cmake ..
cmake --build .
./multiThreadedPhysicsEngine
```

Now, whenever you want to re-run a simulation, it is as simple as `Up Arrow` then `Enter` (i.e., running that block of commands again).

You can use the keys `A` and `R` to respectively activate an attractor/repeller in the centre of the simulation space.

In `src/main.cpp`, there are also numerous parameters that you can modify to your liking at the top of the file:
- `WINDOW_WIDTH`: The width of the window.
- `WINDOW_HEIGHT`: the width of the window.
- `MAX_OBJECT_COUNT`: The maximum object count.
- `MIN_RADIUS`: The minimum object radius.
- `MAX_RADIUS`: The maximum object radius.
- `MAX_ANGLE`: The maximum angle of object spawn velocity.
- `SPEED_COLOURING`: If true, objects are coloured based on speed. By default, they are coloured by rainbow.
- `SPAWN_DELAY`: The delay between consecutive object spawns.
- `SPAWN_SPEED`: The speed at which an object spawns.
- `FRAMERATE_LIMIT`: The maximum framerate.
- `THREAD_COUNT`: The number of threads used (experiment with this, see what works best for you).

For the `COLLISION_RESOLVER` parameter, there are three options:
- `0`: Multithreaded and optimised with uniform collision grid spatial partitioning.
- `1`: Single-threaded and optimised with uniform collision grid spatial partitioning.
- `2`: Single-threaded and brute force collision resolution.
- Any other invalid option will default to multithreading.

Also, note that the default `SPAWN_POSITION` is the centre of the window, but this can be modified also (ensure this is in terms of `WINDOW_WIDTH` and `WINDOW_HEIGHT`, as your objects may spawn out of bounds).
