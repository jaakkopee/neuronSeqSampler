#include "Connection.h"
#include "Neuron.h"

Connection::Connection(Neuron* sourceNeuron, Neuron* targetNeuron, float weight)
    : source(sourceNeuron)
    , target(targetNeuron)
    , weight(weight)
{
}

void Connection::activate() {
    if (source && target) {
        float sourceActivation = source->getActivation();
        float weightedInput = sourceActivation * weight;
        target->activate(weightedInput);
    }
}