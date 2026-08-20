#include "graphics_engine.h"

struct Vertex {
    float position[3];
    float color[4];
};

struct Matrix4x4 {
    float m[4][4];
};

static MeshHandler g_triangle_mesh = 0;
static MaterialHandler g_triangle_material = 0;
static uint32_t g_key_event_id = 0;

static float g_pos_y = -0.2f;
static float g_velocity_y = 0.0f;
static bool g_is_grounded = true;

const float GROUND_Y = -0.2f;
const float JUMP_IMPULSE = 0.02f;
const float GRAVITY = -0.0008f;

void on_key_event(const SDL_Event *event) {
    if (!event)
        return;
    if (event->type == SDL_EVENT_KEY_DOWN &&
        event->key.scancode == SDL_SCANCODE_SPACE) {
        if (g_is_grounded) {
            g_velocity_y = JUMP_IMPULSE;
            g_is_grounded = false;
        }
    }
}

void on_init(GraphicsEngine::Context *ctx) {
    if (!ctx)
        return;

    const Vertex vertices[] = {
        {{0.0f, 0.3f, 0.0f}, {1.0f, 0.2f, 0.2f, 1.0f}},
        {{0.3f, -0.3f, 0.0f}, {0.2f, 1.0f, 0.2f, 1.0f}},
        {{-0.3f, -0.3f, 0.0f}, {0.2f, 0.2f, 1.0f, 1.0f}}};
    const uint32_t indices[] = {0, 1, 2};

    g_triangle_mesh = ctx->create_mesh(vertices, indices);

    const char *shader_code = R"(
    cbuffer ModelBuffer : register(b0) {
        float4x4 u_model;
    };

    struct VSInput { 
        float3 pos : POSITION; 
        float4 col : COLOR; 
    };
    
    struct PSInput { 
        float4 pos : SV_POSITION; 
        float4 col : COLOR; 
    };

    PSInput vertex(VSInput input) {
        PSInput output;
        output.pos = mul(float4(input.pos, 1.0), u_model);
        output.col = input.col;
        return output;
    }

    float4 pixel(PSInput input) : SV_TARGET { 
        return input.col; 
    }
    )";

    g_triangle_material = ctx->create_material(shader_code);
    g_key_event_id = ctx->event_subscribe(SDL_EVENT_KEY_DOWN, on_key_event);
    g_pos_y = GROUND_Y;
}

void on_update(GraphicsEngine::Context *ctx) {
    (void)ctx;

    if (!g_is_grounded) {
        g_pos_y += g_velocity_y;
        g_velocity_y += GRAVITY;
        if (g_pos_y <= GROUND_Y) {
            g_pos_y = GROUND_Y;
            g_velocity_y = 0.0f;
            g_is_grounded = true;
        }
    }
}

void on_render(GraphicsEngine::Context *ctx) {
    if (!ctx)
        return;

    Matrix4x4 model_matrix = {{{1.0f, 0.0f, 0.0f, 0.0f},
                               {0.0f, 1.0f, 0.0f, 0.0f},
                               {0.0f, 0.0f, 1.0f, 0.0f},
                               {0.0f, g_pos_y, 0.0f, 1.0f}}};

    ctx->bind_vertex_data(g_triangle_material, model_matrix);
    ctx->draw(g_triangle_mesh, g_triangle_material);
}

void on_exit(GraphicsEngine::Context *ctx) {
    if (ctx && g_key_event_id != 0) {
        ctx->event_unsubscribe(SDL_EVENT_KEY_DOWN, g_key_event_id);
    }
    g_triangle_mesh = 0;
    g_triangle_material = 0;
}

GRAPHICS_ENGINE_PLUGIN();
