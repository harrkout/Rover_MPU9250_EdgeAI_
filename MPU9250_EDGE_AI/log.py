import serial
import csv

# Open serial connection to ESP32 (adjust /dev/ttyUSB1 to match your port)
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

                # Ensure there are 6 values (3 accel, 3 gyro)
                if len(data) == 6:
                    # Extract gyro values
                    gyro_data = data[3:6]  # Get gyro_x, gyro_y, gyro_z
                    writer.writerow(gyro_data)  # Write only gyro data to the CSV
                    print(f"Logged Gyro Data: {gyro_data}")  # Optional: print parsed data to monitor
                else:
                    print("Unexpected data format")
    
    except KeyboardInterrupt:
        print("\nLogging stopped by user.")

    except Exception as e:
        print(f"Error during logging: {e}")

# Close the serial connection when done
    ser.close()
    print("Serial connection closed.")

