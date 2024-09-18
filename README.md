## **Summary of Code**

This IoT project uses an ESP32 microcontroller with Blynk, RFID, servomotors, IR sensors, LEDs, and a buzzer to manage access control for two zones.<br>
The system reads RFID tags to determine if a user can enter a zone based on its capacity. <br>
The code initializes the necessary components, connects to WiFi, and sets up tasks to handle RFID reading, gate management, sensor monitoring, and updating the server with zone counts.<br>

## **Key Functionalities**

**WiFi and Blynk Initialization**: Connects to WiFi and initializes Blynk for remote monitoring.<br>
**RFID Reader**: Reads RFID tags and manages access.<br>
**Servo Motors**: Controls the gates for the main entrance and two zones.<br>
**LEDs and Buzzer**: Provides visual and auditory feedback based on gate status and RFID readings.<br>
**IR Sensors**: Monitors exit sensors to update zone counts.<br>
