#include <iostream>
#include "PresetManager.h"
#include "NeuronNetwork.h"
int main() { 
  PresetManager::createPresetDirectory(); 
  NeuronNetwork net; 
  PresetManager::loadFactoryPreset(net, "drum_pattern"); 
  return 0; 
}
