#pragma once

#define DEFAULT_VERTICES_PER_BATCH 1000

struct Vertex {
    float x, y, z;
    UInt8 r,g,b,a; // Native GL format, RGBA 32bits
};

struct UniformDescription {
    std::string name;
    Int32 location;
    UInt32 type;
    Int32 size;
};

struct AttributeDescription {
    std::string name;
    Int32 location;
    UInt32 type;
    Int32 size;
};

template<typename T>
struct UniformArray {
    const T *base;
    UInt32 size;

    UniformArray(const T *base, UInt32 size) : base(base), size(size) {}
};

template<typename T>
UniformArray<T> MakeUniformArray(const T *inBase, UInt32 hintSize) {
    return UniformArray<T>(inBase, hintSize);
}

class Shader {
public:
    enum BlendFunc {
        BLEND_None,
        BLEND_Transparent
    };

private:
    struct UniformProxy {
        Int32 uniformLocation;

        UniformProxy(Int32 uniformLocation) : uniformLocation(uniformLocation) {}

        template<typename T>
        UniformProxy &operator=(const T val) {
            SetUniform(uniformLocation, val);
            return *this;
        }
        template<typename T>
        UniformProxy &operator=(const UniformArray<T> &val) {
            SetUniform(uniformLocation, val.base, val.size);
            return *this;
        }
    };

public:
    static void Detach();
    static void SetBlendFunc(BlendFunc inBlend);
    static bool blendEnabled;

public:
    Shader();
    ~Shader();

public:
    bool Initialize(const std::string &inVertShader, const std::string &inFragShader, bool hintUseProg=false);
    void Attach();
    void PrintInfo();
    Int32 GetUniformLocation(const std::string &name) const;
    Int32 GetAttributeLocation(const std::string &name) const;

    UniformProxy operator[](const std::string &name) const {
        return UniformProxy (GetUniformLocation(name));
    }

public:
    static void SetUniform(Int32 loc, const glm::mat4 *mat, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::mat4x3 *mat, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::mat3 *mat, UInt32 hintSize);

    static void SetUniform(Int32 loc, const float *x, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::vec2 *xy, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::vec3 *xyz, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::vec4 *xyzw, UInt32 hintSize);

    static void SetUniform(Int32 loc, const UInt32 *x, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::uvec2 *xy, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::uvec3 *xyz, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::uvec4 *xyzw, UInt32 hintSize);

    static void SetUniform(Int32 loc, const Int32 *x, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::ivec2 *xy, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::ivec3 *xyz, UInt32 hintSize);
    static void SetUniform(Int32 loc, const glm::ivec4 *xyzw, UInt32 hintSize);

    static void SetUniform(Int32 loc, const glm::mat4 &mat);
    static void SetUniform(Int32 loc, const glm::mat4x3 &mat);
    static void SetUniform(Int32 loc, const glm::mat3 &mat);

    static void SetUniform(Int32 loc, const float &x);
    static void SetUniform(Int32 loc, const glm::vec2 &xy);
    static void SetUniform(Int32 loc, const glm::vec3 &xyz);
    static void SetUniform(Int32 loc, const glm::vec4 &xyzw);

    static void SetUniform(Int32 loc, const UInt32 &x);
    static void SetUniform(Int32 loc, const glm::uvec2 &xy);
    static void SetUniform(Int32 loc, const glm::uvec3 &xyz);
    static void SetUniform(Int32 loc, const glm::uvec4 &xyzw);

    static void SetUniform(Int32 loc, const Int32 &x);
    static void SetUniform(Int32 loc, const glm::ivec2 &xy);
    static void SetUniform(Int32 loc, const glm::ivec3 &xyz);
    static void SetUniform(Int32 loc, const glm::ivec4 &xyzw);

public:
    UInt32 progId;
    std::unordered_map<std::string, UniformDescription> uniforms;
    std::unordered_map<std::string, AttributeDescription> attributes;
};

class RenderBatcher {
public:
    enum UsageHint {
        USAGE_Stream,
        USAGE_Static,
        USAGE_Dynamic
    };

public:
    RenderBatcher(UInt32 inVerticesPerBatch=DEFAULT_VERTICES_PER_BATCH);
    ~RenderBatcher();

public:
    void SetShader(const Shader &);
    void Queue(float x, float y, float z, UInt8 r, UInt8 g, UInt8 b, UInt8 a=255);
    void Queue(const Vertex *inVertices, UInt32 inNumVertices);
    void Clear();
    void Upload(UsageHint hintUsage=USAGE_Stream);
    void Draw();
    void UploadDraw(bool hintClear=true);

private:
    void UpsizeBatch();

private:
    const Shader *shader;
    UInt32 vboId;
    UInt32 vertsPerBatch;

    std::vector<Vertex> verts;
    UInt32 nVerts;
};
