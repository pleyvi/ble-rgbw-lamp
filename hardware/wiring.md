### Hardware Topology & Wiring

```mermaid
graph LR
    subgraph Power Supplies
        PS12[12V Power Supply]
        PS5[5V USB Power]
    end

    subgraph Microcontroller
        ESP[ESP32-C6-Zero]
    end

    subgraph HW-532 MOSFET Switches
        M_R[MOSFET: Red]
        M_G[MOSFET: Green]
        M_B[MOSFET: Blue]
        M_W[MOSFET: White]
    end

    subgraph Load
        LED[12V RGBW LED Strip]
    end

    %% Power to Logic
    PS5 -- "5V & GND" --> ESP

    %% Logic PWM Signals (3.3V)
    ESP -- "GPIO 1 (PWM)" --> M_R
    ESP -- "GPIO 2 (PWM)" --> M_G
    ESP -- "GPIO 3 (PWM)" --> M_B
    ESP -- "GPIO 4 (PWM)" --> M_W
    
    %% CRITICAL: Shared Logic Ground
    ESP -- "Logic GND" --> M_R
    ESP -- "Logic GND" --> M_G
    ESP -- "Logic GND" --> M_B
    ESP -- "Logic GND" --> M_W

    %% High Power Routing (12V)
    PS12 -- "12V V+ (Common Anode)" --> LED
    PS12 -- "12V GND (VIN-)" --> M_R
    PS12 -- "12V GND (VIN-)" --> M_G
    PS12 -- "12V GND (VIN-)" --> M_B
    PS12 -- "12V GND (VIN-)" --> M_W

    %% Switched Grounds to LEDs
    M_R -- "OUT- (Red Return)" --> LED
    M_G -- "OUT- (Green Return)" --> LED
    M_B -- "OUT- (Blue Return)" --> LED
    M_W -- "OUT- (White Return)" --> LED
