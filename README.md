# Rover_MPU9250_EdgeAI_
  - Smart car with ESP32, self-hosted server, MQTT, InfluxDB, Grafana and EdgeAI with MPU9250 (9-axis Motion Processing Unit) sensor.

## Contents

- `MPU9250_EDGE_AI`: Simple model predicition output via manual movement
- `MPU9250_i2c`: Original project from https://github.com/bolderflight/invensense-imu
- `Rover_MPU9250_EdgeAI`: Final project 
- `Rover_MPU_DataLogger`: App that connects to WiFi, creates a UI server for motor control (with keyboard arrow keys for easy control) and displays the MPU9250 data.
- `/data/`:
  - `decision_tree_calc.ipynb`: Generates a Machine Learning prediction model from the `.csv` logs
  - `log.py`: Script that logs the serial output values of the MPU9250 (from columns) into a `.csv` file


https://harrkout.github.io/Rover_MPU9250_EdgeAI_/Koutsourelakis_Charilaos_MTP323_IoT_FinalReport_5Oct24.pdf
