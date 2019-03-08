#include "al/core.hpp"
#include "al/core/app/al_DistributedApp.hpp"
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

  EyeballChello() {
    registerParameterAsField(whichSound);

    using rnd::uniformS;
    pose().pos(Vec3f(uniformS(), uniformS(), uniformS()) * 5);
    pose().quat(
        Quatf(uniformS(), uniformS(), uniformS(), uniformS()).normalize());
  }

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
  DynamicScene scene{8};

  SphereTexture eyeball;
  SphereTexture backdrop;

  void onCreate() override {
    scene.showWorldMarker(false);
    scene.registerSynthClass<EyeballChello>();
    scene.setDefaultUserData(&eyeball);
    scene.allocatePolyphony("EyeballChello", 11);
    scene.prepare(audioIO());

    for (float i = 0.1; i < 11; ++i) {
      auto* freeVoice = scene.getVoice<EyeballChello>();
      auto params = std::vector<float>{i};
      freeVoice->setParamFields(params);
      scene.triggerOn(freeVoice);
    }

    eyeball.load("../asset/eye.jpg");
    backdrop.load("../asset/background.jpg");

    nav().pos(0, 0, 10);
    lens().far(1000);
  }

  void onDraw(Graphics& g) override {
    g.clear(0.2);
    g.depthTesting(true);
    g.texture();

    // this is one way to do a background; texture a sphere and make it very big
    g.pushMatrix();
    g.scale(900);
    backdrop.draw(g);
    g.popMatrix();

    scene.render(g);
  }

  virtual void onSound(AudioIOData& io) override {
    scene.render(io);
    //
  }
};

int main() {
  MyApp app;
  app.initAudio();
  Domain::master().spu(app.audioIO().framesPerSecond());
  app.start();
}
