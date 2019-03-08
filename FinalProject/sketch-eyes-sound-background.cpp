#include "al/core.hpp"
#include "al/core/app/al_DistributedApp.hpp"
#include "module/img/loadImage.hpp"
using namespace al;

#include <vector>
using namespace std;

#include "Gamma/SamplePlayer.h"
using namespace gam;

typedef gam::SamplePlayer<float, gam::ipl::Linear, gam::phsInc::Loop>
    SoundPlayer;

Vec3f r(float m = 1) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * m;
}

struct Thing {
  Texture texture;
  Mesh mesh;
};

struct SimpleVoice : PositionedVoice {
  Parameter whichSound{"whichSound"};
  SoundPlayer soundPlayer;

  SimpleVoice() {
    registerParameterAsField(whichSound);
    //
    pose().pos(r(5));
  }

  virtual void onProcess(AudioIOData& io) override {
    while (io()) {
      io.out(0) = soundPlayer();
    }
  }

  virtual void onProcess(Graphics& g) override {
    Thing* thing = (Thing*)userData();
    thing->texture.bind();
    g.draw(thing->mesh);
    thing->texture.unbind();
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

  Thing thing;

  Texture back;
  Mesh b;

  void onCreate() override {
    scene.showWorldMarker(false);

    scene.registerSynthClass<SimpleVoice>();
    scene.setDefaultUserData(&thing);
    scene.allocatePolyphony("SimpleVoice", 11);
    scene.prepare(audioIO());

    for (float i = 0.1; i < 11; ++i) {
      auto* freeVoice = scene.getVoice<SimpleVoice>();
      auto params = std::vector<float>{i};
      freeVoice->setParamFields(params);
      scene.triggerOn(freeVoice);
    }

    auto imageData = imgModule::loadImage("../asset/eye.jpg");
    if (imageData.data.size() == 0) {
      cout << "failed to load image" << endl;
    }
    thing.texture.create2D(imageData.width, imageData.height);
    thing.texture.submit(imageData.data.data(), GL_RGBA, GL_UNSIGNED_BYTE);

    addSphereWithTexcoords(thing.mesh);
    thing.mesh.generateNormals();

    {
      auto imageData = imgModule::loadImage("../asset/background.jpg");
      if (imageData.data.size() == 0) {
        cout << "failed to load image" << endl;
      }
      back.create2D(imageData.width, imageData.height);
      back.submit(imageData.data.data(), GL_RGBA, GL_UNSIGNED_BYTE);

      addSphereWithTexcoords(b);
      b.generateNormals();
    }

    nav().pos(0, 0, 10);
    lens().far(1000);
  }

  void onDraw(Graphics& g) override {
    g.clear(0.2);
    g.depthTesting(true);
    g.texture();

    // this is one way to do a background; texture a sphere and make it very big
    g.pushMatrix();
    back.bind();
    g.scale(900);
    g.draw(b);
    back.unbind();
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
