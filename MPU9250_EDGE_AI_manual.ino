#include "mpu9250.h"
#include "imu_model.h"  // Include your Decision Tree model

/* Mpu9250 object */
bfs::Mpu9250 imu;

/* Decision Tree model */
Eloquent::ML::Port::DecisionTree model;

void setup() {
    /* Serial to display data */
    Serial.begin(115200);
    while (!Serial) {}

    /* Start the I2C bus */
    Wire.begin();
    Wire.setClock(400000);

    /* I2C bus, 0x68 address */
    imu.Config(&Wire, bfs::Mpu9250::I2C_ADDR_PRIM);

    /* Initialize and configure IMU */
    if (!imu.Begin()) {
        Serial.println("Error initializing communication with IMU");
        while (1) {}
    }

    /* Set the sample rate divider */
    if (!imu.ConfigSrd(19)) {
        Serial.println("Error configuring SRD");
        while (1) {}
    }

    // Optionally load your model if necessary
    // model.load("your_model_file_path");  // Uncomment if needed
}

void loop() {
    /* Check if new IMU data is available */
    if (imu.Read() && imu.new_imu_data()) {
        // Read gyroscope data
        float gyroX = imu.gyro_x_radps();
        float gyroY = imu.gyro_y_radps();
        float gyroZ = imu.gyro_z_radps();

        // Prepare input for the decision tree (only gyro data)
        float input[3] = { gyroX, gyroY, gyroZ };

        // Predict direction using the decision tree model
        int prediction = model.predict(input);

        // Print gyroscope data
        Serial.print("Gyro Data: ");
        Serial.print("GyroX: "); Serial.print(gyroX);
        Serial.print("\tGyroY: "); Serial.print(gyroY);
        Serial.print("\tGyroZ: "); Serial.print(gyroZ);

        // Convert prediction to string
        String direction;
        switch (prediction) {
            case 0: direction = "Forward/Idle"; break;
            case 1: direction = "Turning Left"; break;
            case 2: direction = "Turning Right"; break;
            case 3: direction = "Wheelie"; break;
            default: direction = "Unknown"; break;
        }
        
        Serial.print("\tPredicted Direction: "); Serial.println(direction);
        delay(1000); // Adjust delay as needed
    }
}
