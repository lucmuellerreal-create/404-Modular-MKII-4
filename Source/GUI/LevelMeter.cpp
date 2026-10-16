#include "LevelMeter.h"
#include <cmath>
void LevelMeter::advance(float target,float seconds)
{
    level=juce::jmax(target,level*std::exp(-seconds/0.25f));
    repaint();
}
void LevelMeter::paint(juce::Graphics& g)
{
    const auto r=getLocalBounds().toFloat();
    const float db=juce::Decibels::gainToDecibels(level,-60.0f);
    const float normal=juce::jlimit(0.0f,1.0f,(db+60.0f)/60.0f);
    constexpr int count=16;
    const float step=r.getHeight()/count;
    for(int i=0;i<count;++i){
        const auto col=i>=15?juce::Colour(0xffff4e3d):(i>=13?juce::Colour(0xffe3b674):(skin==1?juce::Colour(0xffdce99d):juce::Colour(0xff9cdee0)));
        g.setColour(col.withAlpha(normal>static_cast<float>(i)/count?1.0f:0.09f));
        g.fillRoundedRectangle(1,r.getBottom()-(static_cast<float>(i)+1)*step+1,r.getWidth()-2,juce::jmax(1.0f,step-2),0.7f);
    }
}
