#include "VinylLookAndFeel.h"
#include "SkinDesign.h"
#include <cmath>
VinylLookAndFeel::VinylLookAndFeel()
{
    setColour(juce::ComboBox::textColourId, juce::Colour(0xff85918f));
    setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(0xff232a2d));
    setColour(juce::PopupMenu::textColourId, juce::Colour(0xffe1e6df));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xff626f70));
}
juce::Font VinylLookAndFeel::getComboBoxFont(juce::ComboBox&)
{ return juce::Font(juce::FontOptions(10.0f)); }
void VinylLookAndFeel::drawComboBox(juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox& box)
{
    g.setColour(juce::Colours::black.withAlpha(box.isMouseOverOrDragging() ? 0.17f : 0.055f));
    g.fillRoundedRectangle(0,0,static_cast<float>(w),static_cast<float>(h),3);
    juce::Path arrow;
    const float x = static_cast<float>(w)-10, y = static_cast<float>(h)*0.5f;
    arrow.startNewSubPath(x-3,y-1); arrow.lineTo(x,y+2); arrow.lineTo(x+3,y-1);
    g.setColour(juce::Colour(0xff85918f)); g.strokePath(arrow,juce::PathStrokeType(1));
}
void VinylLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                      float position, float start, float end, juce::Slider& slider)
{
    auto r = juce::Rectangle<float>(static_cast<float>(x),static_cast<float>(y),static_cast<float>(width),static_cast<float>(height));
    const float size = juce::jmin(r.getWidth(),r.getHeight());
    r = r.withSizeKeepingCentre(size,size).reduced(skin==1?13.0f:2.0f);
    const auto centre = r.getCentre();
    const float radius = r.getWidth()*0.5f;
    const float angle = start + position * (end-start);
    if (skin == 1) {
        g.setColour(juce::Colour(0xff9d414b));
        for (int i=0;i<=24;++i) {
            const float a = start + (end-start)*static_cast<float>(i)/24;
            const auto p1=centre.getPointOnCircumference(radius+5,a), p2=centre.getPointOnCircumference(radius+11,a);
            g.drawLine({p1,p2},3);
        }
    }
    g.setColour(juce::Colours::black.withAlpha(0.38f)); g.fillEllipse(r.translated(1,4).expanded(1));
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff323c3e),r.getX(),r.getY(),juce::Colour(0xff080e11),r.getRight(),r.getBottom(),false));
    g.fillEllipse(r);
    g.setColour(juce::Colour(0xff101719)); g.drawEllipse(r,2);
    g.setColour(juce::Colour(0xff526064).withAlpha(0.55f)); g.drawEllipse(r.reduced(4),1);
    auto face=r.reduced(skin==1?6.0f:8.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff2c3537),face.getX(),face.getY(),juce::Colour(0xff12181b),face.getRight(),face.getBottom(),false));
    g.fillEllipse(face);
    g.setColour(juce::Colour(0xff151c1e)); g.drawEllipse(face.reduced(4),2);
    for(int i=0;i<9;++i) {
        const float a=3.8f+static_cast<float>(i)*0.31f;
        const auto a1=centre.getPointOnCircumference(radius*0.68f,a), a2=centre.getPointOnCircumference(radius*0.77f,a);
        g.setColour(juce::Colours::black.withAlpha(skin==1?0.4f:0.13f));g.drawLine({a1,a2},skin==1?4.0f:2.0f);
    }
    const float capRadius=skin==1?14.0f:19.0f;
    auto cap=juce::Rectangle<float>(capRadius*2,capRadius*2).withCentre(centre);
    g.setColour(juce::Colours::black.withAlpha(0.6f));g.fillEllipse(cap.expanded(3).translated(0,1));
    g.setGradientFill(juce::ColourGradient(skin==1?juce::Colour(0xffe4e997):juce::Colour(0xfff0f5ed),cap.getX(),cap.getY(),
        skin==1?juce::Colour(0xffadb969):juce::Colour(0xffc3d3d4),cap.getRight(),cap.getBottom(),false));
    g.fillEllipse(cap);
    const auto tip=centre.getPointOnCircumference(radius*0.82f,angle);
    if(skin==1) {
        g.setColour(juce::Colour(0xffd9e395));
        g.drawLine({centre.getPointOnCircumference(capRadius+2,angle),tip},3);
    } else {
        g.setColour(juce::Colour(0xff182528));
        const auto notch=centre.getPointOnCircumference(capRadius-2,angle);
        g.fillEllipse(notch.x-4,notch.y-4,8,8);
        g.setColour(juce::Colour(0xffd5e6de));g.fillEllipse(tip.x-1.8f,tip.y-1.8f,3.6f,3.6f);
    }
    if (slider.hasKeyboardFocus(true)) {
        g.setColour(juce::Colour(0xffdd9f62).withAlpha(0.7f));g.drawEllipse(r.expanded(1),1.5f);
    }
}
void VinylLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                      float pos, float, float, juce::Slider::SliderStyle, juce::Slider& slider)
{
    const float cx=static_cast<float>(x)+static_cast<float>(width)*0.5f;
    const float top=static_cast<float>(y), length=static_cast<float>(height);
    g.setColour(juce::Colour(0xff718386).withAlpha(0.45f));g.fillRoundedRectangle(cx-3.5f,top,7,length,3);
    g.setColour(juce::Colour(0xff142125));g.fillRoundedRectangle(cx-2,top+1,4,length-2,2);
    auto cap=juce::Rectangle<float>(cx-17,pos-8,34,16);
    g.setColour(juce::Colours::black.withAlpha(0.5f));g.fillRoundedRectangle(cap.translated(0,2).expanded(1),2);
    const auto light=skin==1?juce::Colour(0xffdce2bd):juce::Colour(0xffd5e5e7);
    g.setGradientFill(juce::ColourGradient(light,cap.getX(),cap.getY(),juce::Colour(0xff778c91),cap.getX(),cap.getBottom(),false));
    g.fillRoundedRectangle(cap,2);
    g.setColour(juce::Colour(0xffe3ece5).withAlpha(0.8f));g.drawRoundedRectangle(cap.reduced(1),1,1);
    g.setColour(juce::Colour(0xff485d62));g.fillRect(cap.reduced(5,6));
    if(slider.hasKeyboardFocus(true)) {g.setColour(juce::Colour(0xffdd9f62));g.drawRoundedRectangle(cap.expanded(1),2,1);}
}
void VinylLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& b, bool hover, bool down)
{
    auto r=b.getLocalBounds().toFloat().reduced(1);
    const bool bypass=b.getToggleState();
    const auto c=skin==1?juce::Colour(0xffd6ddaf):juce::Colour(0xffc3d9df);
    g.setColour(juce::Colour(0xff122025));g.fillRoundedRectangle(r.expanded(1),3);
    g.setGradientFill(juce::ColourGradient((down?c.darker(0.2f):c).brighter(hover?0.1f:0.0f),r.getX(),r.getY(),c.darker(0.3f),r.getX(),r.getBottom(),false));
    g.fillRoundedRectangle(r,2);
    g.setColour(bypass?juce::Colour(0xff964330):juce::Colour(0xff233235));
    g.setFont(juce::Font(juce::FontOptions(10.5f,juce::Font::bold)));
    g.drawText("BYPASS",r,juce::Justification::centred);
    if(bypass) {g.setColour(juce::Colour(0xffb75742));g.fillEllipse(r.getX()+5,r.getCentreY()-1.5f,3,3);}
}
