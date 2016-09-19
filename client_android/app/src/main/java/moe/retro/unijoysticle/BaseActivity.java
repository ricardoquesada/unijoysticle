/*
 * Copyright (C) 2016 Ricardo Quesada
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package moe.retro.unijoysticle;

import android.annotation.SuppressLint;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.util.Log;
import android.os.AsyncTask;
import android.view.WindowManager;


import java.io.IOException;
import java.net.InetAddress;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.net.DatagramPacket;
import java.net.DatagramSocket;

import moe.retro.unijoysticle.unijosyticle.R;

@SuppressLint("Registered")
public class BaseActivity extends AppCompatActivity {

    private static final String TAG = BaseActivity.class.getSimpleName();

    public enum JoyBits {
        Up((byte)0b00000001),
        Down((byte)0b00000010),
        Left((byte)0b00000100),
        Right((byte)0b00001000),
        Fire((byte)0b00010000),
        DPad((byte)0b00001111),
        All((byte)0b00011111);

        private final byte value;

        JoyBits(byte joyBitsCode) {
            this.value = joyBitsCode;
        }

        public byte getValue() {
            return this.value;
        }
    }

    public class UDPConnection {
        private AsyncTask<Void, Void, Boolean> async_client;
        final private int SERVER_PORT = 6464;
        private InetAddress mServerAddress;
        private DatagramSocket mSocket;

        UDPConnection(final String serverAddress) {

            async_client = new AsyncTask<Void, Void, Boolean>() {
                @Override
                protected Boolean doInBackground(Void... params) {
                    try {
                        mServerAddress = InetAddress.getByName(serverAddress);
                        mSocket = new DatagramSocket();
                    } catch (SocketException | UnknownHostException e) {
                        e.printStackTrace();
                        return false;
                    }
                    return true;
                }
                @Override
                protected void onPostExecute(Boolean result) {
                    super.onPostExecute(result);

                    if (!result) {
                        AlertDialog.Builder builder = new AlertDialog.Builder(BaseActivity.this);
                        builder.setMessage(R.string.error_dialog_message)
                                .setTitle(R.string.error_dialog_title);
                        builder.setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                finish();
                            }
                        });
                        AlertDialog dialog = builder.create();
                        dialog.show();
                    }
                }
            };
            async_client.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        }

        public void sendState(final byte joyControl, final byte joyState) {
            async_client = new AsyncTask<Void, Void, Boolean>()
            {
                @Override
                protected Boolean doInBackground(Void... params) {
                    byte data[] = new byte[]{joyControl, joyState};
                    DatagramPacket dp = new DatagramPacket(data, data.length, mServerAddress, SERVER_PORT);
                    try {
                        mSocket.send(dp);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    return true;
                }
                protected void onPostExecute(Boolean result) {
                    super.onPostExecute(result);
                }
            };
            async_client.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        }

        public void sendState2(final byte[] data) {
            async_client = new AsyncTask<Void, Void, Boolean>()
            {
                @Override
                protected Boolean doInBackground(Void... params) {
                    DatagramPacket dp = new DatagramPacket(data, data.length, mServerAddress, SERVER_PORT);
                    try {
                        mSocket.send(dp);
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                    return true;
                }
                protected void onPostExecute(Boolean result) {
                    super.onPostExecute(result);
                }
            };
            async_client.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
        }
    }

    private UDPConnection mNet;

    public byte mProtoVersion = 1;       // by default, use v1

    // Protocol v1
    public byte mJoyState = 0;
    public byte mJoyControl = 1;      // joystick 0 or 1

    // Protocol v2
    class ProtoHeader {
        final byte version = 2;
        byte joyControl = 0b00000011;   // joy 1 and 2 enabled
        byte joyState1 = 0;
        byte joyState2 = 0;
    }

    public ProtoHeader mProtoHeader;
//    private ScheduledExecutorService mScheduleTaskExecutor;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //Remove notification bar
        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(this);
        String serverAddress = preferences.getString(getString(R.string.key_serverAddress), "192.168.4.1");

        // udp connection
        mNet = new UDPConnection(serverAddress);

        // get JoyValue
        Bundle b = getIntent().getExtras();
        mJoyControl = b.getByte("joyPort");
        Log.d(TAG, "Joy Selected:" + mJoyControl);

        // schedule a handler every 60 per second
//        mScheduleTaskExecutor = Executors.newScheduledThreadPool(2);
//        mScheduleTaskExecutor.scheduleAtFixedRate(new Runnable() {
//            public void run() {
//                sendJoyState();
//            }
//        }, 0, 16, TimeUnit.MILLISECONDS);       // ~60Hz

        mProtoHeader = new ProtoHeader();
    }

    @Override
    protected void onResume() {
        super.onResume();
        // The activity has become visible (it is now "resumed").
    }

    @Override
    protected void onPause() {
        super.onPause();
        // Another activity is taking focus (this activity is about to be "paused").
    }

    @Override
    protected void onDestroy() {
//        mScheduleTaskExecutor.shutdownNow();
        if (mProtoVersion == 1) {
            mJoyState = 0;
        } else {
            mProtoHeader.joyState2 = 0;
            mProtoHeader.joyState1 = 0;
        }
        sendJoyState();
        super.onDestroy();
        // The activity is about to be destroyed.
    }

    public void sendJoyState() {
        // send it twice, in case the UDP packet is lost
        for (int i=0; i<2; i++) {
            if (mProtoVersion == 1) {
                mNet.sendState(mJoyControl, mJoyState);
            } else {
                byte data[] = new byte[]{mProtoHeader.version, mProtoHeader.joyControl, mProtoHeader.joyState1, mProtoHeader.joyState2};
                mNet.sendState2(data);
            }
        }
    }
}
