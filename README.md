# NetworkLab

NetworkLab is a small network simulator I made using C, C++, and SFML.

I started this project mainly to practice C and learn how graph algorithms work. I first made the network engine in C, and later decided to add a graphical interface so I could actually see what was happening.

The program generates a random network of routers connected to each other. You can add or remove routers, find the shortest path between two routers, and simulate sending a message through the network.

## Screenshots

<img width="1470" height="956" alt="screenshot" src="https://github.com/user-attachments/assets/049fece0-db1b-4bb5-b523-05d94d6c9165" />


## Features

- Randomly generates a network with 20 routers
- Add and remove routers (maximum 40)
- Enable or disable routers
- Find the shortest path using Dijkstra's algorithm
- Highlight the shortest path
- Animate packets traveling between routers
- Hover over connections to check their costs
- Display errors and events at the bottom
- Reset the network

## Built With

- **C** - Network engine, graph, and Dijkstra's algorithm
- **C++** - Interface and simulation
- **SFML 3** - Graphics and user input
- **CMake** - Building the project

## How to Run

You'll need CMake, SFML 3, and a C/C++ compiler installed.

On macOS, you can install CMake and SFML using:

```bash
brew install cmake sfml
```

Clone the project:

```bash
git clone https://github.com/Rojan-Mahat/NetworkLab.git
cd NetworkLab
```

Build and run:

```bash
cmake -S . -B build
cmake --build build
./build/NetworkLab
```

## How It Works

The project has two main parts.

**C engine**

This handles all the network logic. I used a 40x40 adjacency matrix to store the connections between routers.

Each connection has a cost based on simulated distance, bandwidth, and latency.

The main files are:

- `network.c` - Creates and manages routers
- `graph.c` - Handles connections between routers
- `link.c` - Calculates connection costs
- `dijkstra.c` - Finds the shortest path

I wrote Dijkstra's algorithm using arrays to keep track of costs, visited routers, and previous routers. Once it finds the destination, it uses the previous-router array to reconstruct the path.

**C++ / SFML**

This is the graphical part of the program.

It draws the routers and connections, handles mouse and keyboard input, highlights paths, and animates the packets.

The C++ code calls functions from the C engine, so the actual network logic is still handled in C.

## Using It

Click on a router to select it. You can then remove it or enable/disable it using the buttons on the right.

To find a path, enter the source and destination router numbers and click **Run Dijkstra**.

The program highlights the cheapest route it finds. You can also hover over the links to see their individual costs.

You can type a message and click **Send** to watch a packet move along the route.

The bottom part shows messages, errors, and routing information.

You can also reset everything to generate a new network.

## What I Learned

I learned quite a bit while making this, especially about graphs and working with C.

Some things I got to practice:

- Adjacency matrices and weighted graphs
- Dijkstra's algorithm
- Arrays, structs, and pointers in C
- Using C code with C++
- SFML graphics and events
- Debugging with LLDB
- Building C and C++ together using CMake

One problem I ran into was the program crashing whenever I closed the window. I used LLDB to figure out that it was related to when the SFML textures were being destroyed.

Getting the graphics and window resizing to work properly also took longer than I expected.

## AI Assistance

I used AI assistance while working on this project, mainly for the C++ graphical interface in `main.cpp`.

The network engine, graph representation, link calculations, and Dijkstra's algorithm were written by me in C.

I handled the initial C/C++ integration and some of the SFML implementation myself, but used AI to help with UI code, rendering, visual improvements, animations, and debugging.

The goal of this project was mainly to learn the underlying network logic and algorithms, so I spent most of my time working on the C engine.

## Limitations

This isn't a real network emulator. It doesn't actually send packets over the internet.

The connection costs are simulated, and the packets are just animations showing the route calculated by Dijkstra.

I made it mainly to learn more about C, graphs, and how routing works.

## Final Thoughts

This started as a small C project where I was just generating routers and connecting them.

After finishing the graph and Dijkstra's algorithm, I thought it would be interesting to visualize everything, so I added the SFML interface.

I enjoyed working on the C engine the most. The graphical part was a bit more frustrating than I expected, but it was interesting seeing everything come together.

That's pretty much it.

**NetworkLab v1.0**
