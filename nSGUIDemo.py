import tkinter as tk
from tkinter import ttk
import matplotlib.pyplot as plt
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from neuronSeqBase import Neuron, NeuronNetwork, ActivationFunction

class NeuronGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Neuron Network Visualizer")
        self.root.geometry("600x400")
        
        # Create network
        self.network = NeuronNetwork()
        self.neuron1 = Neuron(initial_activation=0.5)
        self.neuron2 = Neuron(initial_activation=0.2)
        
        self.network.add_neuron(self.neuron1)
        self.network.add_neuron(self.neuron2)
        self.network.connect(self.neuron1, self.neuron2, weight=0.5)
        
        self.setup_ui()
    
    def setup_ui(self):
        # Control frame
        control_frame = ttk.Frame(self.root)
        control_frame.pack(pady=10)
        
        # Buttons
        ttk.Button(control_frame, text="Activate Network", 
                  command=self.activate_network).pack(side=tk.LEFT, padx=5)
        ttk.Button(control_frame, text="Reset", 
                  command=self.reset_network).pack(side=tk.LEFT, padx=5)
        
        # Info display
        self.info_text = tk.Text(self.root, height=8, width=60)
        self.info_text.pack(pady=10)
        
        self.update_display()
    
    def activate_network(self):
        self.network.activate()
        self.update_display()
    
    def reset_network(self):
        self.neuron1.activation = 0.5
        self.neuron2.activation = 0.2
        self.neuron1.activation_history = [0.5]
        self.neuron2.activation_history = [0.2]
        self.update_display()
    
    def update_display(self):
        self.info_text.delete(1.0, tk.END)
        info = f"Neuron 1 Activation: {self.neuron1.activation:.4f}\n"
        info += f"Neuron 2 Activation: {self.neuron2.activation:.4f}\n"
        info += f"Neuron 1 History: {[round(x, 4) for x in self.neuron1.activation_history[-5:]]}\n"
        info += f"Neuron 2 History: {[round(x, 4) for x in self.neuron2.activation_history[-5:]]}\n"
        self.info_text.insert(tk.END, info)

if __name__ == "__main__":
    root = tk.Tk()
    app = NeuronGUI(root)
    root.mainloop()
