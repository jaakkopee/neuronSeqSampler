#include "Connection.h"
#include "Neuron.h"

Connection::Connection(Neuron* sourceNeuron, Neuron* targetNeuron, float weight)
    : source(sourceNeuron)
    , target(targetNeuron)
    , weight(weight)
{
}

void Connection::activate(float weightMultiplier) {
    if (source && target) {
        float sourceActivation = source->getActivation();
        float effectiveWeight = weight * weightMultiplier;
        float weightedInput = sourceActivation * effectiveWeight;
        target->activate(weightedInput);
    }
}