#include "al/core.hpp"
#include "al/core/math/al_Random.hpp"
#include <vector>
#include "Gamma/SamplePlayer.h"

#ifdef BUILD_VR
#include "module/openvr/al_VRApp.hpp"
#define App VRApp
#endif

using namespace al;
using namespace std;
using namespace gam;

struct MyApp : public App
{
  public:
    Mesh shapes;

    int proximity = 10;

    void onMessage(osc::Message &m) override
    {
        if (m.addressPattern() == "/proximity") //
            m >> proximity;
    }

    void onDraw(Graphics &g) override
    {
        g.clear(proximity / 255.0);
        g.depthTesting(true);
        g.lighting(true);
        g.meshColor(); // with lighting, use mesh's color array
        g.draw(shapes);
    }

    gam::SamplePlayer<float, gam::ipl::Linear, gam::phsInc::Loop> samplePlayer[10];

    void onCreate() override
    {
        for (int i = 0; i < 2000; ++i)
        {
            int Nv = rnd::prob(0.5)
                         ? (rnd::prob(0.5) ? addCube(shapes) : addDodecahedron(shapes))
                         : addIcosahedron(shapes);

            // Scale and translate the newly added shape
            Mat4f xfm;
            xfm.setIdentity();
            xfm.scale(Vec3f(rnd::uniform(1., 0.1), rnd::uniform(1., 0.1), rnd::uniform(1., 0.1)));
            xfm.translate(Vec3f(rnd::uniformS(8.), rnd::uniformS(8.), rnd::uniformS(8.)));
            shapes.transform(xfm, shapes.vertices().size() - Nv);

            // Color newly added vertices
            for (int i = 0; i < Nv; ++i)
            {
                float f = float(i) / Nv;
                shapes.color(HSV(f * 0.1 + 0.2, 1, 1));
            }
        }
        shapes.decompress();
        shapes.generateNormals();

        nav().pos(0, 0, 24);

        samplePlayer[0].load("../01.wav");
        samplePlayer[1].load("../02.wav");
        samplePlayer[2].load("../03.wav");
        samplePlayer[3].load("../04.wav");
        //
        Sync::master().spu(audioIO().fps());
    }

    void onSound(AudioIOData &io) override
    {
        while (io())
        {
            float f = 0;
            for (int i = 0; i < 2; i++)
                f += samplePlayer[i]();
            f = f / 2;
            io.out(0) = f * (proximity / 255.0);
            io.out(1) = f * (1 - (proximity / 255.0));
        }
    }
};

int main()
{
    MyApp app;
    app.initAudio();
    app.start();
}
