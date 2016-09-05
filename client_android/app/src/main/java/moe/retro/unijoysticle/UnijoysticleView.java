/*
 * Copyright (C) 2013 The Android Open Source Project
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

import android.annotation.TargetApi;
import android.content.Context;
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.LightingColorFilter;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.graphics.PointF;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;
import java.util.ArrayList;

import android.view.Display;
import android.view.Surface;
import android.view.View;
import android.view.WindowManager;

import moe.retro.unijoysticle.unijosyticle.R;


public class UnijoysticleView extends View implements SensorEventListener {

    private static final String TAG = UnijoysticleView.class.getSimpleName();

    final static int JOY_UP       = 1 << 0;
    final static int JOY_DOWN     = 1 << 1;
    final static int JOY_LEFT     = 1 << 2;
    final static int JOY_RIGHT    = 1 << 3;
    final static int JOY_FIRE     = 1 << 4;

    private final float mMovementThreshold;
    private final float mJumpThreshold;
    private final float mRotationRatio;
    private Display mDisplay;


    private ArrayList<Sprite> mSprites;

    private SensorManager mSensorManager;
    private Sensor mAccelerometer;

    // sprites order: Top, Top-Right, Left, Right, Bottom-Left, Bottom, Bottom-Right, Top-Left, Fire
    private final byte mSpritesJoyBits[] = {
            0b0001, /* top */
            0b1001, /* top-right */
            0b0100, /* left */
            0b1000, /* right */
            0b0110, /* bottom-left */
            0b0010, /* bottom */
            0b1010, /* bottom-right */
            0b0101, /* top-left */
            0b10000, /* fire */
    };


    public UnijoysticleView(Context context, AttributeSet attrs) {
        super(context, attrs);

        final float GRAVITY = 9.81f;
        UnijoysticleActivity host = (UnijoysticleActivity) getContext();
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getContext());
        mMovementThreshold = preferences.getFloat(host.getString(R.string.key_movementThreshold),
                Float.parseFloat(host.getString(R.string.default_movementThreshold)))
                * GRAVITY;
        mJumpThreshold = preferences.getFloat(host.getString(R.string.key_jumpThreshold),
                Float.parseFloat(host.getString(R.string.default_jumpThreshold)))
                * GRAVITY;
        mRotationRatio = preferences.getFloat(host.getString(R.string.key_rotationRatio),
                Float.parseFloat(host.getString(R.string.default_rotationRatio)));

        setFocusable(true);
        setFocusableInTouchMode(true);

        mSensorManager = (SensorManager) getContext().getSystemService(Context.SENSOR_SERVICE);
        mAccelerometer = mSensorManager.getDefaultSensor(Sensor.TYPE_LINEAR_ACCELERATION);

        // Get an instance of the WindowManager
        WindowManager windowManager = (WindowManager) getContext().getSystemService(Context.WINDOW_SERVICE);
        mDisplay = windowManager.getDefaultDisplay();
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        mSensorManager.registerListener(this, mAccelerometer, SensorManager.SENSOR_DELAY_GAME);

        // when using the accel only, screen might go off. prevent it
        setKeepScreenOn(true);
    }

    @Override
    public void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        mSensorManager.unregisterListener(this);

        // turn it off
        setKeepScreenOn(false);
    }

    @Override
    public void onWindowFocusChanged(boolean hasWindowFocus) {
        // Turn on and off animations based on the window focus.
        // Alternately, we could update the game state using the Activity
        // onResume()
        // and onPause() lifecycle events.
        super.onWindowFocusChanged(hasWindowFocus);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        super.onDraw(canvas);
        // Update the animation

        // Draw the buttons
        int num = mSprites.size();
        for (int i = 0; i < num; i++) {
            Sprite sprite = mSprites.get(i);
            sprite.draw(canvas);
        }
    }

    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        // add nine sprites after we have the layout in order to know the View size
        mSprites = new ArrayList<Sprite>();
        for(int i=0; i<9; ++i) {
            Sprite sprite = new Sprite(i);
            mSprites.add(sprite);
        }
    }


    //
    // Sensor Implementation
    //
    @Override
    public void onSensorChanged(SensorEvent event) {

        Sensor mySensor = event.sensor;
        if (mySensor.getType() != Sensor.TYPE_LINEAR_ACCELERATION)
            return;

        UnijoysticleActivity host = (UnijoysticleActivity) getContext();

        float x = 0;
        float y = 0;
        float z = event.values[2];

        switch (mDisplay.getRotation()) {
            case Surface.ROTATION_0:
                x = event.values[0];
                y = event.values[1];
                break;
            case Surface.ROTATION_90:
                x = -event.values[1];
                y = event.values[0];
                break;
            case Surface.ROTATION_180:
                x = -event.values[0];
                y = -event.values[1];
                break;
            case Surface.ROTATION_270:
                x = event.values[1];
                y = -event.values[0];
                break;
        }


        float angle = 0;
        boolean validAngle = false;

        if (Math.abs(y) > mMovementThreshold || Math.abs(z) > mMovementThreshold) {
            // -y, and not y to simulate a forward movement, and not backwards.
            angle = (float) Math.atan2(-y, z);
            if (angle < 0) {
                angle += 2 * Math.PI;
            }
            angle = (float) (Math.toDegrees(angle) * mRotationRatio) % 360;
            validAngle = true;
        }

        // Jumping? The press button
        if (z < -mJumpThreshold) {
            /* && abs(self.userAcceleration.y) < noMovementThreshold { */
            host.mJoyState |= JOY_FIRE;
        }
        // hold it pressed until jump starts descend
        else if (z >= 0) {
            host.mJoyState &= ~JOY_FIRE;
        }

        // if Fire don't do anything else
        if ((host.mJoyState & JOY_FIRE) != 0) {
            host.mJoyState = JOY_FIRE;
        }

        // start clean
        host.mJoyState &= ~(JOY_UP | JOY_DOWN | JOY_LEFT | JOY_RIGHT);

        // allow jumping and moving at the same time. Don't use "else"
        if (validAngle) {

            // Z and X movements are related in a pedal
            // When X has its peak, it means that Z is almost 0.
            // And when Z has its peak, it means X is almost 0, at least if the unicycle were stationary

            // userAcceleration.z (up and down) controls joy Up & Down for the unicycle game
            // userAcceleration.z > 0 == Joy down
            // userAcceleration.z < 0 == Joy up
            if (angle > (90 - 67.5) && angle < (90 + 67.5)) {
                host.mJoyState &= ~JOY_DOWN;
                host.mJoyState |= JOY_UP;
            } else if (angle > (270 - 67.5) && angle < (270 + 67.5)) {
                host.mJoyState &= ~JOY_UP;
                host.mJoyState |= JOY_DOWN;
            }

            // userAcceleration.z y (left and right) controls joy Left & Right for the unicycle game
            // userAcceleration.z.y > 0 == Joy Right
            // userAcceleration.z.y < 0 == Joy Left
            if (angle > (360 - 67.5) || angle < 0 + 67.5) {
                host.mJoyState &= ~JOY_LEFT;
                host.mJoyState |= JOY_RIGHT;
            } else if (angle > (180 - 67.5) && angle < (180 + 67.5)) {
                host.mJoyState &= ~JOY_RIGHT;
                host.mJoyState |= JOY_LEFT;
            }
        }
        repaintButtons();
    }

    @Override
    public void onAccuracyChanged(Sensor sensor, int accuracy) {

    }

    private void repaintButtons() {
        UnijoysticleActivity host = (UnijoysticleActivity) getContext();
        for(int i=0; i<9; i++) {
            Sprite sprite = mSprites.get(i);
            if ((mSpritesJoyBits[i] & 0b00001111) != 0) {
                // testing dpad bitmask
                if (mSpritesJoyBits[i] == (host.mJoyState & 0b00001111)) {
                    sprite.setColor(Color.RED);
                } else {
                    sprite.setColor(Color.WHITE);
                }
            } else {
                if (mSpritesJoyBits[i] == (host.mJoyState & 0b00010000)) {
                    sprite.setColor(Color.RED);
                } else {
                    sprite.setColor(Color.WHITE);
                }
            }
        }
        host.sendJoyState();
        invalidate();
    }

    //
    // Sprite
    //
    class Sprite {
        public float mPosX;
        public float mPosY;
        public int mIndex;
        public Paint mPaint;
        public Bitmap mBitmap;
        public Matrix mMatrix;

        public Sprite(int i) {
            mIndex = i;
            mPaint = new Paint();

            Resources res = getResources();
            mMatrix = new Matrix();

            final int sw = getWidth();
            final int sh = getHeight();
            if (i<8) {
                // sprites order: Top, Top-Right, Left, Right, Bottom-Left, Bottom, Bottom-Right, Top-Left, Fire
                // positions:
                final int pos_x[] = {/**/  0,  1,    -1, /**/ 1,   -1,  0,  1,   -1};
                final int pos_y[] = {/**/ -1, -1,     0, /**/ 0,    1,  1,  1,   -1};
                // rotations:
                final int rots[] =  {    -90,  0,   180, /**/ 0,  180, 90, 90,  -90};
                // texture ids:
                final int texid[] = {      0,  1,     0,      0,    1,  0,  1,    1};

                if (texid[i]==0)
                    mBitmap = BitmapFactory.decodeResource(res, R.drawable.arrow_bold_right);
                else
                    mBitmap = BitmapFactory.decodeResource(res, R.drawable.arrow_bold_top_right);

                mPosX = sw/2 + pos_x[i] * mBitmap.getWidth() - mBitmap.getWidth() / 2;
                mPosY = sh/2 + pos_y[i] * mBitmap.getHeight() - mBitmap.getHeight() / 2;
                mMatrix.setRotate(rots[i], mBitmap.getWidth()/2, mBitmap.getHeight()/2);
            }  else if(i == 8) {
                // fire button
                mBitmap = BitmapFactory.decodeResource(res, R.drawable.button);
                mMatrix.setScale(0.75f, 0.75f, mBitmap.getWidth()/2, mBitmap.getHeight()/2);
                final int padding = 50;
                mPosX = sw/2 - mBitmap.getWidth()/2;
                mPosY = sh/2 - mBitmap.getHeight()/2;
            }
        }

        public void draw(Canvas canvas) {
            canvas.save();
            canvas.translate(mPosX, mPosY);
            canvas.drawBitmap(mBitmap, mMatrix, mPaint);
            canvas.restore();
        }

        public boolean containPoint(float x, float y) {
            return (x >= mPosX &&
                    x <= (mPosX + mBitmap.getWidth()) &&
                    y >= mPosY &&
                    y <= (mPosY + mBitmap.getHeight()));
        }

        public void setColor(int color) {
            final ColorFilter filter = new LightingColorFilter(color, 1);
            mPaint.setColorFilter(filter);
        }
    }
}
