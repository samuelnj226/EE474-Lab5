package com.example.group10.tankdrive;

import android.content.Context;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class TiltMode extends AppCompatActivity implements SensorEventListener {
    private SensorManager mSensorManager;
    private Sensor mSGravity;
    private Sensor mSGeomagnetic;
    private float[] mGravity;
    private float[] mGeomagnetic;
    private float azimut;
    private float pitch;
    private float roll;

    protected ConnectThread ct;


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tilt_mode);
        ct = ConnectThread.create();
        // Get the sensor manager
        mSensorManager = (SensorManager) this.getSystemService(SENSOR_SERVICE);
        // Get the sensors
        mSGravity = mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER);

        mSGeomagnetic = mSensorManager.getDefaultSensor(Sensor.TYPE_MAGNETIC_FIELD);
        if (mSGeomagnetic == null) {
            Log.d("CREATE", "mSGeomagnetic sensor not available");
        } else {
            Log.d("CREATE", "mSGeomagnetic sensor available");
        }

        Log.d("CREATE", "" + mSGeomagnetic.getType());

        mSensorManager.registerListener(this, mSGravity, SensorManager.SENSOR_DELAY_NORMAL);
        mSensorManager.registerListener(this, mSGeomagnetic, SensorManager.SENSOR_DELAY_NORMAL);
    }

    // Do nothing because we don't care; only included to fulfill the abstract class
    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {
    }

    // Send drive data based on rotation to the tank
    // Some of code retrieved from:
    // http://stackoverflow.com/questions/27106607/get-rotation-and-display-in-degrees
    public void onSensorChanged(SensorEvent event) {
        Log.d("SENSOR", "Sensor changed: " + event.sensor.getType());
        // Update the accelerometer data
        if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
            mGravity = event.values;
        if (event.sensor.getType() == Sensor.TYPE_MAGNETIC_FIELD)
            mGeomagnetic = event.values;
        if (mGravity != null && mGeomagnetic != null) {
            float R[] = new float[9];
            float I[] = new float[9];
            boolean success = SensorManager.getRotationMatrix(R, I, mGravity, mGeomagnetic);
            if (success) {
                // Get the orientation based off of sensor data
                float orientation[] = new float[3];
                SensorManager.getOrientation(R, orientation);
                azimut = orientation[0]; // orientation contains: azimut, pitch and roll
                pitch = orientation[1];
                roll = orientation[2];
            }

            double angle = Math.toDegrees(pitch);
            Log.d("SENSOR_DATA", "Angle: " + angle);

            // Turn left
            if (angle > 25 && angle < 90) {
                byte[] bytes = new byte[4];
                bytes[0] = (byte) 0x78;
                bytes[1] = (byte) 0xF8;
                bytes[2] = (byte) 0x0D;
                bytes[3] = (byte) 0x0A;

                ct.write(bytes);
                //Turn right
            } else if (angle < -25) {
                byte[] bytes = new byte[4];
                bytes[0] = (byte) 0xF8;
                bytes[1] = (byte) 0x78;
                bytes[2] = (byte) 0x0D;
                bytes[3] = (byte) 0x0A;

                ct.write(bytes);
                //Stop
            } else {
                byte[] bytes = new byte[4];
                bytes[0] = (byte) 0x00;
                bytes[1] = (byte) 0x00;
                bytes[2] = (byte) 0x0D;
                bytes[3] = (byte) 0x0A;

                ct.write(bytes);
            }
        }
    }
}
