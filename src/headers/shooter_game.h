
#ifndef COMPONENT_ARCHITECTURE_H
#define COMPONENT_ARCHITECTURE_H

#include "engine.h"
#include "renderer.h"

static const size_t RENDER_BUFFER_MAX = 256;

void load_shooter();
void update_shooter();
void render_shooter();
void render_shooter_ui();

#endif