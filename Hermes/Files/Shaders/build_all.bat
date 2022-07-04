@echo off
if not exist Bin\ mkdir Bin
for %%v in (*.glsl) do "%VULKAN_SDK%/Bin/glslc.exe" %%v -o Bin/%%v.spv -I Common --target-env=vulkan1.1
