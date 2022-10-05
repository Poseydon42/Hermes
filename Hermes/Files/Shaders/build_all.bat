@echo off

set SharedDataHeaderPath="../../Source/RenderingEngine/"

if not exist Bin\ mkdir Bin
for %%v in (*.glsl) do "%VULKAN_SDK%/Bin/glslc.exe" %%v -o Bin/%%v.spv -I Common -I %SharedDataHeaderPath% -D_GLSL_ --target-env=vulkan1.1
