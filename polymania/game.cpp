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
    float pcamx, pcamy;
    float camx, camy;

    GameSystemImplementation(Int32 inWidth, Int32 inHeight);
    ~GameSystemImplementation();

    void Update(GameSystem &game, const std::shared_ptr<Controller> &inController);
    void Draw(GameSystem &game);

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
    return true;
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
GameSystem::GameSystem(const Event &ev) : interp(0.0), quitRequested(false),
                                          impl(new GameSystemImplementation(ev["inWidth"], ev["inHeight"])) {

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
GameSystemImplementation::GameSystemImplementation(Int32 inWidth, Int32 inHeight) : width(inWidth), height(inHeight), camx(0), camy(0) {
    resMan.AddResourceLoader<ResourceShader>("glf");
    resMan.AddResourceLoader<ResourceShader>("glv");

    Shader::SetBlendFunc(Shader::BLEND_Transparent);

    std::string vertshader = resMan.Load<ResourceShader>("shaders/default.glv")->string;
    std::string fragshader = resMan.Load<ResourceShader>("shaders/default.glf")->string;
    shader.Initialize(vertshader, fragshader, true);
    shader["projection"] = glm::perspective(60.0f, float(width)/float(height), 0.1f, 100.0f);
    shader["modelview"] = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(camx, camy, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    batch.SetShader(shader);

    batch.Queue( 0.0f,  0.0f, 1.0f, 255, 120, 120, 255);
    batch.Queue(-0.5f,  0.0f, 1.0f, 0, 255, 255, 150);
    batch.Queue(-0.5f, -0.5f, 1.0f, 0, 255, 0, 0);

    batch.Queue( 0.0f,  0.0f, 1.0f, 255, 120, 120, 255);
    batch.Queue(-0.5f,  -0.5f, 1.0f, 0, 255, 0, 0);
    batch.Queue( 0.0f, -0.5f, 1.0f, 0, 255, 255, 150);
    batch.Upload(RenderBatcher::USAGE_Static);
}

GameSystemImplementation::~GameSystemImplementation() {
}

void GameSystemImplementation::Update(GameSystem &game, const std::shared_ptr<Controller> &k) {
    pcamx = camx; 
    pcamy = camy; 
    if(k->left)  camx -= 0.1f;
    if(k->right)  camx += 0.1f;
    if(k->up) camy += 0.1f;
    if(k->down) camy -= 0.1f;
}
void GameSystemImplementation::Draw(GameSystem &game){
    if(pcamx != camx || pcamy != camy) {
        float icamx = pcamx+(camx-pcamx)*float(game.interp);
        float icamy = pcamy+(camy-pcamy)*float(game.interp);
        shader["modelview"] = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(icamx, icamy, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader["camx"] = icamx;
        shader["camy"] = icamy;
    }
    batch.Draw();
}


