#pragma once
#include "configuration.h"
#if HAS_SCREEN
#include <OLEDDisplay.h>
#include <OLEDDisplayUi.h>

namespace graphics
{
namespace MeshTopologyRenderer
{

// Draw the mesh topology visualization
void drawMeshTopology(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y);

} // namespace MeshTopologyRenderer
} // namespace graphics
#endif
