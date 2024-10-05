# author: koutsourelakis

import serial
import csv

# Open serial connection to ESP32
# Linux: ser = serial.Serial('/dev/ttyUSB*', 115200, timeout=2)
# Windows: ser = serial.Serial('COM*', 115200, timeout=2)
# Note: change the asterisk (*) with your port number
try:
    ser = serial.Serial('/dev/ttyUSB2', 115200, timeout=2)  # Add timeout to avoid infinite blocking
except serial.SerialException as e:
    print(f"Error opening serial port: {e}")
    exit()

# Open CSV file for logging gyro data
with open('imu_gyro_data.csv', mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['gyro_x', 'gyro_y', 'gyro_z'])  # CSV header for gyro data

    print("Logging started. Press Ctrl+C to stop.")

    try:
        while True:
            # Read line from serial and decode
            line = ser.readline().decode('utf-8').strip()

            if line:
                print(f"Raw Line: {line}")  # Print raw data for debugging

                # Split the line by spaces
                data = line.split()  # Use default whitespace split

                # Ensure there are 12 values in the output while running the app "Rover_IMU_DataLogger"
                '''
                 ------------------------------
                | No |   Data type  | Function |
                |:--:|:------------:|:--------:|
                |  0 | new_imu_data |          |
                |  1 | new_mag_data |          |
                |  2 | accel_x_mps2 |          |
                |  3 | accel_y_mps2 |          |
                |  4 | accel_z_mps2 |          |
                |  5 | gyro_x_radps |   USED   |
                |  6 | gyro_y_radps |   USED   |
                |  7 | gyro_z_radps |   USED   |
                |  8 |   mag_x_ut   |          |
                |  9 |   mag_y_ut   |          |
                | 10 |   mag_z_ut   |          |
                | 11 |  die_temp_c  |          |
                 ------------------------------
                '''
                if len(data) == 12:
                    # Extract gyro values by selecting the columns we want to log
                    gyro_data = data[5:8]  # Get gyro_x, gyro_y, gyro_z
                    writer.writerow(gyro_data)  # Write only gyro data to the CSV
                    print(f"Logged Gyro Data: {gyro_data}")  # print parsed data to monitor
                else:
                    print("Unexpected data format")
    
    except KeyboardInterrupt:
        print("\nLogging stopped by user.")

    except Exception as e:
        print(f"Error during logging: {e}")

    ser.close()
    print("Serial connection closed.")

