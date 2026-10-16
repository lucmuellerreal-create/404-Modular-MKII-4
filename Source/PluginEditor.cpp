#include "PluginEditor.h"
SPVinylLabEditor::SPVinylLabEditor(SPVinylLabProcessor& p) : AudioProcessorEditor(p), processor(p)
{
    setLookAndFeel(&look);
    addAndMakeVisible(panel);
    modes.addItemList({"4SX Vinyl", "3DR Vinyl", "40G Vinyl"},1);
    modes.setComponentID("modeSelect");modes.setTooltip("Vinyl mode / skin (host automatable)");
    panel.addAndMakeVisible(modes);panel.addAndMakeVisible(bypass);panel.addAndMakeVisible(display);
    panel.addAndMakeVisible(inputMeter);panel.addAndMakeVisible(outputMeter);panel.addAndMakeVisible(displayCaption);
    display.setComponentID("ledDisplay");display.setInterceptsMouseClicks(false,false);
    displayCaption.setJustificationType(juce::Justification::centred);displayCaption.setInterceptsMouseClicks(false,false);
    bypass.setComponentID("bypass");bypass.setTooltip("Bypass: unprocessed stereo signal");
    bypass.setClickingTogglesState(true);
    for(std::size_t i=0;i<knobs.size();++i){
        panel.addAndMakeVisible(knobs[i]);panel.addAndMakeVisible(values[i]);
        knobs[i].setComponentID("primary"+juce::String(static_cast<int>(i)));
        values[i].setJustificationType(juce::Justification::centred);values[i].setInterceptsMouseClicks(false,false);
        knobs[i].onDragStart=[this]{++activeDrags;};
        knobs[i].onDragEnd=[this]{activeDrags=juce::jmax(0,activeDrags-1);};
        knobs[i].onValueChange=[this,i]{
            if(!rebinding && attachedMode>=0){selectedControl[static_cast<std::size_t>(attachedMode)]=static_cast<int>(i);updateReadouts();}
        };
    }
    for(std::size_t i=0;i<gains.size();++i){
        auto& slider=gains[i];panel.addAndMakeVisible(slider);panel.addAndMakeVisible(gainValues[i]);
        slider.setSliderStyle(juce::Slider::LinearVertical);slider.setTextBoxStyle(juce::Slider::NoTextBox,false,0,0);
        slider.setComponentID(i==0?"inputGain":"outputGain");
        slider.setName(i==0?"Input gain":"Output gain");slider.setTooltip(slider.getName()+" (-24 to +12 dB), double-click: 0 dB");
        slider.setWantsKeyboardFocus(true);slider.setScrollWheelEnabled(true);
        slider.onDragStart=[this]{++activeDrags;};slider.onDragEnd=[this]{activeDrags=juce::jmax(0,activeDrags-1);};
        slider.onValueChange=[this]{if(!rebinding)updateReadouts();};
        gainValues[i].setJustificationType(juce::Justification::centred);gainValues[i].setInterceptsMouseClicks(false,false);
    }
    enabledAttachment=std::make_unique<juce::ParameterAttachment>(*p.parameters.getParameter("enabled"),[this](float value){
        bypass.setToggleState(value<0.5f,juce::dontSendNotification);
        panel.setIndicators(value>=0.5f,peakHold>0);
    });
    bypass.onClick=[this]{enabledAttachment->setValueAsCompleteGesture(bypass.getToggleState()?0.0f:1.0f);};
    enabledAttachment->sendInitialUpdate();
    modeAttachment=std::make_unique<APVTS::ComboBoxAttachment>(p.parameters,"mode",modes);
    modes.onChange=[this]{attachMode(juce::jlimit(0,2,juce::roundToInt(processor.parameters.getRawParameterValue("mode")->load())));};
    setResizable(true,true);
    attachMode(juce::jlimit(0,2,juce::roundToInt(p.parameters.getRawParameterValue("mode")->load())));
    previousTick=juce::Time::getMillisecondCounterHiRes();startTimerHz(60);
}
SPVinylLabEditor::~SPVinylLabEditor()
{
    stopTimer();modes.onChange=nullptr;bypass.onClick=nullptr;
    for(auto& a:attachments)a.reset();for(auto& a:gainAttachments)a.reset();
    enabledAttachment.reset();modeAttachment.reset();
    setLookAndFeel(nullptr);
}
void SPVinylLabEditor::attachMode(int mode)
{
    if(mode==attachedMode || activeDrags>0)return;
    rebinding=true;
    for(auto& a:attachments)a.reset();for(auto& a:gainAttachments)a.reset();
    attachedMode=mode;look.setSkin(mode);panel.setMode(mode);inputMeter.setSkin(mode);outputMeter.setSkin(mode);
    const char* ids[]{mode==0?"tone":"compression","noise","wow"};
    const char* names[]{mode==0?"Frequency range":"Compression","Vinyl noise","Wow and flutter"};
    for(std::size_t i=0;i<knobs.size();++i){
        auto id=SPVinylLabProcessor::parameterID(mode,ids[i]);
        knobs[i].setName(names[i]);knobs[i].setTooltip(juce::String(names[i])+" (0-127)");
        attachments[i]=std::make_unique<APVTS::SliderAttachment>(processor.parameters,id,knobs[i]);
        knobs[i].setDoubleClickReturnValue(true,processor.parameters.getParameter(id)->convertFrom0to1(processor.parameters.getParameter(id)->getDefaultValue()));
    }
    for(std::size_t i=0;i<gains.size();++i){
        gainAttachments[i]=std::make_unique<APVTS::SliderAttachment>(processor.parameters,SPVinylLabProcessor::parameterID(mode,i==0?"input":"output"),gains[i]);
        gains[i].setDoubleClickReturnValue(true,0.0);
    }
    const auto& l=VinylSkin::layout(mode);
    const auto colour=mode==1?juce::Colour(0xffe2e9e6):VinylSkin::ink(mode);
    for(auto& label:values){label.setColour(juce::Label::textColourId,colour);label.setFont(juce::Font(juce::FontOptions(12.0f,juce::Font::bold)));}
    for(auto& label:gainValues){label.setColour(juce::Label::textColourId,juce::Colour(0xffdbe4e2));label.setFont(juce::Font(juce::FontOptions(12.0f,juce::Font::bold)));}
    displayCaption.setColour(juce::Label::textColourId,juce::Colour(0xffe2e9e6));
    displayCaption.setFont(juce::Font(juce::FontOptions(mode==1?12.0f:14.0f,juce::Font::bold)));
    if(auto* c=getConstrainer())c->setFixedAspectRatio(static_cast<double>(l.width)/l.height);
    setResizeLimits(juce::roundToInt(l.width*0.8),juce::roundToInt(l.height*0.8),l.width*2,l.height*2);
    const int width=getWidth()>0?juce::jlimit(juce::roundToInt(l.width*0.8),l.width*2,getWidth()):l.width;
    setSize(width,juce::roundToInt(static_cast<double>(width)*l.height/l.width));
    resized();rebinding=false;updateReadouts();repaint();
}
void SPVinylLabEditor::updateReadouts()
{
    if(attachedMode<0)return;
    for(std::size_t i=0;i<knobs.size();++i)values[i].setText(juce::String(juce::roundToInt(knobs[i].getValue()*127.0)),juce::dontSendNotification);
    for(std::size_t i=0;i<gains.size();++i)gainValues[i].setText(juce::String(gains[i].getValue(),1)+" dB",juce::dontSendNotification);
    const auto index=static_cast<std::size_t>(selectedControl[static_cast<std::size_t>(attachedMode)]);
    display.setValue(juce::roundToInt(knobs[index].getValue()*127.0));
    const char* captions[]{attachedMode==0?"RANGE / VALUE":"CMP / VALUE","NOZ / VALUE","FLU / VALUE"};
    displayCaption.setText(captions[index],juce::dontSendNotification);
}
void SPVinylLabEditor::refreshFromProcessor()
{
    const double now=juce::Time::getMillisecondCounterHiRes();
    const float dt=juce::jlimit(0.001f,0.25f,static_cast<float>((now-previousTick)*0.001));previousTick=now;
    const float in=processor.inputPeak.exchange(0,std::memory_order_relaxed);
    const float out=processor.outputPeak.exchange(0,std::memory_order_relaxed);
    peakHold=juce::jmax(0.0f,peakHold-dt);if(juce::jmax(in,out)>=0.999f)peakHold=0.8f;
    inputMeter.advance(in,dt);outputMeter.advance(out,dt);
    attachMode(juce::jlimit(0,2,juce::roundToInt(processor.parameters.getRawParameterValue("mode")->load())));
    updateReadouts();
    panel.setIndicators(processor.parameters.getRawParameterValue("enabled")->load()>=0.5f,peakHold>0);
}
void SPVinylLabEditor::timerCallback()
{
    refreshFromProcessor();
}
void SPVinylLabEditor::paint(juce::Graphics& g) { g.fillAll(juce::Colour(0xff22282b)); }
void SPVinylLabEditor::resized()
{
    if(attachedMode<0)return;
    const auto& l=VinylSkin::layout(attachedMode);
    const float scale=juce::jmin(static_cast<float>(getWidth())/l.width,static_cast<float>(getHeight())/l.height);
    panel.setTransform(juce::AffineTransform());panel.setBounds(0,0,l.width,l.height);
    panel.setTransform(juce::AffineTransform::scale(scale).translated((getWidth()-l.width*scale)*0.5f,(getHeight()-l.height*scale)*0.5f));
    modes.setBounds(l.mode);bypass.setBounds(l.bypass);display.setBounds(l.display);
    gains[0].setBounds(l.input);gains[1].setBounds(l.output);
    inputMeter.setBounds(l.inputMeter);outputMeter.setBounds(l.outputMeter);
    for(std::size_t i=0;i<knobs.size();++i){knobs[i].setBounds(attachedMode==1?l.knobs[i].expanded(13):l.knobs[i]);values[i].setBounds(l.knobs[i].getX()-10,l.knobs[i].getBottom()+(attachedMode==1?10:1),l.knobs[i].getWidth()+20,18);}
    for(std::size_t i=0;i<gains.size();++i)gainValues[i].setBounds(gains[i].getX()-20,gains[i].getBottom()+1,gains[i].getWidth()+40,20);
    displayCaption.setBounds(l.display.getX()-15,l.display.getBottom()+10,l.display.getWidth()+30,24);
}
