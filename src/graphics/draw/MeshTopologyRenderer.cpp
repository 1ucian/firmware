#include "configuration.h"
#if HAS_SCREEN
#include "MeshTopologyRenderer.h"
#include "NodeDB.h"
#include "graphics/ScreenFonts.h"
#include "graphics/SharedUIDisplay.h"
#include <algorithm>
#include <cmath>
#include <vector>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace graphics
{
namespace MeshTopologyRenderer
{

struct NodePosition {
    uint32_t nodeNum;
    int16_t x;
    int16_t y;
    int hops;
    char shortName[5];
};

void drawMeshTopology(OLEDDisplay *display, OLEDDisplayUiState *state, int16_t x, int16_t y)
{
    // Screen dimensions
    const int screenW = display->getWidth();
    const int screenH = display->getHeight();
    const int headerHeight = FONT_HEIGHT_SMALL + 2;

    // Drawing area (below header)
    const int centerX = screenW / 2;
    const int centerY = y + headerHeight + (screenH - headerHeight) / 2;

    // Radii for hop rings
    const int radius1 = 18;  // 1-hop ring
    const int radius2 = 30;  // 2-hop ring
    const int radius3 = 42;  // 3+ hop ring (for larger screens)

    // Draw header
    graphics::drawCommonHeader(display, x, y, "Mesh");

    // Collect all nodes by hop count
    std::vector<NodePosition> hop1Nodes;
    std::vector<NodePosition> hop2Nodes;
    std::vector<NodePosition> hop3Nodes;

    int totalNodes = nodeDB->getNumMeshNodes();
    uint32_t myNodeNum = nodeDB->getNodeNum();

    for (int i = 0; i < totalNodes; i++) {
        auto *node = nodeDB->getMeshNodeByIndex(i);
        if (!node || node->num == myNodeNum)
            continue;

        NodePosition np;
        np.nodeNum = node->num;
        np.hops = node->has_hops_away ? node->hops_away : 99;

        // Get short name (max 4 chars)
        if (node->has_user && node->user.short_name[0]) {
            strncpy(np.shortName, node->user.short_name, 4);
            np.shortName[4] = '\0';
        } else {
            snprintf(np.shortName, sizeof(np.shortName), "%04X", (uint16_t)(node->num & 0xFFFF));
        }

        // Sort by hop count
        if (np.hops <= 1) {
            hop1Nodes.push_back(np);
        } else if (np.hops == 2) {
            hop2Nodes.push_back(np);
        } else {
            hop3Nodes.push_back(np);
        }
    }

    // Calculate positions for each ring
    auto positionNodesInRing = [&](std::vector<NodePosition> &nodes, int radius, float startAngle = 0) {
        int count = nodes.size();
        if (count == 0) return;

        float angleStep = (2.0f * M_PI) / count;
        for (int i = 0; i < count; i++) {
            float angle = startAngle + (i * angleStep);
            nodes[i].x = centerX + (int)(radius * cos(angle));
            nodes[i].y = centerY + (int)(radius * sin(angle));
        }
    };

    positionNodesInRing(hop1Nodes, radius1, -M_PI / 2);  // Start from top
    positionNodesInRing(hop2Nodes, radius2, -M_PI / 2 + 0.3f);  // Slight offset
    if (screenH > 64) {
        positionNodesInRing(hop3Nodes, radius3, -M_PI / 2 + 0.5f);
    }

    // Draw connection lines from center to hop-1 nodes
    display->setColor(WHITE);
    for (const auto &np : hop1Nodes) {
        display->drawLine(centerX, centerY, np.x, np.y);
    }

    // Draw connection lines from hop-1 to hop-2 (simplified - just radial)
    for (const auto &np : hop2Nodes) {
        // Find closest hop-1 node and draw line
        int closestDist = 9999;
        int closestX = centerX, closestY = centerY;
        for (const auto &h1 : hop1Nodes) {
            int dx = np.x - h1.x;
            int dy = np.y - h1.y;
            int dist = dx*dx + dy*dy;
            if (dist < closestDist) {
                closestDist = dist;
                closestX = h1.x;
                closestY = h1.y;
            }
        }
        display->drawLine(closestX, closestY, np.x, np.y);
    }

    // Draw center node (ME)
    display->fillCircle(centerX, centerY, 5);
    display->setColor(BLACK);
    display->fillCircle(centerX, centerY, 3);
    display->setColor(WHITE);

    // Draw hop-1 nodes (filled circles)
    for (const auto &np : hop1Nodes) {
        display->fillCircle(np.x, np.y, 3);
    }

    // Draw hop-2 nodes (hollow circles)
    for (const auto &np : hop2Nodes) {
        display->drawCircle(np.x, np.y, 3);
    }

    // Draw hop-3+ nodes (small dots) - only on larger screens
    if (screenH > 64) {
        for (const auto &np : hop3Nodes) {
            display->setPixel(np.x, np.y);
            display->setPixel(np.x + 1, np.y);
            display->setPixel(np.x, np.y + 1);
            display->setPixel(np.x + 1, np.y + 1);
        }
    }

    // Draw node count summary at bottom
    display->setFont(FONT_SMALL);
    display->setTextAlignment(TEXT_ALIGN_LEFT);
    char countStr[32];
    int totalVisible = hop1Nodes.size() + hop2Nodes.size() + hop3Nodes.size();
    snprintf(countStr, sizeof(countStr), "%d nodes", totalVisible);
    display->drawString(2, screenH - FONT_HEIGHT_SMALL, countStr);

    // Draw legend on right
    display->setTextAlignment(TEXT_ALIGN_RIGHT);
    display->drawString(screenW - 2, screenH - FONT_HEIGHT_SMALL,
        hop1Nodes.size() > 0 ? "1-hop" : "");
}

} // namespace MeshTopologyRenderer
} // namespace graphics
#endif
