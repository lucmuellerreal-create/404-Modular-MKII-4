#include "LEDDisplay.h"
namespace {
juce::Path segment(float x,float y,float length,float thickness,bool vertical)
{
    juce::Path p;
    const float t=thickness*0.5f;
    p.startNewSubPath(x,y+t);p.lineTo(x+t,y);p.lineTo(x+length-t,y);p.lineTo(x+length,y+t);
    p.lineTo(x+length-t,y+thickness);p.lineTo(x+t,y+thickness);p.closeSubPath();
    if(vertical)p.applyTransform(juce::AffineTransform::rotation(juce::MathConstants<float>::halfPi,x,y).translated(thickness,0));
    return p;
}
void digit(juce::Graphics& g,int number,float x)
{
    static constexpr unsigned masks[]{0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f};
    // a,b,c,d,e,f,g, in a 44 x 68 logical digit. Slight italic shear.
    const juce::Path parts[]{segment(7,0,31,6,false),segment(35,4,28,6,true),segment(32,36,28,6,true),
        segment(2,62,31,6,false),segment(0,36,28,6,true),segment(3,4,28,6,true),segment(4,31,31,6,false)};
    for(int i=0;i<7;++i){
        const bool on=(masks[number]&(1u<<i))!=0;
        const auto transform=juce::AffineTransform::translation(x,0);
        if(on){g.setColour(juce::Colour(0xffff4139).withAlpha(0.14f));g.strokePath(parts[i],juce::PathStrokeType(4),transform);}
        g.setColour(on?juce::Colour(0xffff4b40):juce::Colour(0xff38282c));g.fillPath(parts[i],transform);
    }
}
}
void LEDDisplay::paint(juce::Graphics& g)
{
    const auto bounds=getLocalBounds().toFloat();
    g.setColour(juce::Colour(0xff080c10));g.fillRect(bounds);
    g.setColour(juce::Colour(0xff606166));g.drawRect(bounds.reduced(1),1);
    auto screen=bounds.reduced(5);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff211a20),screen.getX(),screen.getY(),juce::Colour(0xff171217),screen.getRight(),screen.getBottom(),false));g.fillRect(screen);
    g.saveState();
    const float scale=juce::jmin(screen.getWidth()/148.0f,screen.getHeight()/76.0f);
    g.addTransform(juce::AffineTransform::scale(scale).translated(screen.getCentreX()-74*scale,screen.getCentreY()-34*scale));
    digit(g,value/100,4);digit(g,(value/10)%10,53);digit(g,value%10,102);
    g.restoreState();
    g.setColour(juce::Colours::white.withAlpha(0.025f));g.fillRect(screen.withHeight(screen.getHeight()*0.47f));
}
