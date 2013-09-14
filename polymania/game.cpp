#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "types.hpp"
#include "context.hpp"
#include "controller.hpp"
#include "timer.hpp"
#include "asyncmodel.hpp"
#include "resource.hpp"
#include "shader.hpp"
#include "object.hpp"
#include "game.hpp"

//////////////////////////////////////////////////////////////////////////
class GameSystemImplementation {
public:
    Int32 width, height;
    ResourceManager resMan;
    RenderBatcher batch;
    Shader shader;
    float pcamx, pcamy, pcamz;
    float camx, camy, camz;

    GameSystemImplementation(Int32 inWidth, Int32 inHeight);
    ~GameSystemImplementation();

    void Update(GameSystem &game, const std::shared_ptr<Controller> &inController);
    void Draw(GameSystem &game);

    void SetPerspective(Int32 width, Int32 height) {
        shader["projection"] = glm::perspective(60.0f, float(width)/float(height), 0.1f, 100.0f);
    }
    void LookAt(const glm::vec3 &eye, const glm::vec3 &target, const glm::vec3 &up) {
        shader["modelview"] = glm::lookAt(eye, target, up);
    }
};

//////////////////////////////////////////////////////////////////////////
// Messages
bool GameSystem::OnCloseWindow(const Event &ev) {
    quitRequested = true;
    return true;
}

bool GameSystem::OnResizedWindow(const Event &ev) {
    impl->width = ev["inWidth"];
    impl->height = ev["inHeight"];
    impl->SetPerspective(impl->width, impl->height);
    return true;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
GameSystem::GameSystem(const Event &ev) : Object(ev), interp(0.0), quitRequested(false),
                                          impl(new GameSystemImplementation(ev["inWidth"], ev["inHeight"])) {

}

GameSystem::~GameSystem() {
}

void GameSystem::Update( GameSystem &game, const std::shared_ptr<Controller> &k ) {
    impl->Update(*this, k);
}

void GameSystem::Draw(GameSystem &game) {
    impl->Draw(*this);
}

Int32 GameSystem::GetWidth() const {
    return impl->width;
}

Int32 GameSystem::GetHeight() const {
    return impl->height;
}

//////////////////////////////////////////////////////////////////////////
GameSystemImplementation::GameSystemImplementation(Int32 inWidth, Int32 inHeight) : width(inWidth), height(inHeight), camx(0), camy(0), camz(6.0f) {
    resMan.AddResourceLoader<ResourceShader>("glf");
    resMan.AddResourceLoader<ResourceShader>("glv");

    Shader::SetBlendFunc(Shader::BLEND_Transparent);

    std::string vertshader = resMan.Load<ResourceShader>("shaders/default.glv")->string;
    std::string fragshader = resMan.Load<ResourceShader>("shaders/default.glf")->string;
    shader.Initialize(vertshader, fragshader, true);
    SetPerspective(width, height);
    LookAt(glm::vec3(0.0f, 0.0f, camz), glm::vec3(camx, camy, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    batch.SetShader(shader);

    Vertex verts[] = {
        // front
        {0.0f,  0.0f, 1.0f, 255, 0, 0, 255},
        {-1.0f,  0.0f, 1.0f, 0, 0, 255, 150},
        {-1.0f, -1.0f, 1.0f, 0, 255, 0, 0},
        {0.0f,  0.0f, 1.0f, 255, 0, 0, 255},
        {-1.0f,  -1.0f, 1.0f, 0, 255, 0, 0},
        {0.0f, -1.0f, 1.0f, 0, 0, 255, 150},

        // back
        {0.0f,  -1.0f, 0.0f, 255, 0, 0, 255},
        {-1.0f, -1.0f, 0.0f, 0, 0, 255, 150},
        {-1.0f,  0.0f, 0.0f, 0, 255, 0, 0},
        {0.0f,  -1.0f, 0.0f, 255, 0, 0, 255},
        {-1.0f,  0.0f, 0.0f, 0, 255, 0, 0},
        {0.0f,   0.0f, 0.0f, 0, 0, 255, 150},

        // top
        {0.0f,  0.0f, 0.0f, 255, 0, 0, 255},
        {-1.0f,  0.0f, 0.0f, 0, 0, 255, 150},
        {-1.0f,  0.0f, 1.0f, 0, 255, 0, 0},
        {0.0f,  0.0f, 0.0f, 255, 0, 0, 255},
        {-1.0f,  0.0f, 1.0f, 0, 255, 0, 0},
        {0.0f,  0.0f, 1.0f, 0, 0, 255, 150},

        // bottom
        {0.0f,  -1.0f, 1.0f, 255, 0, 0, 255},
        {-1.0f, -1.0f, 1.0f, 0, 0, 255, 150},
        {-1.0f, -1.0f, 0.0f, 0, 255, 0, 0},
        {0.0f,  -1.0f, 1.0f, 255, 0, 0, 255},
        {-1.0f, -1.0f, 0.0f, 0, 255, 0, 0},
        {0.0f,  -1.0f, 0.0f, 0, 0, 255, 150},

        // left
        {-1.0f,  0.0f, 1.0f, 255, 0, 0, 255},
        {-1.0f,  0.0f, 0.0f, 0, 0, 255, 150},
        {-1.0f, -1.0f, 0.0f, 0, 255, 0, 0},
        {-1.0f,  0.0f, 1.0f, 255, 0, 0, 255},
        {-1.0f, -1.0f, 0.0f, 0, 255, 0, 0},
        {-1.0f, -1.0f, 1.0f, 0, 0, 255, 150},

        // right
        {0.0f,  0.0f, 0.0f, 255, 0, 0, 255},
        {0.0f,  0.0f, 1.0f, 0, 0, 255, 150},
        {0.0f, -1.0f, 1.0f, 0, 255, 0, 0},
        {0.0f,  0.0f, 0.0f, 255, 0, 0, 255},
        {0.0f, -1.0f, 1.0f, 0, 255, 0, 0},
        {0.0f, -1.0f, 0.0f, 0, 0, 255, 150}
    };
    batch.Queue(verts, sizeof(verts)/sizeof(Vertex));
    batch.Upload(RenderBatcher::USAGE_Static);
}

GameSystemImplementation::~GameSystemImplementation() {
}

void GameSystemImplementation::Update(GameSystem &game, const std::shared_ptr<Controller> &k) {
    pcamx = camx; 
    pcamy = camy; 
    pcamz = camz;
    if(k->left)  camx -= 0.1f;
    if(k->right)  camx += 0.1f;
    if(k->up) camy += 0.1f;
    if(k->down) camy -= 0.1f;
    if(k->x) camz -= 0.1f;
    if(k->y) camz += 0.1f;
}
void GameSystemImplementation::Draw(GameSystem &game){
    if(pcamx != camx || pcamy != camy || pcamz != camz) {
        float icamx = pcamx+(camx-pcamx)*float(game.interp);
        float icamy = pcamy+(camy-pcamy)*float(game.interp);
        float icamz = pcamz+(camz-pcamz)*float(game.interp);
        LookAt(glm::vec3(icamx, icamy, icamz), glm::vec3(icamx, icamy, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader["camx"] = icamx;
        shader["camy"] = icamy;
    }
    batch.Draw();
}


