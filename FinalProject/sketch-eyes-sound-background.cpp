// use
// - DynamicScene

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

struct SharedState {
  Pose pose;
};

struct MyApp : DistributedApp<SharedState> {
  vector<SoundPlayer> soundPlayer;
  Texture t;
  Mesh m;
  vector<Vec3f> eye;

  Texture back;
  Mesh b;

  void onCreate() {
    auto imageData = imgModule::loadImage("../asset/eye.jpg");
    if (imageData.data.size() == 0) {
      cout << "failed to load image" << endl;
    }
    t.create2D(imageData.width, imageData.height);
    t.submit(imageData.data.data(), GL_RGBA, GL_UNSIGNED_BYTE);

    addSphereWithTexcoords(m);
    m.generateNormals();

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

    for (int i = 0; i < 10; i++) {
      eye.push_back(r(5));
    }

    nav().pos(0, 0, 10);
    lens().far(1000);
    cout << lens().far() << endl;
  }

  void onDraw(Graphics& g) {
    g.clear(0.2);
    g.depthTesting(true);
    g.texture();

    // this is one way to do a background; texture a sphere and make it very big
    g.pushBack();
    back.bind();
    g.scale(900);
    g.draw(b);
    back.unbind();
    g.popBack();

    t.bind();
    for (auto& p : eye) {
      g.pushMatrix();
      g.translate(p);
      g.draw(m);
      g.popMatrix();
    }
    t.unbind();
  }

  void onSound(AudioIOData& io) override {
    Sync::master().spu(audioIO().fps());  // XXX put this somewhere else!
    while (io()) {
      float f = soundPlayer[0]();
      io.out(0) = f;
      io.out(1) = f;
    }
  }
};

int main() {
  MyApp app;
  for (int i = 1; i < 11; ++i) {
    app.soundPlayer.emplace_back();
    char fileName[100];
    sprintf(fileName, "../asset/%02d.wav", i);
    cout << fileName << endl;
    app.soundPlayer.back().load(fileName);
  }
  app.initAudio();
  app.start();
}
