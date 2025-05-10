#pragma once

namespace cologne
{
    struct Color
    {
        Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
        float r;
        float g;
        float b;
        float a;
        static Color hsv_to_rgb(float h, float s, float v, float a = 1.0f)
        {
            float r, g, b;
            int i = floor(h * 6);
            float f = h * 6 - i;
            float p = v * (1 - s);
            float q = v * (1 - f * s);
            float t = v * (1 - (1 - f) * s);
            switch (i % 6)
            {
                case 0: r = v, g = t, b = p; break;
                case 1: r = q, g = v, b = p; break;
                case 2: r = p, g = v, b = t; break;
                case 3: r = p, g = q, b = v; break;
                case 4: r = t, g = p, b = v; break;
                case 5: r = v, g = p, b = q; break;
            }
            return {r, g, b, a};
        }
    };

inline Color crimson = {220.0f / 255.0f, 20.0f / 255.0f, 60.0f / 255.0f, 1.0f};      // #DC143C
inline Color red = {255.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f};          // #FF0000
inline Color maroon = {128.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f};         // #800000
inline Color brown = {165.0f / 255.0f, 42.0f / 255.0f, 42.0f / 255.0f, 1.0f};        // #A52A2A
inline Color misty_rose = {255.0f / 255.0f, 228.0f / 255.0f, 225.0f / 255.0f, 1.0f}; // #FFE4E1
inline Color salmon = {250.0f / 255.0f, 128.0f / 255.0f, 114.0f / 255.0f, 1.0f};     // #FA8072
inline Color coral = {255.0f / 255.0f, 127.0f / 255.0f, 80.0f / 255.0f, 1.0f};       // #FF7F50
inline Color orange_red = {255.0f / 255.0f, 69.0f / 255.0f, 0.0f / 255.0f, 1.0f};    // #FF4500
inline Color chocolate = {210.0f / 255.0f, 105.0f / 255.0f, 30.0f / 255.0f, 1.0f};   // #D2691E
inline Color orange = {255.0f / 255.0f, 165.0f / 255.0f, 0.0f / 255.0f, 1.0f};      // #FFA500
inline Color gold = {255.0f / 255.0f, 215.0f / 255.0f, 0.0f / 255.0f, 1.0f};        // #FFD700
inline Color ivory = {255.0f / 255.0f, 255.0f / 255.0f, 240.0f / 255.0f, 1.0f};     // #FFFFF0
inline Color yellow = {255.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f};      // #FFFF00
inline Color olive = {128.0f / 255.0f, 128.0f / 255.0f, 0.0f / 255.0f, 1.0f};       // #808000
inline Color yellow_green = {154.0f / 255.0f, 205.0f / 255.0f, 50.0f / 255.0f, 1.0f}; // #9ACD32
inline Color lawn_green = {124.0f / 255.0f, 252.0f / 255.0f, 0.0f / 255.0f, 1.0f};  // #7CFC00
inline Color chartreuse = {127.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f};  // #7FFF00
inline Color lime = {0.0f / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f, 1.0f};          // #00FF00
inline Color green = {0.0f / 255.0f, 128.0f / 255.0f, 0.0f / 255.0f, 1.0f};         // #008000
inline Color spring_green = {0.0f / 255.0f, 255.0f / 255.0f, 127.0f / 255.0f, 1.0f}; // #00FF7F
inline Color aquamarine = {127.0f / 255.0f, 255.0f / 255.0f, 212.0f / 255.0f, 1.0f}; // #7FFFD4
inline Color turquoise = {64.0f / 255.0f, 224.0f / 255.0f, 208.0f / 255.0f, 1.0f};  // #40E0D0
inline Color azure = {240.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f};     // #F0FFFF. Note: There are multiple definitions of Azure. This is the X11/CSS one. Another common one is #007FFF.
inline Color cyan = {0.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f};        // #00FFFF (Same as Aqua)
inline Color teal = {0.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f};        // #008080
inline Color lavender = {230.0f / 255.0f, 230.0f / 255.0f, 250.0f / 255.0f, 1.0f};   // #E6E6FA
inline Color blue = {0.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 1.0f};          // #0000FF
inline Color navy = {0.0f / 255.0f, 0.0f / 255.0f, 128.0f / 255.0f, 1.0f};         // #000080
inline Color blue_violet = {138.0f / 255.0f, 43.0f / 255.0f, 226.0f / 255.0f, 1.0f}; // #8A2BE2
inline Color indigo = {75.0f / 255.0f, 0.0f / 255.0f, 130.0f / 255.0f, 1.0f};       // #4B0082
inline Color dark_violet = {148.0f / 255.0f, 0.0f / 255.0f, 211.0f / 255.0f, 1.0f}; // #9400D3
inline Color plum = {221.0f / 255.0f, 160.0f / 255.0f, 221.0f / 255.0f, 1.0f};       // #DDA0DD
inline Color magenta = {255.0f / 255.0f, 0.0f / 255.0f, 255.0f / 255.0f, 1.0f};     // #FF00FF (Same as Fuchsia)
inline Color purple = {128.0f / 255.0f, 0.0f / 255.0f, 128.0f / 255.0f, 1.0f};      // #800080
inline Color red_violet = {199.0f / 255.0f, 21.0f / 255.0f, 133.0f / 255.0f, 1.0f}; // #C71585
inline Color tan = {210.0f / 255.0f, 180.0f / 255.0f, 140.0f / 255.0f, 1.0f};       // #D2B48C
inline Color beige = {245.0f / 255.0f, 245.0f / 255.0f, 220.0f / 255.0f, 1.0f};     // #F5F5DC
inline Color slate_gray = {112.0f / 255.0f, 128.0f / 255.0f, 144.0f / 255.0f, 1.0f}; // #708090
inline Color dark_slate_gray = {47.0f / 255.0f, 79.0f / 255.0f, 79.0f / 255.0f, 1.0f}; // #2F4F4F
inline Color white = {255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 1.0f};     // #FFFFFF
inline Color white_smoke = {245.0f / 255.0f, 245.0f / 255.0f, 245.0f / 255.0f, 1.0f}; // #F5F5F5
inline Color light_gray = {211.0f / 255.0f, 211.0f / 255.0f, 211.0f / 255.0f, 1.0f}; // #D3D3D3
inline Color silver = {192.0f / 255.0f, 192.0f / 255.0f, 192.0f / 255.0f, 1.0f};     // #C0C0C0
inline Color dark_gray = {169.0f / 255.0f, 169.0f / 255.0f, 169.0f / 255.0f, 1.0f};  // #A9A9A9
inline Color gray = {128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 1.0f};      // #808080
inline Color dim_gray = {105.0f / 255.0f, 105.0f / 255.0f, 105.0f / 255.0f, 1.0f};  // #696969
inline Color black = {0.0f / 255.0f, 0.0f / 255.0f, 0.0f / 255.0f, 1.0f};
}
