#include "al/core.hpp"
#include "al/util/al_Asset.hpp"
#include "al/util/al_Image.hpp"
using namespace al;

#include <algorithm>  // max
#include <cstdint>    // uint8_t
#include <vector>
using namespace std;

struct MyApp : App {
  vector<Mesh> mesh;
  Vec3f scene_min, scene_max, scene_center;
  float a = 0.f;  // current rotation angle

  void onCreate() {
    Scene* scene = Scene::import("../asset/face.obj");
    if (scene == nullptr) {
      printf("FAIL\n");
      exit(1);
    }

    scene->getBounds(scene_min, scene_max);
    scene_center = (scene_min + scene_max) / 2.f;
    scene->print();

    Image image;
    mesh.resize(scene->meshes());
    for (int i = 0; i < scene->meshes(); ++i) {
      scene->mesh(i, mesh[i]);
    }
  }

  void onDraw(Graphics& g) {
    g.clear(0.1);

    g.depthTesting(true);
    g.lighting(true);

    g.pushMatrix();

    // rotate it around the y axis
    g.rotate(a, 0.f, 1.f, 0.f);
    a += 0.5;

    // scale the whole asset to fit into our view frustum
    float tmp = scene_max[0] - scene_min[0];
    tmp = std::max(scene_max[1] - scene_min[1], tmp);
    tmp = std::max(scene_max[2] - scene_min[2], tmp);
    tmp = 2.f / tmp;
    g.scale(tmp);

    // center the model
    g.translate(-scene_center);

    for (auto& m : mesh) {
      g.draw(m);
    }

    g.popMatrix();
  }
};

int main(int argc, char* const argv[]) {
  MyApp win;
  win.start();
}
