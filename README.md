Smart Waste Segregation and Recycling System

Project Overview

Our project is an AI-based Smart Waste Segregation and Recycling System designed to automatically identify and segregate different types of waste into separate compartments.

The system uses an AI camera for waste classification, Raspberry Pi for AI processing, and ESP32 for controlling sensors, conveyor motor, servo motors and other hardware components.

How the System Works

1. Waste is placed into the input section.
2. The camera captures the waste image.
3. The AI model identifies the waste category.
4. The waste type is sent to the ESP32 control system.
5. The conveyor moves the waste forward.
6. The corresponding servo motor operates.
7. The waste is directed into the correct compartment.
8. Ultrasonic sensors monitor bin filling level.
9. A load cell measures the waste weight.
10. LCD displays system information.
11. MQTT can be used to send waste data to the cloud.

Waste Categories

- Plastic
- Paper
- Metal
- Glass
- Wet Waste
- Sanitary Waste

Main Hardware

- ESP32
- Raspberry Pi
- AI Camera
- Conveyor Belt
- DC Gear Motor
- Servo Motors
- Ultrasonic Sensor
- Load Cell
- HX711 Module
- LCD Display
- Motor Driver

Software and Technologies

- Arduino IDE
- C/C++
- Python
- Computer Vision
- Machine Learning / AI
- MQTT
- GitHub

Role of ESP32

The ESP32 acts as the main hardware control unit. It reads sensor data, controls the conveyor motor and servo motors, displays information on the LCD and can communicate waste data through Wi-Fi and MQTT.

Role of AI

The AI camera and Raspberry Pi are responsible for identifying the type of waste. The classification result is then used by the ESP32 to control the appropriate segregation mechanism.

Project Goal

The main goal is to reduce manual waste segregation, improve waste management efficiency and support proper recycling through automated waste classification and segregation.

Future Scope

The system can be extended with a municipal monitoring dashboard, QR-based user identification, reward points, cloud data storage, solar power integration and real-time waste monitoring.
