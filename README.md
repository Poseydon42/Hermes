# Hermes
Hermes is a personal C++20/Vulkan 3D rendering engine built to explore real-time graphics, GPU compute, and low-level rendering architecture.

Key features include:
- Forward+ rendering with compute-based light culling.
- An automatic render graph handling resource allocation, synchronization, and command submission.
- Physically based rendering and SDF text rendering.

The project is no longer under active development, but the repository preserves the implementation and the engineering techniques explored.

## How to build and run:
 - Make sure you have Vulkan SDK, Ninja, and a C++20 compiler installed
 - `git clone https://github.com/Poseydon42/Hermes.git`
 - `mkdir Build && cd Build && cmake -B . -S ..`
 - `ninja`
 - Run the built file from the project directory

## License:

Everything is under MIT license.
