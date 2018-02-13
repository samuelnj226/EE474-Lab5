package com.example.group10.tankdrive;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.util.Log;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * Created by reill on 12/8/2016.
 */

public class ConnectThread extends Thread {
    private static ConnectThread ct = null;
    private final BluetoothSocket mmSocket;
    private final BluetoothDevice mmDevice;

    private InputStream mmInStream;
    private OutputStream mmOutStream;

    // Singleton; maintain only one connection
    public static ConnectThread create() {
        if (ct == null) {
            return null;
        }
        return ct;
    }

    public static ConnectThread create(BluetoothDevice device) {
        if (ct == null) {
            ct = new ConnectThread(device);
        }
        return ct;
    }

    // Constructor; only for use by create()
    private ConnectThread(BluetoothDevice device) {
        // Use a temporary object that is later assigned to mmSocket,
        // because mmSocket is final
        BluetoothSocket tmp = null;
        mmDevice = device;

        // Get a BluetoothSocket to connect with the given BluetoothDevice
        try {
            Log.d("CREATE", "Opening socket");
            // MY_UUID is the app's UUID string, also used by the server code
            tmp = device.createRfcommSocketToServiceRecord(MainActivity.MY_UUID);
        } catch (IOException e) { }
        mmSocket = tmp;

        int attempts = 0;

        while (true) {
            try {
                Log.d("CREATE", "Connecting to the device through socket");
                // Connect the device through the socket. This will block
                // until it succeeds or throws an exception
                mmSocket.connect();
                break;
            } catch (IOException connectException) {
                attempts++;
                if (attempts < 3) {
                    continue;
                }
                // Unable to connect; close the socket and get out
                try {
                    Log.d("CREATE", "Closing socket because we're unable to connect");
                    mmSocket.close();
                } catch (IOException closeException) {
                }
                return;
            }
        }

        InputStream tmpIn = null;
        OutputStream tmpOut = null;

        // Get the input and output streams, using temp objects because
        // member streams are final
        try {
            while (tmpIn == null || tmpOut == null) {
                tmpIn = mmSocket.getInputStream();
                tmpOut = mmSocket.getOutputStream();
            }
        } catch (IOException e) { Log.d("CREATE", "Exception"); }

        Log.d("CREATE", "Got here");

        mmInStream = tmpIn;
        mmOutStream = tmpOut;
    }

    // Start running the tank
    public void run() {
        Log.d("CREATE", "Run");
        byte[] bytes = new byte[4];
        bytes[0] = (byte) 0xFF;
        bytes[1] = (byte) 0xFF;
        bytes[2] = (byte) 0x0D;
        bytes[3] = (byte) 0x0A;

        write(bytes);
    }
    /* Call this from the main activity to send data to the remote device */
    public void write(byte[] bytes) {
        Log.d("CREATE", "Write");
        try {
            mmOutStream.write(bytes);
        } catch (IOException e) { }
    }

    /** Will cancel an in-progress connection, and close the socket */
    public void cancel() {
        try {
            Log.d("CREATE", "Cancelled");
            byte[] bytes = new byte[4];
            bytes[0] = (byte) 0xFF;
            bytes[1] = (byte) 0x00;
            bytes[2] = (byte) 0x0D;
            bytes[3] = (byte) 0x0A;

            write(bytes);

            mmSocket.close();
        } catch (IOException e) { }
    }
}

