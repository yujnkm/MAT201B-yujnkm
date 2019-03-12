#include "al/core.hpp"
#include "al/core/app/al_DistributedApp.hpp"
#include "al/util/al_Asset.hpp"
#include "module/img/loadImage.hpp"
using namespace al;

#include <vector>
using namespace std;

#include "Gamma/SamplePlayer.h"
using namespace gam;

typedef SamplePlayer<float, gam::ipl::Linear, gam::phsInc::Loop> SoundPlayer;

struct SphereTexture {
  Texture texture;
  Mesh mesh;

  void draw(Graphics& g) {
    texture.bind();
    g.draw(mesh);
    texture.unbind();
  }

  void load(string fileName) {
    auto imageData = imgModule::loadImage(fileName);
    if (imageData.data.size() == 0) {
      cout << "failed to load image" << endl;
    }
    texture.create2D(imageData.width, imageData.height);
    texture.submit(imageData.data.data(), GL_RGBA, GL_UNSIGNED_BYTE);
    addSphereWithTexcoords(mesh, 1, 100);
    mesh.generateNormals();
  }
};

struct EyeballChello : PositionedVoice {
  Parameter whichSound{"whichSound"};
  SoundPlayer soundPlayer;

  EyeballChello() { registerParameterAsField(whichSound); }

  virtual void onProcess(AudioIOData& io) override {
    while (io()) {
      io.out(0) = soundPlayer();
    }
  }

  virtual void onProcess(Graphics& g) override {
    ((SphereTexture*)userData())->draw(g);
  }

  virtual void onTriggerOn() override {
    char fileName[100];
    sprintf(fileName, "../asset/%02d.wav", int(whichSound));
    cout << fileName << endl;
    soundPlayer.load(fileName);
  }
};

struct SharedState {
  Pose pose;
};

struct MyApp : DistributedApp<SharedState> {
  DistributedScene scene{PolySynth::TIME_MASTER_CPU};

  SphereTexture eyeball;
  SphereTexture backdrop;

  vector<Mesh> mesh;
  Vec3f scene_min, scene_max, scene_center;

  void onCreate() override {
    scene.showWorldMarker(false);
    scene.registerSynthClass<EyeballChello>();
    scene.setDefaultUserData(&eyeball);
    scene.allocatePolyphony("EyeballChello", 11);
    scene.prepare(audioIO());

    registerDynamicScene(scene);

    for (float i = 0.1; i < 11; ++i) {
      auto* freeVoice = scene.getVoice<EyeballChello>();
      auto params = std::vector<float>{i};
      freeVoice->setParamFields(params);

      using rnd::uniformS;
      freeVoice->pose().pos(Vec3f(uniformS(), uniformS(), uniformS()) * 5);
      freeVoice->pose().quat(
          Quatf(uniformS(), uniformS(), uniformS(), uniformS()).normalize());
      scene.triggerOn(freeVoice);
    }

    eyeball.load("../asset/eye.jpg");
    eyeball.mesh.scale(0.4);
    backdrop.load("../asset/background.jpg");

    Scene* scene = Scene::import("../asset/face.obj");
    if (scene == nullptr) {
      printf("FAIL\n");
      exit(1);
    }

    scene->getBounds(scene_min, scene_max);
    scene_center = (scene_min + scene_max) / 2.f;
    scene->print();

    mesh.resize(scene->meshes());
    for (int i = 0; i < scene->meshes(); ++i) {
      mesh[i].scale(5);
      scene->mesh(i, mesh[i]);
    }

    // nav().pos(0, 0, 10);
    lens().far(1000);
  }

  void onDraw(Graphics& g) override {
    g.clear(0.2);
    g.depthTesting(true);

    // this is one way to do a background; texture a sphere and make it very big
    g.pushMatrix();
    g.scale(900);
    g.texture();
    g.lighting(false);
    backdrop.draw(g);
    g.popMatrix();

    g.pushMatrix();
    // scale the whole asset to fit into our view frustum
    float tmp = scene_max[0] - scene_min[0];
    tmp = std::max(scene_max[1] - scene_min[1], tmp);
    tmp = std::max(scene_max[2] - scene_min[2], tmp);
    tmp = 2.f / tmp;
    g.scale(tmp);
    g.translate(-scene_center);  // center the model
    g.lighting(true);
    g.color(1);
    for (auto& m : mesh) {
      g.draw(m);
    }
    g.popMatrix();

    g.lighting(false);
    g.texture();
    eyeball.draw(g);
    scene.render(g);
  }

  virtual void onSound(AudioIOData& io) override {
    scene.render(io);
    //
  }

  void simulate(double dt) override {
    // if (app.isPrimary()) {
    state().pose = nav();
    cout << "simulate" << endl;
  }

  void onAnimate(double dt) override {
    if (hasRole(ROLE_RENDERER)) {
      pose() = state().pose;
    }
    cout << "onAnimate" << endl;
    //
  }
};

int main() {
  MyApp app;
  app.initAudio();
  Domain::master().spu(app.audioIO().framesPerSecond());
  app.start();
}
