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
import android.widget.SeekBar;
import android.widget.TextView;

import moe.retro.unijoysticle.unijosyticle.R;

public class LinearActivity extends BaseActivity implements SeekBar.OnSeekBarChangeListener {

    private TextView mValueText;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setTitle(getString(R.string.action_bar_title_linear));

        setContentView(R.layout.activity_linear);

        mValueText = (TextView) findViewById(R.id.textViewLinear);
        SeekBar seekBar = (SeekBar) findViewById(R.id.seekBarLinear);
        seekBar.setOnSeekBarChangeListener(this);

    }

    // OnSeekBarChangeListener methods :
    @Override
    public void onProgressChanged(SeekBar seek, int value, boolean fromTouch)
    {
        String t = String.valueOf(value);
        mValueText.setText(t);
        mJoyState = (byte) value;
        sendJoyState();
    }
    @Override
    public void onStartTrackingTouch(SeekBar seek) {}
    @Override
    public void onStopTrackingTouch(SeekBar seek) {}

}
