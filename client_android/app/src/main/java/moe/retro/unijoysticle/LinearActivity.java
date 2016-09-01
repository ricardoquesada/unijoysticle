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
