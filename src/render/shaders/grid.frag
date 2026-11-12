#version 330 core

in vec2 vNdcPos;
out vec4 FragColor;

uniform vec3 uGridColor;
uniform vec3 uAxisColor;
uniform float uZoom;
uniform mat4 uInvViewProj;
uniform float uCellSize;
uniform float uSubCellSize;

const float SUBGRID_ALPHA = 0.3;
const float AXIS_WIDTH_MULT = 2.0;
const float GRID_ALPHA = 0.7;

void main() {
    vec4 ndc = vec4(vNdcPos, 0.0, 1.0);
    vec4 worldPos4 = uInvViewProj * ndc;
    vec2 worldPos = worldPos4.xy / worldPos4.w;

    float cellSize = uCellSize;
    float lineWidth = 0.01 / (uZoom / 100.0);

    // Calculate grid lines
    vec2 gridPos = worldPos / cellSize;
    vec2 gridFrac = abs(fract(gridPos - 0.5) - 0.5);
    vec2 distToLine = gridFrac * cellSize;

    // Main grid lines (thicker)
    float lineX = 1.0 - smoothstep(0.0, lineWidth, distToLine.x);
    float lineY = 1.0 - smoothstep(0.0, lineWidth, distToLine.y);
    float isLine = max(lineX, lineY);

    // Sub-grid for finer detail when zoomed in
    float subCellSize = uSubCellSize;
    vec2 subGridPos = worldPos / subCellSize;
    vec2 subGridFrac = fract(subGridPos);
    vec2 subDistToLine = min(subGridFrac, 1.0 - subGridFrac) * subCellSize;

    float subLineWidth = lineWidth * 1.0;
    float subLineX = 1.0 - smoothstep(0.0, subLineWidth, subDistToLine.x);
    float subLineY = 1.0 - smoothstep(0.0, subLineWidth, subDistToLine.y);
    float subLine = max(subLineX, subLineY);

    // Blend sub-grid with lower opacity
    isLine = max(isLine, subLine * SUBGRID_ALPHA);

    // Axes with consistent width
    float axisWidth = lineWidth * AXIS_WIDTH_MULT;
    float axisX = 1.0 - smoothstep(0.0, axisWidth, abs(worldPos.x));
    float axisY = 1.0 - smoothstep(0.0, axisWidth, abs(worldPos.y));
    float axes = max(axisX, axisY);

    isLine = max(isLine, axes);

    vec3 gridColor = uGridColor;
    vec3 axisColor = uAxisColor;

    // Adjust alpha based on zoom for better visibility
    float alpha = isLine * GRID_ALPHA;

    vec3 finalColor = gridColor;
    if (axes > 0.5) {
        finalColor = axisColor;
        // alpha = 1.0; // Axes always fully visible
    }

    FragColor = vec4(finalColor, alpha);
}