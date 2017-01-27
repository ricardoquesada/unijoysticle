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

import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.support.design.widget.FloatingActionButton;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.util.Log;
import android.widget.Button;
import android.widget.RadioGroup;

import com.google.firebase.analytics.FirebaseAnalytics;

import moe.retro.unijoysticle.unijosyticle.R;


public class MainActivity extends AppCompatActivity {

    private static final String TAG = MainActivity.class.getSimpleName();
    private FirebaseAnalytics mFirebaseAnalytics;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        // Obtain the FirebaseAnalytics instance.
        mFirebaseAnalytics = FirebaseAnalytics.getInstance(this);

        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        FloatingActionButton fab = (FloatingActionButton) findViewById(R.id.fab);
        if (null != fab) {
            // Log launch
            Bundle bundle = new Bundle();
            bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "documentation");
            mFirebaseAnalytics.logEvent(FirebaseAnalytics.Event.SELECT_CONTENT, bundle);

            fab.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    Intent browserIntent = new Intent(Intent.ACTION_VIEW, Uri.parse("https://github.com/ricardoquesada/unijoysticle/blob/master/DOCUMENTATION.md"));
                    startActivity(browserIntent);
                }
            });
        }

        // Disable Gyruss modes until they are implemented
//        RadioButton radioGyruss = (RadioButton) findViewById(R.id.radioButtonGyruss);
//        radioGyruss.setEnabled(false);

        final RadioGroup radioButtonGroupMode = (RadioGroup) findViewById(R.id.radioGroupMode);
        final RadioGroup radioButtonGroupJoy = (RadioGroup) findViewById(R.id.radioGroupJoy);

        // Comamnd and Home use both Joy#1 and Joy#2
        radioButtonGroupMode.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener()
        {
            @Override
            public void onCheckedChanged(RadioGroup group, int checkedId) {
                boolean enabled = (checkedId != R.id.radioButtonCommando &&
                        checkedId != R.id.radioButtonHome);
                for(int i = 0; i < radioButtonGroupJoy.getChildCount(); i++){
                    radioButtonGroupJoy.getChildAt(i).setEnabled(enabled);
                }
            }
        });

        Button startButton = (Button) findViewById(R.id.buttonStart);
        if (null != startButton) {
            startButton.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    Log.d(TAG, "Button pressed");

                    Bundle bundle = new Bundle();
                    bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "unknown");

                    int radioButtonModeID = radioButtonGroupMode.getCheckedRadioButtonId();
                    Intent i = null;
                    switch(radioButtonModeID) {
                        case R.id.radioButtonDPad:
                            i = new Intent(getApplicationContext(), DpadActivity.class);
                            bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "d-pad mode");
                            break;
//                        case R.id.radioButtonGyruss:
//                            i = new Intent(getApplicationContext(), GyrussActivity.class);
//                            bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "gyruss mode");
//                            break;
                        case R.id.radioButtonHome:
                            i = new Intent(getApplicationContext(), HomeActivity.class);
                            bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "commodore home mode");
                            break;
                        case R.id.radioButtonUniJoystiCle:
                            i = new Intent(getApplicationContext(), UnijoysticleActivity.class);
                            bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "unijoysticle mode");
                            break;
                        case R.id.radioButtonCommando:
                            i = new Intent(getApplicationContext(), CommandoActivity.class);
                            bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "commando mode");
                            break;
                    }
                    if (i != null) {

                        // which joy is selected: 0 or 1?
                        int radioButtonJoyID = radioButtonGroupJoy.getCheckedRadioButtonId();
                        View radioButton = radioButtonGroupJoy.findViewById(radioButtonJoyID);
                        int idx = radioButtonGroupJoy.indexOfChild(radioButton);

                        i.putExtra("joyPort", (byte)idx);
                        startActivity(i);

                        mFirebaseAnalytics.logEvent(FirebaseAnalytics.Event.SELECT_CONTENT, bundle);

                    } else {
                        Log.d(TAG, "Unknown error. Invalid radio button value");
                    }
                }
            });
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        //noinspection SimplifiableIfStatement
        if (id == R.id.action_settings) {
            Intent intent = new Intent(getApplicationContext(), SettingsActivity.class);
            startActivity(intent);

            Bundle bundle = new Bundle();
            bundle.putString(FirebaseAnalytics.Param.CONTENT_TYPE, "settings");
            mFirebaseAnalytics.logEvent(FirebaseAnalytics.Event.SELECT_CONTENT, bundle);
            return true;
        }

        return super.onOptionsItemSelected(item);
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
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
        super.onDestroy();
        // The activity is about to be destroyed.
    }
}
