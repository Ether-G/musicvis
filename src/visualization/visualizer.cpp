#include "visualization/visualizer.h"
#include "render/render_engine.h"

Visualizer::Visualizer(std::shared_ptr<RenderEngine> renderEngine)
    : m_renderEngine(renderEngine)
{
}

Visualizer::~Visualizer() {
}