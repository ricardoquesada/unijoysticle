/* The following code was written by Matthew Wiggins
 * Modified to support floats, min and default values by Ricardo Quesada
 *
 * Released under the APACHE 2.0 license
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Improvements :
 * - save the value on positive button click, not on seekbar change
 * - handle @string/... values in xml file
 */

package moe.retro.unijoysticle;

import android.app.AlertDialog;
import android.content.Context;
import android.os.Bundle;
import android.preference.DialogPreference;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;


public class SeekBarPreference extends DialogPreference implements SeekBar.OnSeekBarChangeListener, OnClickListener
{
    // ------------------------------------------------------------------------------------------
    // Private attributes :
    private static final String androidns="http://schemas.android.com/apk/res/android";

    private SeekBar mSeekBar;
    private TextView mSplashText,mValueText;
    private Context mContext;

    private String mDialogMessage;
    private int mValue = 0;             // SeekBar value: from 0-mValueMax (int)
    private int mMax = 100;               // SeekBar max value (int)
    private float mUserDefault, mUserMin, mUserMax; // user values (float)
    private static final String TAG = SeekBarPreference.class.getSimpleName();


    // ------------------------------------------------------------------------------------------



    // ------------------------------------------------------------------------------------------
    // Constructor :
    public SeekBarPreference(Context context, AttributeSet attrs) {

        super(context,attrs);
        mContext = context;

        // Get string value for dialogMessage :
        int mDialogMessageId = attrs.getAttributeResourceValue(androidns, "dialogMessage", 0);
        if(mDialogMessageId == 0) mDialogMessage = attrs.getAttributeValue(androidns, "dialogMessage");
        else mDialogMessage = mContext.getString(mDialogMessageId);

        // Some defaults. use setMinMaxDefault() to set the values
        mUserMax = attrs.getAttributeIntValue(androidns, "max", 0) / 10.0f;
        mUserDefault = attrs.getAttributeIntValue(androidns, "defaultValue", 0) / 10.0f;
        String minVal = attrs.getAttributeValue(androidns, "text");
        mUserMin = Float.valueOf(minVal) / 10.0f;

        mMax = (int) (mUserMax-mUserMin) * 10;

        setSummary(String.valueOf(mUserDefault));
    }
    // ------------------------------------------------------------------------------------------



    // ------------------------------------------------------------------------------------------
    // DialogPreference methods :
    @Override
    protected View onCreateDialogView() {

        LinearLayout.LayoutParams params;
        LinearLayout layout = new LinearLayout(mContext);
        layout.setOrientation(LinearLayout.VERTICAL);
        layout.setPadding(6,6,6,6);

        mSplashText = new TextView(mContext);
        mSplashText.setPadding(30, 10, 30, 10);
        if (mDialogMessage != null)
            mSplashText.setText(mDialogMessage);
        layout.addView(mSplashText);

        mValueText = new TextView(mContext);
        mValueText.setGravity(Gravity.CENTER_HORIZONTAL);
        mValueText.setTextSize(32);
        params = new LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT);
        layout.addView(mValueText, params);

        mSeekBar = new SeekBar(mContext);
        mSeekBar.setOnSeekBarChangeListener(this);
        layout.addView(mSeekBar, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

        if (shouldPersist()) {
            float persistedFloat = getPersistedFloat(mUserDefault);
            mValue = (int) (((persistedFloat-mUserMin) / (mUserMax - mUserMin)) * mMax);
            setSummary(String.valueOf(persistedFloat));
        }
        mSeekBar.setMax(mMax);
        mSeekBar.setKeyProgressIncrement(1);
        mSeekBar.setProgress(mValue);

        return layout;
    }

    @Override
    protected void onBindDialogView(View v) {
        super.onBindDialogView(v);
        mSeekBar.setMax(mMax);
        mSeekBar.setProgress(mValue);
    }

    @Override
    protected void onSetInitialValue(boolean restore, Object defaultValue)
    {
        super.onSetInitialValue(restore, defaultValue);
        if (restore) {
            float persistedFloat = getPersistedFloat(mUserDefault);
            int def = (int) ((persistedFloat-mUserMin) / (mUserMax-mUserMin) * mMax);
            mValue = shouldPersist() ? def : 0;
            setSummary(String.valueOf(persistedFloat));
        } else {
            mValue = (Integer) defaultValue;
        }
    }

    // ------------------------------------------------------------------------------------------
    // OnSeekBarChangeListener methods :
    @Override
    public void onProgressChanged(SeekBar seek, int value, boolean fromTouch)
    {
        float f = ((mUserMax-mUserMin) * value) / mMax + mUserMin;
        String t = String.valueOf(f);
        mValueText.setText(t);
    }

    @Override
    public void onStartTrackingTouch(SeekBar seek) {}
    @Override
    public void onStopTrackingTouch(SeekBar seek) {}

    // ------------------------------------------------------------------------------------------
    // Set the positive button listener and onClick action :
    @Override
    public void showDialog(Bundle state) {

        super.showDialog(state);

        Button positiveButton = ((AlertDialog) getDialog()).getButton(AlertDialog.BUTTON_POSITIVE);
        positiveButton.setOnClickListener(this);
    }

    @Override
    public void onClick(View v) {

        if (shouldPersist()) {
            mValue = mSeekBar.getProgress();
            float toStore = ((mUserMax-mUserMin) * mValue) / mMax + mUserMin;
            persistFloat(toStore);
            callChangeListener(toStore);
            setSummary(String.valueOf(toStore));
        }

        ((AlertDialog) getDialog()).dismiss();
    }
    // ------------------------------------------------------------------------------------------
}