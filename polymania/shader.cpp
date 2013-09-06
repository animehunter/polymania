#ifdef __arm__
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#define GLFW_INCLUDE_ES2
#else
#include <GL/glew.h>
#endif

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

void RenderBatcher::Render(bool clear) {
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), &verts[0], GL_STREAM_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, verts.size());
    if(clear) Clear();
}

void RenderBatcher::UpsizeBatch() {
    verts.resize(verts.size() + vertsPerBatch);
}

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
