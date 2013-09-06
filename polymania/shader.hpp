#pragma once

#define DEFAULT_VERTICES_PER_BATCH 1000

struct Vertex {
    float x, y, z;
    UInt8 r,g,b,a; // Native GL format, RGBA 32bits
};

class Shader {
public:
    enum BlendFunc {
        BLEND_None,
        BLEND_Transparent
    };

public:
    static void RemoveProg();
    static void SetBlendFunc(BlendFunc inBlend);
    static bool blendEnabled;

public:
    Shader();
    ~Shader();

public:
    bool Initialize(const std::string &inVertShader, const std::string &inFragShader, bool hintUseProg=false);
    void UseProg();

public:
    UInt32 progId;
};

class RenderBatcher {
public:
    RenderBatcher(UInt32 inVerticesPerBatch=DEFAULT_VERTICES_PER_BATCH);
    ~RenderBatcher();

public:
    void SetShader(const Shader &);
    void Queue(float x, float y, float z, UInt8 r, UInt8 g, UInt8 b, UInt8 a=255);
    void Queue(const Vertex *inVertices, UInt32 inNumVertices);
    void Clear();
    void Render(bool clear=true);
    void SetTransparency(bool enable);

private:
    void UpsizeBatch();

private:
    const Shader *shader;
    UInt32 vboId;
    UInt32 vertsPerBatch;

    std::vector<Vertex> verts;
    UInt32 nVerts;
};
