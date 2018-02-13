package com.example.group10.tankdrive;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.content.res.Configuration;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.ImageButton;
import android.widget.SeekBar;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Set;
import java.util.UUID;
import android.content.Intent;

import static java.lang.Math.floor;
import static java.lang.Math.round;
import static java.sql.DriverManager.println;

public class MainActivity extends AppCompatActivity {
    protected int requestCode = 0;
    protected BluetoothAdapter mBluetoothAdapter;
    public static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");
    protected ConnectThread ct = null;
    protected SeekBar speed = null;

    // Listener for the up button
    protected View.OnTouchListener uClick = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                forward(v);
                v.setPressed(true);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                stop(v);
                v.setPressed(false);
            }
            return true;
        }
    };

    // Listener for the down button
    protected View.OnTouchListener dClick = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                back(v);
                v.setPressed(true);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                stop(v);
                v.setPressed(false);
            }
            return true;
        }
    };

    // Listener for the left button
    protected View.OnTouchListener lClick = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                left(v);
                v.setPressed(true);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                stop(v);
                v.setPressed(false);
            }
            return true;
        }
    };

    // Listener for the right button
    protected View.OnTouchListener rClick = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                right(v);
                v.setPressed(true);
            } else if (event.getAction() == MotionEvent.ACTION_UP) {
                stop(v);
                v.setPressed(false);
            }
            return true;
        }
    };

    // Listener for the power button
    protected View.OnTouchListener pClick = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                if (v.isPressed()) {
                    power_off(v);
                } else {
                    ct.run();
                }
                v.setPressed(!v.isPressed());
            }
            return true;
        }
    };

    // Listener for the self-drive button
    protected View.OnTouchListener sdClick = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                if (v.isPressed()) {
                    sd_off(v);
                } else {
                    sd_start(v);
                }
                v.setPressed(!v.isPressed());
            }
            return true;
        }
    };

    // Listener for the tilt drive button
    protected View.OnTouchListener tClick = new View.OnTouchListener() {
        @Override
        public boolean onTouch(View v, MotionEvent event) {
            if (event.getAction() == MotionEvent.ACTION_DOWN) {
                tilt_on(v);
                v.setPressed(!v.isPressed());
            }
            return true;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Log.d("CREATE", "Creating");

        // Get all of the views we'll use
        ImageButton up = (ImageButton) findViewById(R.id.up);
        ImageButton down = (ImageButton) findViewById(R.id.down);
        ImageButton left = (ImageButton) findViewById(R.id.left);
        ImageButton right = (ImageButton) findViewById(R.id.right);
        ImageButton power = (ImageButton) findViewById(R.id.power);
        ImageButton sd = (ImageButton) findViewById(R.id.self_drive);
        ImageButton tilt = (ImageButton) findViewById(R.id.tilt);

        // Set the views' listeners
        up.setOnTouchListener(uClick);
        down.setOnTouchListener(dClick);
        left.setOnTouchListener(lClick);
        right.setOnTouchListener(rClick);
        power.setOnTouchListener(pClick);
        sd.setOnTouchListener(sdClick);
        tilt.setOnTouchListener(tClick);

        speed = (SeekBar) findViewById(R.id.speed);

        // Set up bluetooth
        BluetoothDevice tank = null;

        if (ct == null) {
            mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
            // Crash if the phone doesn't have a bluetooth adapter
            if (mBluetoothAdapter == null) {
                return;
            }

            // Prompt user to enable bluetooth
            if (!mBluetoothAdapter.isEnabled()) {
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, requestCode);
            }

            // Find the tank in the paired devices
            Set<BluetoothDevice> pairedDevices = mBluetoothAdapter.getBondedDevices();
            // If there are paired devices
            if (pairedDevices.size() > 0) {
                // Loop through paired devices
                for (BluetoothDevice device : pairedDevices) {
                    tank = device;
                }
            }

            // Connect to the tank
            if (tank != null) {
                ct = ConnectThread.create(tank);
            }
        }
    }

    // Drive forward
    public void forward(View view) {
        Log.d("DRIVE", "Forward");
        byte duty = (byte) floor(((float) speed.getProgress() / speed.getMax()) * 126.0);
        byte[] bytes = new byte[4];
        bytes[0] = (byte) (0x80 | duty);
        bytes[1] = (byte) (0x80 | duty);
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        ct.write(bytes);
    }

    // Turn left
    public void left(View view) {
        Log.d("DRIVE", "Left");
        byte duty = (byte) floor(((float) speed.getProgress() / speed.getMax()) * 126.0);
        byte[] bytes = new byte[4];
        bytes[0] = duty;
        bytes[1] = (byte) (0x80 | duty);
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        ct.write(bytes);
    }

    // Turn right
    public void right(View view) {
        Log.d("DRIVE", "Right");
        byte duty = (byte) floor(((float) speed.getProgress() / speed.getMax()) * 126.0);
        byte[] bytes = new byte[4];
        bytes[0] = (byte) (0x80 | duty);
        bytes[1] = duty;
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        ct.write(bytes);
    }

    // Back up
    public void back(View view) {
        Log.d("DRIVE", "Backward");
        byte duty = (byte) floor(((float) speed.getProgress() / speed.getMax()) * 126.0);
        byte[] bytes = new byte[4];
        bytes[0] = duty;
        bytes[1] = duty;
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        ct.write(bytes);
    }

    // Stop
    public void stop(View view) {
        Log.d("CREATE", "Stop");
        byte[] bytes = new byte[4];
        bytes[0] = (byte) 0x00;
        bytes[1] = (byte) 0x00;
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        ct.write(bytes);
    }

    // Power off
    public void power_off(View view) {
        byte[] bytes = new byte[4];
        bytes[0] = (byte) 0xFF;
        bytes[1] = (byte) 0x00;
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        ct.write(bytes);
    }

    // Start self-driving
    public void sd_start(View view) {
        byte[] bytes = new byte[4];
        bytes[0] = (byte) 0xFF;
        bytes[1] = (byte) 0x01;
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        ct.write(bytes);
    }

    // Turn off self-driving
    public void sd_off(View view) {
        byte[] bytes = new byte[4];
        bytes[0] = (byte) 0xFF;
        bytes[1] = (byte) 0x02;
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        ct.write(bytes);
    }

    // Go into tilt mode
    public void tilt_on(View view) {
        Intent intent = new Intent(this, TiltMode.class);
        startActivity(intent);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        this.requestCode = requestCode;
    }

}