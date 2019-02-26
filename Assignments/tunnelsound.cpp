#include "al/core.hpp"
#include "al/core/math/al_Random.hpp"
#include <vector>
#include "Gamma/SamplePlayer.h"

using namespace al;
using namespace std;
using namespace gam;

struct MyApp : public App
{
  public:
    Mesh shapes;

    int proximity;

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

    gam::SamplePlayer<float, gam::ipl::Linear, gam::phsInc::Loop> samplePlayer;

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

        samplePlayer.load("../01.wav");
        samplePlayer.load("../02.wav");
        //

        samplePlayer.loop();
        Sync::master().spu(audioIO().fps());
    }

    void onSound(AudioIOData &io) override
    {
        while (io())
        {
            float f = samplePlayer();
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
