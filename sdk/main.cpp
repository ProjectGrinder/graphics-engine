#include <graphics_engine.h>
#include <string>

const std::string shader = R"(
struct VSInput {
    float3 pos: POSITION;
    float4 color: COLOR;
};

struct PSInput {
    float4 pos: SV_POSITION;
    float4 color: COLOR;
};

PSInput vertex(VSInput input) {
    PSInput output;
    output.pos = float4(input.pos, 1.0f);
    output.color = input.color;
    return output;
}

float4 pixel(PSInput input): SV_Target {
    return input.color;
}
)";

struct Vertex {
    float pos[3];
    float color[4];
};

MeshHandler triangle_mesh = 0;
MaterialHandler material = 0;

void on_init(GraphicsEngine::Context *ctx) {
    std::array<Vertex, 3> vertices = {
        {{{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
         {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
         {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}}};
    std::array<uint32_t, 3> indices = {0, 1, 2};

    triangle_mesh = ctx->create_mesh(vertices, indices);
    material = ctx->create_material(shader);
}

void on_update(GraphicsEngine::Context *ctx) {}

void on_render(GraphicsEngine::Context *ctx) {
    ctx->draw(triangle_mesh, material);
}

void on_exit(GraphicsEngine::Context *ctx) {}

GRAPHICS_ENGINE_PLUGIN();
