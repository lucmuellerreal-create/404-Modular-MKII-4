#include "SkinDesign.h"

namespace VinylSkin {
const Layout& layout(int mode)
{
    static const std::array<Layout, 3> layouts {{
        {667,517, {8,2,657,513}, {92,95,505,113}, {91,265,506,214}, {219,240,253,253},
         {{{124,100,104,104},{294,100,104,104},{462,100,104,104}}},
         {255,324,179,90},{135,301,49,113},{505,301,49,113},{209,325,6,88},{475,325,6,88},{490,444,81,25},{543,16,77,18},{429,172},{136,454}},
        {688,469, {18,15,655,442}, {38,65,616,359}, {38,65,616,359}, {},
         {{{133,112,104,104},{294,112,104,104},{455,112,104,104}}},
         {251,309,190,80},{122,302,49,91},{521,302,49,91},{200,321,6,72},{486,321,6,72},{548,432,91,20},{290,26,92,18},{51,442},{51,442}},
        {684,524, {6,1,674,519}, {89,95,507,114}, {89,265,507,215}, {217,240,253,253},
         {{{122,100,104,104},{291,100,104,104},{460,100,104,104}}},
         {252,324,180,90},{133,301,49,113},{503,301,49,113},{208,325,6,88},{473,325,6,88},{488,444,81,25},{550,16,77,18},{427,172},{134,454}}
    }};
    return layouts[static_cast<std::size_t>(juce::jlimit(0, 2, mode))];
}
juce::Colour ink(int mode) { return juce::Colour(mode == 0 ? 0xffe7eeee : 0xff263235); }
void text(juce::Graphics& g, const juce::String& value, juce::Rectangle<float> bounds,
          float size, juce::Colour colour, bool bold)
{
    g.setColour(colour);
    g.setFont(juce::Font(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain)));
    g.drawText(value, bounds, juce::Justification::centred, false);
}
void lamp(juce::Graphics& g, juce::Point<float> centre, bool on, juce::Colour colour)
{
    const juce::Rectangle<float> r(centre.x - 5, centre.y - 5, 10, 10);
    if (on) {
        g.setGradientFill(juce::ColourGradient(colour.withAlpha(0.35f), centre.x, centre.y,
                                             colour.withAlpha(0.0f), centre.x + 9, centre.y, true));
        g.fillEllipse(r.expanded(5));
    }
    g.setColour(juce::Colour(0xff11191c)); g.fillEllipse(r);
    g.setGradientFill(juce::ColourGradient(on ? colour.brighter(0.5f) : colour.darker(0.8f),
        r.getCentreX() - 1, r.getY() + 1, on ? colour.darker(0.3f) : juce::Colour(0xff2b3031), r.getRight(), r.getBottom(), true));
    g.fillEllipse(r.reduced(1.5f));
    g.setColour(juce::Colours::white.withAlpha(on ? 0.6f : 0.13f)); g.fillEllipse(r.getX() + 2, r.getY() + 2, 2.5f, 2);
}
namespace {
void metal(juce::Graphics& g, juce::Rectangle<float> r, bool dark, float radius)
{
    juce::ColourGradient grad(dark ? juce::Colour(0xff747c82) : juce::Colour(0xffdce7e9), r.getX(), r.getY(),
        dark ? juce::Colour(0xff444952) : juce::Colour(0xffbacbce), r.getRight(), r.getBottom(), false);
    grad.addColour(0.25, dark ? juce::Colour(0xff565d66) : juce::Colour(0xffcfdddf));
    grad.addColour(0.72, dark ? juce::Colour(0xff51555e) : juce::Colour(0xffd4dfe0));
    g.setGradientFill(grad); g.fillRoundedRectangle(r, radius);
    g.saveState(); g.reduceClipRegion(r.toNearestInt());
    for (int x = static_cast<int>(r.getX()) + 3; x < r.getRight() - 3; x += 3) {
        g.setColour(juce::Colours::white.withAlpha(x % 9 == 0 ? 0.024f : 0.011f));
        g.drawVerticalLine(x, r.getY() + radius, r.getBottom() - radius);
    }
    g.restoreState();
    g.setColour(juce::Colours::black.withAlpha(0.4f)); g.drawRoundedRectangle(r, radius, 2);
    g.setColour(juce::Colours::white.withAlpha(dark ? 0.12f : 0.6f)); g.drawRoundedRectangle(r.reduced(3), radius - 2, 1);
}
void panel(juce::Graphics& g, juce::Rectangle<float> r, float radius)
{
    g.setColour(juce::Colours::black.withAlpha(0.2f)); g.fillRoundedRectangle(r.translated(1, 3).expanded(2), radius);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff526368), r.getX(), r.getY(), juce::Colour(0xff243438), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle(r, radius);
    g.setColour(juce::Colour(0xff728285).withAlpha(0.5f)); g.drawRoundedRectangle(r.reduced(2), radius - 1, 2);
    g.setColour(juce::Colour(0xff142126).withAlpha(0.7f)); g.drawRoundedRectangle(r.reduced(5), radius - 3, 1);
}
void screw(juce::Graphics& g, float x, float y)
{
    g.setColour(juce::Colours::black.withAlpha(0.28f)); g.fillEllipse(x - 8, y - 6, 17, 17);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xfff0f4ee), x - 5, y - 7, juce::Colour(0xff738588), x + 6, y + 7, false));
    g.fillEllipse(x - 7, y - 7, 14, 14);
    g.setColour(juce::Colour(0xff637072)); g.drawEllipse(x - 4, y - 4, 8, 8, 1);
    g.drawLine(x - 3, y - 1, x + 3, y + 1, 1.3f); g.drawLine(x + 1, y - 3, x - 1, y + 3, 1.3f);
    juce::Path shine; shine.addCentredArc(x, y, 6, 6, 0, 3.6f, 5.5f, true);
    g.setColour(juce::Colours::white.withAlpha(0.6f)); g.strokePath(shine, juce::PathStrokeType(1));
}
void faderScale(juce::Graphics& g, juce::Rectangle<int> r, bool output, int mode)
{
    const float cx = static_cast<float>(r.getCentreX());
    const float top = static_cast<float>(r.getY() + 9), bottom = static_cast<float>(r.getBottom() - 9);
    g.setColour(juce::Colour(0xff66777a).withAlpha(0.55f));
    for (int i = 0; i <= 6; ++i) {
        const float y = top + (bottom - top) * static_cast<float>(i) / 6;
        g.drawLine(cx - 23, y, cx - 17, y, 1); g.drawLine(cx + 17, y, cx + 23, y, 1);
    }
    const float x = output ? cx + 32 : cx - 59;
    const auto col = mode == 1 ? juce::Colour(0xffbfc9c7) : juce::Colour(0xffbdced0);
    text(g, "+12", {x, top - 7, 30, 12}, 8, col);
    text(g, "-24", {x, bottom - 4, 30, 12}, 8, col);
}
}
void paint(juce::Graphics& g, int mode, bool effect, bool peak)
{
    const auto& l = layout(mode);
    const auto white = juce::Colour(0xffe2e9e6);
    g.fillAll(juce::Colour(0xff22282b));
    metal(g, l.shell, mode == 0, mode == 1 ? 8.0f : 21.0f);
    if (mode == 1) {
        g.setColour(juce::Colour(0xff191c1d)); g.fillRect(l.top);
        text(g, "SP VINYL LAB", {39,25,139,18}, 15, ink(mode));
        text(g, "VINYL SIMULATOR", {39,45,139,11}, 8, ink(mode));
        text(g, "3DR VINYL", {489,26,162,26}, 25, ink(mode));
        const auto red = juce::Colour(0xffa64750);
        g.setColour(red); g.drawLine(54,85,246,85,1.5f); g.drawLine(446,85,638,85,1.5f);
        text(g, "EFFECTS CONTROL", {255,77,181,18}, 15, red);
        g.setColour(juce::Colour(0xffa6aca6)); g.drawLine(53,272,639,272,1.5f);
    } else {
        panel(g, l.top, 58); panel(g, l.bottom, 47);
        if (mode == 0) {
            text(g, "4SX", {202,15,121,43}, 43, juce::Colour(0xffe6a164));
            text(g, "VINYL", {328,15,158,43}, 43, white);
        } else text(g, "40G VINYL", {198,15,287,43}, 43, ink(mode));
        text(g, "VINYL SIMULATOR", {242,59,200,13}, 9, ink(mode).withAlpha(0.75f));
        const auto r = l.ring;
        g.setColour(juce::Colours::black.withAlpha(0.3f)); g.fillEllipse(r.translated(1,4));
        g.setGradientFill(juce::ColourGradient(juce::Colour(0xff698085), r.getX(),r.getY(),juce::Colour(0xff152328),r.getRight(),r.getBottom(),false));
        g.fillEllipse(r);
        g.setColour(juce::Colour(0xff304348)); g.fillEllipse(r.reduced(6));
        auto inner = r.reduced(17);
        juce::ColourGradient face(mode == 2 ? juce::Colour(0xff17526d) : juce::Colour(0xff313637), inner.getCentreX(),inner.getY(),
            mode == 2 ? juce::Colour(0xff1f243e) : juce::Colour(0xff1c2022),inner.getX(),inner.getCentreY(),true);
        if (mode == 2) { face.addColour(0.25,juce::Colour(0xff254e78)); face.addColour(0.7,juce::Colour(0xff222643)); }
        g.setGradientFill(face); g.fillEllipse(inner);
        g.setColour(juce::Colour(0xff76878b)); g.drawEllipse(inner,1.5f);
        g.setColour(juce::Colour(0xff17272c)); g.drawEllipse(r.reduced(10),3);
        for (float x : {l.shell.getX()+26, l.shell.getRight()-(mode==2?27.0f:10.0f)}) {
            screw(g,x,32); screw(g,x,static_cast<float>(l.height)-29);
        }
        text(g,"PEAK",{l.peak.x-22,l.peak.y+11,44,12},8,white);
        text(g,"EFFECT",{l.effect.x+10,l.effect.y-6,58,13},9,white);
    }
    const char* upper[3] {mode == 0 ? "RNG" : "CMP", "NOZ", "FLU"};
    const char* lower[3] {mode == 0 ? "FREQUENCY RANGE" : "COMPRESSION", "VINYL NOISE", "WOW / FLUTTER"};
    for (int i = 0; i < 3; ++i) {
        auto r = l.knobs[static_cast<std::size_t>(i)].toFloat();
        text(g, mode == 1 ? "CTRL " + juce::String(i+1) : juce::String(upper[i]),
             {r.getCentreX()-70,r.getY()-(mode==1?17.0f:26.0f),140,18},mode==1?13.0f:16.0f,mode==1?white:ink(mode));
        text(g,lower[i],{r.getCentreX()-82,r.getBottom()+(mode==1?31.0f:22.0f),164,17},mode==1?12.0f:9.0f,mode==1?white:ink(mode));
    }
    const auto in = l.input.toFloat(), out = l.output.toFloat();
    text(g,"INPUT",{in.getCentreX()-45,in.getY()-23,90,17},14,white);
    text(g,"OUTPUT",{out.getCentreX()-48,out.getY()-23,96,17},14,white);
    faderScale(g,l.input,false,mode); faderScale(g,l.output,true,mode);
    text(g,"VINYL SIM",{l.display.getCentreX()-105.0f,static_cast<float>(l.display.getY())-35,210,25},mode==1?18.0f:22.0f,white);
    text(g,"SP VINYL LAB",mode==1?juce::Rectangle<float>(60,434,140,16):juce::Rectangle<float>(250,static_cast<float>(l.height)-18,184,10),mode==1?11.0f:7.0f,ink(mode));
    if (mode != 1) lamp(g,l.peak,peak,juce::Colour(0xffff5140));
    lamp(g,l.effect,effect,juce::Colour(0xffff6148));
}
}
