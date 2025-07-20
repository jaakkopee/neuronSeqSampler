import tkinter as tk
from tkinter import ttk
from neuronSeqBase import Neuron, NeuronNetwork, ActivationFunction


class NeuronSeqGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Neuron Sequence Sampler")
        self.root.geometry("1024x800")
        # Create network
        self.network = NeuronNetwork()
        self.neurons = []
        # Create neurons and add them to the network
        for i in range(4): 
            sample_index = i + 1
            neuron = Neuron(sample_index=sample_index, initial_activation=-0.2, threshold=1.0, decay_rate=1.0)
            self.neurons.append(neuron)
            self.network.add_neuron(neuron)

        # Connect neurons in a matrix
        # Changed from 16x16 to 4x4 for simplicity
        for i in range(4):
            for j in range(4):  
                if i != j:
                    self.network.connect(self.neurons[i], self.neurons[j], weight=0.0)

        self.setup_ui()

    def setup_ui(self):
        # Info display (create this first!)
        self.info_text = tk.Text(self.root, height=20, width=80)
        self.info_text.pack(pady=10)
        
        # Control frame
        control_frame = ttk.Frame(self.root)
        control_frame.pack(pady=10)
        
        # Add a few test sliders instead of all connections
        self.sliders = []
        # Only create sliders for the first 10 connections to avoid UI overflow
        num_sliders = min(16, len(self.network.connections))
        for i in range(num_sliders):
            slider_frame = ttk.Frame(control_frame)
            slider_frame.pack(pady=2)
            
            label = ttk.Label(slider_frame, text=f"Connection {i + 1}:", width=15)
            label.pack(side=tk.LEFT, padx=5)
            
            slider = ttk.Scale(slider_frame, from_=-0.4, to=1.2, orient=tk.HORIZONTAL, length=200,
                               command=lambda value, idx=i: self.update_weight(idx, float(value)))
            slider.set(-0.2)  # Set default value
            slider.pack(side=tk.LEFT, padx=5)
            self.sliders.append(slider)
        self.update_display()

    def neuron_loop(self):
        self.network.activate()
        self.update_display()
        self.root.after(100, self.neuron_loop)

    def update_weight(self, idx, value):
        if idx < len(self.network.connections):
            connection = self.network.connections[idx]
            connection.weight = value
            print(f"Updated weight from Neuron {connection.source} to Neuron {connection.target} to {value:.2f}")
        else:
            print(f"Invalid connection index: {idx}")
        self.update_display()

    def update_display(self):
        self.info_text.delete(1.0, tk.END)
        for i, neuron in enumerate(self.neurons):
            info = f"Neuron {i+1} Activation: {neuron.activation:.4f}\n"
            info += f"Neuron {i+1} History: {[round(x, 4) for x in neuron.activation_history[-5:]]}\n"
            self.info_text.insert(tk.END, info)
        self.info_text.insert(tk.END, "\n")

if __name__ == "__main__":
    try:
        print("Starting Neuron Sequence Sampler GUI...")
        root = tk.Tk()
        print("Tkinter root window created")
        app = NeuronSeqGUI(root)
        print("GUI application initialized")
        print(f"Created {len(app.neurons)} neurons with {len(app.network.connections)} connections")
        app.neuron_loop()  # Start the neuron activation loop
        print("Starting main loop...")
        root.mainloop()
    except Exception as e:
        print(f"Error: {e}")
        import traceback
        traceback.print_exc()


