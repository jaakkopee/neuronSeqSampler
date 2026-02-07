#pragma once
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <SFML/Graphics.hpp>

/**
 * @brief Base class for all modular components in the neuronSeqSampler GUI2 system
 * 
 * Provides a common interface for:
 * - Input/Output port management
 * - Signal routing and processing
 * - Parameter access and manipulation
 * - GUI representation
 */
class ModularComponent {
public:
    /**
     * @brief Port types for module connections
     */
    enum class PortType {
        AUDIO_IN,           // Audio input (samples)
        AUDIO_OUT,          // Audio output (samples)
        CONTROL_IN,         // Control input (modulation, triggers, etc.)
        CONTROL_OUT,        // Control output
        RHYTHM_IN,          // Rhythm/beat information input
        RHYTHM_OUT,         // Rhythm/beat information output
        ONSET_IN,           // Onset detection input
        ONSET_OUT,          // Onset detection output
        NEURON_IN,          // Neuron network input
        NEURON_OUT          // Neuron network output
    };

    /**
     * @brief Port definition
     */
    struct Port {
        std::string name;
        PortType type;
        int index;          // Port index within this module
        bool isInput;       // true for input, false for output
        
        Port(const std::string& n, PortType t, int idx, bool input)
            : name(n), type(t), index(idx), isInput(input) {}
    };

    /**
     * @brief Connection between ports
     */
    struct Connection {
        ModularComponent* sourceModule;
        int sourcePortIndex;
        ModularComponent* destModule;
        int destPortIndex;
        
        Connection(ModularComponent* src, int srcPort, ModularComponent* dst, int dstPort)
            : sourceModule(src), sourcePortIndex(srcPort), 
              destModule(dst), destPortIndex(dstPort) {}
    };

    /**
     * @brief Parameter definition for module configuration
     */
    struct Parameter {
        std::string name;
        float value;
        float minValue;
        float maxValue;
        float defaultValue;
        std::string unit;   // e.g., "dB", "Hz", "ms", "%"
        
        Parameter(const std::string& n, float def, float min, float max, const std::string& u = "")
            : name(n), value(def), minValue(min), maxValue(max), defaultValue(def), unit(u) {}
    };

protected:
    std::string moduleName;
    std::string moduleType;
    std::vector<Port> inputPorts;
    std::vector<Port> outputPorts;
    std::vector<Parameter> parameters;
    
    // Visual properties for GUI
    sf::Vector2f position;
    sf::Vector2f size;
    sf::Color moduleColor;
    bool selected;
    bool enabled;

public:
    ModularComponent(const std::string& name, const std::string& type)
        : moduleName(name), moduleType(type), 
          position(0, 0), size(120, 80), 
          moduleColor(sf::Color(100, 100, 150)),
          selected(false), enabled(true) {}
    
    virtual ~ModularComponent() = default;

    // ========================= CORE FUNCTIONALITY =========================
    
    /**
     * @brief Process module logic (called every frame/sample)
     * @param deltaTime Time since last process call
     */
    virtual void process(float deltaTime) = 0;
    
    /**
     * @brief Initialize module (called once at startup)
     */
    virtual void initialize() {}
    
    /**
     * @brief Reset module to initial state
     */
    virtual void reset() {}

    // ========================= PORT MANAGEMENT =========================
    
    /**
     * @brief Get all input ports
     */
    const std::vector<Port>& getInputPorts() const { return inputPorts; }
    
    /**
     * @brief Get all output ports
     */
    const std::vector<Port>& getOutputPorts() const { return outputPorts; }
    
    /**
     * @brief Find port by index and type
     */
    const Port* getPort(int index, bool isInput) const {
        const auto& ports = isInput ? inputPorts : outputPorts;
        for (const auto& port : ports) {
            if (port.index == index) return &port;
        }
        return nullptr;
    }

    // ========================= SIGNAL ROUTING =========================
    
    /**
     * @brief Send data to an output port
     * @param portIndex Output port index
     * @param data Data to send
     */
    virtual void sendOutput(int portIndex, const std::vector<float>& data) = 0;
    
    /**
     * @brief Receive data from an input port
     * @param portIndex Input port index
     * @param data Data received
     */
    virtual void receiveInput(int portIndex, const std::vector<float>& data) = 0;

    // ========================= PARAMETER MANAGEMENT =========================
    
    /**
     * @brief Get all parameters
     */
    const std::vector<Parameter>& getParameters() const { return parameters; }
    
    /**
     * @brief Get parameter by name
     */
    Parameter* getParameter(const std::string& name) {
        for (auto& param : parameters) {
            if (param.name == name) return &param;
        }
        return nullptr;
    }
    
    /**
     * @brief Set parameter value
     */
    virtual void setParameter(const std::string& name, float value) {
        Parameter* param = getParameter(name);
        if (param) {
            param->value = std::max(param->minValue, std::min(param->maxValue, value));
        }
    }

    // ========================= GUI PROPERTIES =========================
    
    std::string getName() const { return moduleName; }
    std::string getType() const { return moduleType; }
    
    void setPosition(const sf::Vector2f& pos) { position = pos; }
    sf::Vector2f getPosition() const { return position; }
    
    void setSize(const sf::Vector2f& s) { size = s; }
    sf::Vector2f getSize() const { return size; }
    
    void setColor(const sf::Color& color) { moduleColor = color; }
    sf::Color getColor() const { return moduleColor; }
    
    void setSelected(bool sel) { selected = sel; }
    bool isSelected() const { return selected; }
    
    void setEnabled(bool en) { enabled = en; }
    bool isEnabled() const { return enabled; }
    
    /**
     * @brief Get port position for drawing connections
     * @param portIndex Port index
     * @param isInput true for input port, false for output
     * @return Absolute position of the port on screen
     */
    virtual sf::Vector2f getPortPosition(int portIndex, bool isInput) const {
        const auto& ports = isInput ? inputPorts : outputPorts;
        if (portIndex >= 0 && portIndex < static_cast<int>(ports.size())) {
            float x = isInput ? position.x : position.x + size.x;
            float portSpacing = size.y / (ports.size() + 1);
            float y = position.y + portSpacing * (portIndex + 1);
            return sf::Vector2f(x, y);
        }
        return position;
    }
    
    /**
     * @brief Render module-specific parameter window
     * @param window SFML window to render to
     */
    virtual void renderParameterWindow(sf::RenderWindow& window) = 0;
    
    /**
     * @brief Check if a point is inside the module bounds
     */
    bool contains(const sf::Vector2f& point) const {
        return point.x >= position.x && point.x <= position.x + size.x &&
               point.y >= position.y && point.y <= position.y + size.y;
    }
    
    /**
     * @brief Check if a point is on a port
     * @return Port index if hit, -1 otherwise; sets isInput flag
     */
    int hitTestPort(const sf::Vector2f& point, bool& isInput) const {
        const float portRadius = 8.0f;
        
        // Test input ports
        for (size_t i = 0; i < inputPorts.size(); i++) {
            sf::Vector2f portPos = getPortPosition(i, true);
            float dx = point.x - portPos.x;
            float dy = point.y - portPos.y;
            if (dx*dx + dy*dy <= portRadius*portRadius) {
                isInput = true;
                return i;
            }
        }
        
        // Test output ports
        for (size_t i = 0; i < outputPorts.size(); i++) {
            sf::Vector2f portPos = getPortPosition(i, false);
            float dx = point.x - portPos.x;
            float dy = point.y - portPos.y;
            if (dx*dx + dy*dy <= portRadius*portRadius) {
                isInput = false;
                return i;
            }
        }
        
        return -1;
    }
};
