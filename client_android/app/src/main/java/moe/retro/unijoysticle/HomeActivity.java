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

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.SeekBar;
import android.widget.Spinner;
import android.widget.Switch;
import android.widget.TextView;

import java.util.concurrent.Executors;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.TimeUnit;

import moe.retro.unijoysticle.unijosyticle.R;

public class HomeActivity extends BaseActivity implements
        SeekBar.OnSeekBarChangeListener, View.OnClickListener, Spinner.OnItemSelectedListener, CompoundButton.OnCheckedChangeListener {

    private static final String TAG = HomeActivity.class.getSimpleName();

    private TextView mValueText;
    private Boolean mFirstTime = true;
    private ScheduledExecutorService mScheduleTaskExecutor;
    private boolean mClearJoyState = false;

    private enum HomeCommands {
        NOTHING((byte)0),
        SONG_0((byte)1),
        SONG_1((byte)2),
        SONG_2((byte)3),
        SONG_3((byte)4),
        SONG_4((byte)5),
        SONG_5((byte)6),
        SONG_6((byte)7),
        SONG_7((byte)8),
        SONG_8((byte)9),

        RESERVED_0((byte)10),

        SONG_STOP((byte)11),
        SONG_PLAY((byte)12),
        SONG_PAUSE((byte)13),
        SONG_RESUME((byte)14),
        SONG_NEXT((byte)15),
        SONG_PREV((byte)16),
        DIMMER_0((byte)17),
        DIMMER_25((byte)18),
        DIMMER_50((byte)19),
        DIMMER_75((byte)20),
        DIMMER_100((byte)21),
        ALARM_OFF((byte)22),
        ALARM_ON((byte)23),

        RESERVED_2((byte)24),
        RESERVED_3((byte)25),
        RESERVED_4((byte)26),
        RESERVED_5((byte)27),
        RESERVED_6((byte)28),
        RESERVED_7((byte)29),
        RESERVED_8((byte)30),
        RESERVED_9((byte)31);

        private final byte value;
        HomeCommands(byte v) {
            this.value = v;
        }
        public byte getValue() {
            return this.value;
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setTitle(getString(R.string.action_bar_title_home));

        setContentView(R.layout.activity_home);

        // dimmer
        mValueText = (TextView) findViewById(R.id.textView_dimmer);
        SeekBar seekBar = (SeekBar) findViewById(R.id.seekBar_dimmer);
        seekBar.setOnSeekBarChangeListener(this);

        // Songs
        Button buttonPlay = (Button) findViewById(R.id.button_play);
        Button buttonStop = (Button) findViewById(R.id.button_stop);
        buttonPlay.setOnClickListener(this);
        buttonStop.setOnClickListener(this);

        Spinner spinnerSong = (Spinner) findViewById(R.id.spinner_songs);
        spinnerSong.setOnItemSelectedListener(this);
        // Create an ArrayAdapter using the string array and a default spinner layout
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
                R.array.songs, android.R.layout.simple_spinner_item);
        // Specify the layout to use when the list of choices appears
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        // Apply the adapter to the spinner
        spinnerSong.setAdapter(adapter);

        // alarm
        Switch alarmSwitch = (Switch) findViewById(R.id.switch_alarm);
        alarmSwitch.setOnCheckedChangeListener(this);


        mProtoVersion = 2;
        mProtoHeader.joyState1 = 0;     // reset joystick values
        mProtoHeader.joyState2 = 0;
        mProtoHeader.joyControl = 3;    // both joyticks (1 and 2)


        // schedule a handler every 60 per second
        mScheduleTaskExecutor = Executors.newScheduledThreadPool(2);
        mScheduleTaskExecutor.scheduleAtFixedRate(new Runnable() {
            public void run() {
                try {
                    if (mClearJoyState) {
                        Thread.sleep(160);
                        mProtoHeader.joyState2 = 0;
                        sendJoyState();
                        mClearJoyState = false;
                    }

                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }, 0, 16, TimeUnit.MILLISECONDS);       // ~60Hz

    }

    //
    // Dimmer
    //

    // OnSeekBarChangeListener methods :
    @Override
    public void onProgressChanged(SeekBar seek, int value, boolean fromTouch)
    {
        String values[] = {"Off", "25%", "50%", "75%", "100%"};
        HomeCommands[] commands = {HomeCommands.DIMMER_0,
                HomeCommands.DIMMER_25,
                HomeCommands.DIMMER_50,
                HomeCommands.DIMMER_75,
                HomeCommands.DIMMER_100};
        mValueText.setText(values[value]);

        mProtoHeader.joyState2 = commands[value].getValue();
        sendJoyState();
        mClearJoyState = true;
    }
    @Override
    public void onStartTrackingTouch(SeekBar seek) {}
    @Override
    public void onStopTrackingTouch(SeekBar seek) {}


    //
    // Songs
    //
    @Override
    public void onClick(View v) {
        switch(v.getId()) {
            case R.id.button_play:
                mProtoHeader.joyState2 = HomeCommands.SONG_PLAY.getValue();
                break;
            case R.id.button_stop:
                mProtoHeader.joyState2 = HomeCommands.SONG_STOP.getValue();
                break;
        }
        sendJoyState();
        mClearJoyState = true;
    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
        if (b) {
            mProtoHeader.joyState2 = HomeCommands.ALARM_ON.getValue();
        } else {
            mProtoHeader.joyState2 = HomeCommands.ALARM_OFF.getValue();
        }
        sendJoyState();
        mClearJoyState = true;
    }


    @Override
    public void onItemSelected(AdapterView<?> adapterView, View v, int i, long l) {

        // ignore first time since this will be called right after selecting the HomeActivity
        if (!mFirstTime) {
            mProtoHeader.joyState2 = (byte) ((byte) i + 1);
            sendJoyState();
            mClearJoyState = true;
        }
        mFirstTime = false;
    }

    @Override
    public void onNothingSelected(AdapterView<?> adapterView) {
    }
}
