#ifdef __arm__
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#define GLFW_INCLUDE_ES2
#else
#include <GL/glew.h>
#endif

#include <glm/glm.hpp>

#include <cstring>
#include <vector>
#include <iostream>
#include <fstream>

#include "types.hpp"
#include "shader.hpp"


///////////////////////////////////////////////////////////
// Shader Headers
#ifdef __arm__
const char *vheader = "#define IN attribute\n"
    "#define OUT varying\n"
    "precision mediump float\n"
    "precision mediump int\n";

const char *fheader = "#define IN varying\n"
    "#define fragColor gl_FragColor\n"
    "precision mediump float\n"
    "precision mediump int\n";
#else
const char *vheader = "#version 130\n"
    "#define lowp\n"
    "#define mediump\n"
    "#define highp\n"
    "#define IN in\n"
    "#define OUT out\n";

const char *fheader = "#version 130\n"
    "#define lowp\n"
    "#define mediump\n"
    "#define highp\n"
    "#define IN in\n"
    "out vec4 fragColor;\n";
#endif


//////////////////////////////////////////////////////////////////////////
// Implmentation

bool Shader::blendEnabled = false;

RenderBatcher::RenderBatcher( UInt32 vertsPerBatch) : shader(0), vboId(0), vertsPerBatch(vertsPerBatch), nVerts(0) {
    verts.resize(vertsPerBatch);
    glGenBuffers(1, &vboId);
}

RenderBatcher::~RenderBatcher() {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &vboId);
}

void RenderBatcher::SetShader(const Shader &s) {
    shader = &s;
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    UInt32 posIdx = glGetAttribLocation(shader->progId, "pos");
    UInt32 colorIdx = glGetAttribLocation(shader->progId, "color");
    glEnableVertexAttribArray(posIdx); //pos
    glEnableVertexAttribArray(colorIdx); //color
    glVertexAttribPointer(posIdx, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
    glVertexAttribPointer(colorIdx, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)(3*sizeof(float)));
}
void RenderBatcher::Queue(float x, float y, float z, UInt8 r, UInt8 g, UInt8 b, UInt8 a) {
    auto &v = verts[nVerts];
    v.x = x;
    v.y = y;
    v.z = z;
    v.r = r;
    v.g = g;
    v.b = b;
    v.a = a;
    nVerts++;
    if(nVerts == verts.size()) {
        UpsizeBatch();
    }
}

void RenderBatcher::Queue(const Vertex *inVertices, UInt32 inNumVertices) {
    UInt32 inVerticesPos = 0;
    while(nVerts+inNumVertices-inVerticesPos >= verts.size()) {
        UInt32 maxCopy = verts.size() - nVerts;
        std::memcpy(&verts[nVerts], inVertices+inVerticesPos, sizeof(Vertex)*maxCopy);
        inVerticesPos += maxCopy;
        UpsizeBatch();
    }

    std::memcpy(&verts[nVerts], inVertices+inVerticesPos, sizeof(Vertex)*(inNumVertices-inVerticesPos));
    nVerts += inNumVertices;
}

void RenderBatcher::Clear() {
    nVerts = 0;
}

void RenderBatcher::Upload(UsageHint hintUsage) {
    GLenum hint;
    switch(hintUsage)
    {
        case USAGE_Stream:
            hint = GL_STREAM_DRAW;
            break;
        case USAGE_Dynamic:
            hint = GL_DYNAMIC_DRAW;
            break;
        case USAGE_Static:
            hint = GL_STATIC_DRAW;
            break;
    }
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), &verts[0], hint);
}

void RenderBatcher::Draw() {
    glDrawArrays(GL_TRIANGLES, 0, verts.size());
}
void RenderBatcher::UploadDraw(bool clear) {
    
    Upload(USAGE_Stream);
    Draw();
    if(clear) Clear();
}

void RenderBatcher::UpsizeBatch() {
    verts.resize(verts.size() + vertsPerBatch);
}

//////////////////////////////////////////////////////////////////////////

Shader::Shader() : progId(0) {

}

Shader::~Shader() {
    RemoveProg();
    if(progId > 0) glDeleteProgram(progId);
}

bool Shader::Initialize(const std::string &vertshader, const std::string &fragshader, bool useProg) {
    UInt32 vshaderId, fshaderId;

    progId = glCreateProgram();
    vshaderId = glCreateShader(GL_VERTEX_SHADER);
    fshaderId = glCreateShader(GL_FRAGMENT_SHADER);

    if(vertshader.empty() || fragshader.empty()) {
        std::cerr << "Failed to load shader" << std::endl;
        return false;
    }
    const char *vertSource[] = {vheader, vertshader.c_str()};
    const char *fragSource[] = {fheader, fragshader.c_str()};

    glShaderSource(vshaderId, 2, vertSource, 0);
    glShaderSource(fshaderId, 2, fragSource, 0);
    glCompileShader(vshaderId);
    glCompileShader(fshaderId);

    GLint compiled, linked;
    glGetShaderiv(vshaderId, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cerr << "Failed to compile vertshader" << std::endl;
        return false;
    }
    glGetShaderiv(fshaderId, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        std::cerr << "Failed to compile fragshader" << std::endl;
        return false;
    }

    glAttachShader(progId, vshaderId);
    glAttachShader(progId, fshaderId);

#ifndef __arm__
    glBindFragDataLocation(progId, 0, "fragColor");
#endif

    glLinkProgram(progId);

    glGetProgramiv(progId, GL_LINK_STATUS, &linked);
    if (!linked) {
        std::cerr << "Failed to link prog" << std::endl;
        return false;
    }
    glDeleteShader(vshaderId);
    glDeleteShader(fshaderId);

    if(useProg) glUseProgram(progId);

    return true;
}

void Shader::UseProg() {
    glUseProgram(progId);
}

Int32 Shader::GetUniformLocation( const std::string &name ) {
    return glGetUniformLocation(progId, name.c_str());
}
void Shader::SetUniform( Int32 loc, const glm::mat4 *mat, UInt32 size) {
    glUniformMatrix4fv(loc, size, GL_FALSE, (GLfloat*)mat);
}
void Shader::SetUniform( Int32 loc, const glm::mat4x3 *mat, UInt32 size) {
    glUniformMatrix4x3fv(loc, size, GL_FALSE, (GLfloat*)mat);
}
void Shader::SetUniform( Int32 loc, const glm::mat3 *mat, UInt32 size) {
    glUniformMatrix3fv(loc, size, GL_FALSE, (GLfloat*)mat);
}

void Shader::SetUniform(Int32 loc, const float *x, UInt32 size) {
    glUniform1fv(loc, size, x);
}
void Shader::SetUniform(Int32 loc, const glm::vec2 *xy, UInt32 size) {
    glUniform2fv(loc, size, (GLfloat*)xy);
}
void Shader::SetUniform(Int32 loc, const glm::vec3 *xyz, UInt32 size) {
    glUniform3fv(loc, size, (GLfloat*)xyz);
}
void Shader::SetUniform(Int32 loc, const glm::vec4 *xyzw, UInt32 size) {
    glUniform4fv(loc, size, (GLfloat*)xyzw);
}

void Shader::SetUniform(Int32 loc, const UInt32 *x, UInt32 size) {
    glUniform1uiv(loc, size, x);
}
void Shader::SetUniform(Int32 loc, const glm::uvec2 *xy, UInt32 size) {
    glUniform2uiv(loc, size, (GLuint*)xy);
}
void Shader::SetUniform(Int32 loc, const glm::uvec3 *xyz, UInt32 size) {
    glUniform3uiv(loc, size, (GLuint*)xyz);
}
void Shader::SetUniform(Int32 loc, const glm::uvec4 *xyzw, UInt32 size) {
    glUniform4uiv(loc, size, (GLuint*)xyzw);
}

void Shader::SetUniform(Int32 loc, const Int32* x, UInt32 size) {
    glUniform1iv(loc, size, x);
}
void Shader::SetUniform(Int32 loc, const glm::ivec2 *xy, UInt32 size) {
    glUniform2iv(loc, size, (GLint*)xy);
}
void Shader::SetUniform(Int32 loc, const glm::ivec3 *xyz, UInt32 size) {
    glUniform3iv(loc, size, (GLint*)xyz);
}
void Shader::SetUniform(Int32 loc, const glm::ivec4 *xyzw, UInt32 size) {
    glUniform4iv(loc, size, (GLint*)xyzw);
}


void Shader::RemoveProg() {
    glUseProgram(0);
}

void Shader::SetBlendFunc(BlendFunc inBlend) {
    if(inBlend != BLEND_None) {
        if(!blendEnabled) {
            glEnable(GL_BLEND);
            blendEnabled = true;
        }
    }
    switch(inBlend) {
        case BLEND_None:
            glDisable(GL_BLEND);
            blendEnabled = false;
            break;
        case BLEND_Transparent:
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
    }
}
