# Hermes
Welcome to Hermes! This used to be a game engine I was developing a while ago to learn Vulkan and modern 3D graphics. The project was sadly abandoned due to me having less and less time in light of having to prepare for exams, and then starting university, but I think it's cool enough to keep here as a reminder of what I did back in the day.

## Main features:
 - Forward+ rendering pipeline with compute-based light culling
 - Fully automatic render graph system that allocates resources, managed swapchain and command buffers, and inserts synchronization completely on its own
 - PBR rendering
 - SDF-based text rendering
 - Beginnings of a custom UI system

## How to build and run:
 - Make sure you have Vulkan SDK, Ninja, and a C++20 compiler installed
 - `git clone https://github.com/Poseydon42/Hermes.git`
 - `mkdir Build && cd Build && cmake -B . -S ..`
 - `ninja`
 - Run the built file from the project directory

## License:

Everything is under MIT license.
