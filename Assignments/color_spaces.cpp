#include "al/core.hpp"
#include "al/util/al_Image.hpp" //getting the image function
using namespace al;
using namespace std;

struct MyApp : App
{
    Mesh mesh{Mesh::POINTS};
    vector<Vec3f> position[4]; //an array of 4 vectors using Vec3f

    void onCreate() override
    {
        Image image;
        const char *fileName = "../color_space.jpg";
        if (!image.load(fileName))
        {
            exit(1);
        }

        nav().pullBack(7);

        Image::RGBAPix<uint8_t> pixel;
        for (int row = 0; row < image.height(); row++)
        {
            for (int column = 0; column < image.width(); column++)
            {
                image.read(pixel, column, row);
                float x = (float)column / image.width() - 0.5;
                float y = (float)row / image.width() - 0.5;
                mesh.vertex(x, y, 0);
                position[0].push_back(Vec3f(x, y, 0));
                mesh.color(picel.r / 256.0, pixel.g / 256.0, pixel.b / 256.0);
            }
        }

        for (const Color &c : mesh.colors())
            position[0].push_back(Vec3f(c.r, c.g, c.b) - Vec3f(0.5, 0.5, 0.5));

  const auto& c = mesh.colors())
  for (int i = 0; i < c.size(); ++i)
  {
      HSV
  }
