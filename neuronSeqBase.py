import numpy as np
from nSSampler import SampleKit

# Initialize the SampleKit
sample_kit = SampleKit()

#add 16 samples in the directory ./samples/808
for i in range(1, 6):
    sample_name = f"{i}"
    file_path = f"./samples/girliepop/{sample_name}.wav"
    sample_kit.add_sample(sample_name, file_path)

class ActivationFunction:
    @staticmethod
    def sigmoid(x):
        import math
        return 1 / (1 + math.exp(-x))

    @staticmethod
    def relu(x):
        return max(0, x)

    @staticmethod
    def tanh(x):
        import math
        return math.tanh(x)

    @staticmethod
    def linear(x):
        return x

    @staticmethod
    def softmax(x):
        e_x = np.exp(x - np.max(x))
        return e_x / e_x.sum(axis=0)

class Connection:
    def __init__(self, source_neuron, target_neuron, weight=1.0):
        self.source = source_neuron
        self.target = target_neuron
        self.weight = weight

    def get_activation(self):
        source_activation = self.source.activate(self.weight)
        target_activation = self.target.activate(source_activation)
        return target_activation
    
class Neuron:
    def __init__(self, sample_index, initial_activation=0.0, threshold=1.0, decay_rate=1.0, activation_function=ActivationFunction.linear):
        self.sample_index = sample_index
        self.activation = initial_activation
        self.activation_history = []
        self.max_history_length = 100
        self.activation_history.append(self.activation)
        self.threshold = threshold
        self.decay_rate = decay_rate
        self.activation_function = activation_function
    
    def activate(self, input_value):
        self.activation += input_value

        if self.activation > self.threshold:
            self.playSample()
            self.activation -= self.decay_rate

        if self.activation < -self.threshold:
            self.activation += self.threshold

        # Maintain activation history
        if len(self.activation_history) >= self.max_history_length:
            self.activation_history.pop(0)
        self.activation_history.append(self.activation)
        # Apply activation function
        return self.activation_function(self.activation)
    
    def playSample(self):
        sample_name = f"{self.sample_index}"
        if sample_name in sample_kit.samples:
            sample_kit.play_sample(sample_name)
        else:
            print(f"Sample '{sample_name}' not found.")

class NeuronNetwork:
    def __init__(self):
        self.neurons = []
        self.connections = []

    def add_neuron(self, neuron):
        self.neurons.append(neuron)

    def connect(self, source_neuron, target_neuron, weight=1.0):
        connection = Connection(source_neuron, target_neuron, weight)
        self.connections.append(connection)

    def activate(self):
        for connection in self.connections:
            connection.get_activation()

# Example usage
if __name__ == "__main__":
    neuron1 = Neuron(initial_activation=0.5, threshold=1.0, decay_rate=0.1)
    neuron2 = Neuron(initial_activation=0.2, threshold=1.0, decay_rate=0.1)

    network = NeuronNetwork()
    network.add_neuron(neuron1)
    network.add_neuron(neuron2)

    network.connect(neuron1, neuron2, weight=0.5)

    network.activate()

    print("Neuron 1 Activation:", neuron1.activation)
    print("Neuron 2 Activation:", neuron2.activation)
    print("Neuron 1 Activation History:", neuron1.activation_history)
    print("Neuron 2 Activation History:", neuron2.activation_history)
