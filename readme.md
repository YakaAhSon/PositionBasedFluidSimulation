# Position Based Fluid Simulation
Game Physics class final project of Lu Guowei and Liu Zhaopeng.

Based on [this paper](http://mmacklin.com/pbf_sig_preprint.pdf).

## Scope
* Partical Based Fluid Simulation
* Fluid - Rigid Body interaction (Buoyancy)
* Rigid body collision
* Cloth Simulation

## Control
* `SPACE` to start/pause simulation
* `N` to toggle one step simulation when paused
* `W`,`A`,`S`,`D` to roam around
* `Left Click and Drag` to change camera direction
  
## Performence
We test the performence on `GTX1060` with `5` iterations per frame.
|Particals|Time Per Iteration|Frame Rate|
|-|-|-|
| 64K|1.3ms|110fps|
|128K|2.0ms|70fps
----------