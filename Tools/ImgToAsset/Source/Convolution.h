#pragma once

#include <memory>

#include "Image.h"

std::unique_ptr<Image> GenerateDiffuseConvolution(const Image& Source);
