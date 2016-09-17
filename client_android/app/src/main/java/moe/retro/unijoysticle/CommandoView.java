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
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.ColorFilter;
import android.graphics.LightingColorFilter;
import android.graphics.Matrix;
import android.graphics.Paint;
import android.os.Build;
import android.util.AttributeSet;
import android.util.Log;
import java.util.ArrayList;

import android.view.InputDevice;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.hardware.input.InputManager;

import moe.retro.unijoysticle.unijosyticle.R;


/*
 * A trivial joystick based physics game to demonstrate joystick handling. If
 * the game controller has a vibrator, then it is used to provide feedback when
 * a bullet is fired or the ship crashes into an obstacle. Otherwise, the system
 * vibrator is used for that purpose.
 */
@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
public class CommandoView extends View implements InputManager.InputDeviceListener {

    private static final String TAG = CommandoView.class.getSimpleName();

    private final static int JOY_UP1       = 1 << 0;
    private final static int JOY_DOWN1     = 1 << 1;
    private final static int JOY_LEFT1     = 1 << 2;
    private final static int JOY_RIGHT1    = 1 << 3;
    private final static int JOY_FIRE1     = 1 << 4;

    private final static int JOY_UP2       = 1 << 8;
    private final static int JOY_DOWN2     = 1 << 9;
    private final static int JOY_LEFT2     = 1 << 10;
    private final static int JOY_RIGHT2    = 1 << 11;
    private final static int JOY_FIRE2     = 1 << 12;

    private final InputManager mInputManager;
    private Controller mController = null;

    private ArrayList<Sprite> mSprites;
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

    public CommandoView(Context context, AttributeSet attrs) {
        super(context, attrs);

        CommandoActivity host = (CommandoActivity) getContext();

        setFocusable(true);
        setFocusableInTouchMode(true);

        mInputManager = (InputManager) this.getContext().getSystemService(Context.INPUT_SERVICE);
        findControllers();

        host.mProtoVersion = 2;
    }

    private void findControllers() {
        int[] deviceIds = mInputManager.getInputDeviceIds();
        for (int deviceId : deviceIds) {
            if (createController(deviceId))
                break;
        }
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        mInputManager.registerInputDeviceListener(this, null);
    }

    @Override
    public void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        mInputManager.unregisterInputDeviceListener(this);
    }

    private boolean createController(int deviceId) {
        InputDevice dev = mInputManager.getInputDevice(deviceId);
        int sources = dev.getSources();
        // if the device is a gamepad/joystick, create a ship to represent it
        if (
                ((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
                ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)
                ) {
            Log.d(TAG, "Name: " + dev.getName() + " Descriptor: " + dev.getDescriptor());

            String devName = dev.getName();
            if (devName.startsWith("OUYA")) {
                mController = new OUYAController();
            } else {
                mController = new GenericController();
            }

            CommandoActivity host = (CommandoActivity) getContext();
            host.setTitle(host.getString(R.string.action_bar_title_commando) + ": " + devName);
            return true;
        }
        return false;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        int deviceId = event.getDeviceId();
        if (deviceId != -1 && mController != null) {
            int dpadValue = mController.getDirectionPressed(event);
            if (dpadValue != -1) {
                CommandoActivity host = (CommandoActivity) getContext();
                host.mProtoHeader.joyState1 |= (dpadValue & 0b00011111);
                host.mProtoHeader.joyState2 |= (dpadValue >> 8);
                repaintButtons();
                return true;
            }
        }

        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        int deviceId = event.getDeviceId();
        if (deviceId != -1 && mController != null) {
            int dpadValue = mController.getDirectionPressed(event);
            if (dpadValue != -1) {
                CommandoActivity host = (CommandoActivity) getContext();
                host.mProtoHeader.joyState1 &= ~(byte)(dpadValue & 0xff);
                host.mProtoHeader.joyState2 &= ~(byte)(dpadValue >> 8);
                repaintButtons();
                return true;
            }
        }
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {

        if (mController != null) {
            int dpadValue = mController.getDirectionPressed(event);
            if (dpadValue != -1) {
                CommandoActivity host = (CommandoActivity) getContext();
                byte maskedValue1 = (byte) (dpadValue & 0b00001111);
                byte maskedValue2 = (byte) (dpadValue >> 8);
                host.mProtoHeader.joyState1 &= 0b11110000;
                host.mProtoHeader.joyState1 |= maskedValue1;
                host.mProtoHeader.joyState2 &= 0b11110000;
                host.mProtoHeader.joyState2 |= maskedValue2;
                repaintButtons();
                return true;
            }
        }
        return super.onGenericMotionEvent(event);
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
        final int num = mSprites.size();
        for (int i = 0; i < num; i++) {
            Sprite sprite = mSprites.get(i);
            sprite.draw(canvas);
        }
    }

    private void repaintButtons() {
        CommandoActivity host = (CommandoActivity) getContext();
        final int num = mSprites.size();
        for(int i=0; i<num; i++) {

            final int j = i % 9;
            byte jState = i<9 ? host.mProtoHeader.joyState2 : host.mProtoHeader.joyState1;
            Sprite sprite = mSprites.get(i);
            if ((mSpritesJoyBits[j] & 0b00001111) != 0) {
                // testing dpad bitmask
                if (mSpritesJoyBits[j] == (jState & 0b00001111)) {
                    sprite.setColor(Color.RED);
                } else {
                    sprite.setColor(Color.WHITE);
                }
            } else {
                if (mSpritesJoyBits[j] == (jState & 0b00010000)) {
                    sprite.setColor(Color.RED);
                } else {
                    sprite.setColor(Color.WHITE);
                }
            }
        }
        host.sendJoyState();
        invalidate();

    }

    /*
     * When an input device is added, we add a ship based upon the device.
     * @see
     * com.example.inputmanagercompat.InputManagerCompat.InputDeviceListener
     * #onInputDeviceAdded(int)
     */
    @Override
    public void onInputDeviceAdded(int deviceId) {
        Log.d(TAG, "onInputDeviceAdded: " + deviceId);
        createController(deviceId);
    }

    /*
     * This is an unusual case. Input devices don't typically change, but they
     * certainly can --- for example a device may have different modes
     * @see
     * com.example.inputmanagercompat.InputManagerCompat.InputDeviceListener
     * #onInputDeviceChanged(int)
     */
    @Override
    public void onInputDeviceChanged(int deviceId) {
        Log.d(TAG, "onInputDeviceChanged: " + deviceId);
        createController(deviceId);
    }

    /*
     * @see
     * com.example.inputmanagercompat.InputManagerCompat.InputDeviceListener
     * #onInputDeviceRemoved(int)
     */
    @Override
    public void onInputDeviceRemoved(int deviceId) {
        Log.d(TAG, "onInputDeviceRemoved: " + deviceId);
        CommandoActivity host = (CommandoActivity) getContext();
        host.setTitle(host.getString(R.string.action_bar_title_commando));
    }

    //
    @Override
    protected void onSizeChanged(int w, int h, int oldw, int oldh) {
        super.onSizeChanged(w, h, oldw, oldh);

        // add nine sprites after we have the layout in order to know the View size
        mSprites = new ArrayList<Sprite>();
        for(int i=0; i<18; ++i) {
            Sprite sprite = new Sprite(i);
            mSprites.add(sprite);
        }
    }

    //
    // Controller logic
    //
    public abstract class Controller {
        abstract public int getDirectionPressed(InputEvent event);

        public boolean isDpadDevice(InputEvent event) {
            // Check that input comes from a device with directional pads.
            return (
                    ((event.getSource() & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
                    ((event.getSource() & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)
            );
        }
    }

    // Code taken from here:
    // https://developer.android.com/training/game-controllers/controller-input.html
    public class GenericController extends Controller {
        @Override
        public int getDirectionPressed(InputEvent event) {

            int directionPressed = 0;
            boolean processed = false;

            if (!isDpadDevice(event)) {
                return -1;
            }

            // If the input event is a MotionEvent, check its hat axis values.
            if (event instanceof MotionEvent) {

                processed = true;

                // Use the hat axis value to find the D-pad direction
                MotionEvent motionEvent = (MotionEvent) event;

                // Joy #2: Dpad
                // Check if the AXIS_HAT_X value is -1 or 1, and set the D-pad
                // LEFT and RIGHT direction accordingly.
                float xaxis2 = motionEvent.getAxisValue(MotionEvent.AXIS_HAT_X);
                float yaxis2 = motionEvent.getAxisValue(MotionEvent.AXIS_HAT_Y);
                if (Float.compare(xaxis2, -1.0f) == 0) {
                    directionPressed |= JOY_LEFT2;
                } else if (Float.compare(xaxis2, 1.0f) == 0) {
                    directionPressed |= JOY_RIGHT2;
                }
                // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
                // UP and DOWN direction accordingly.
                if (Float.compare(yaxis2, -1.0f) == 0) {
                    directionPressed |= JOY_UP2;
                } else if (Float.compare(yaxis2, 1.0f) == 0) {
                    directionPressed |= JOY_DOWN2;
                }

                // Joy #2: Left Joystick
                // Check if the AXIS_X value is -1 or 1
                xaxis2 = motionEvent.getAxisValue(MotionEvent.AXIS_X);
                yaxis2 = motionEvent.getAxisValue(MotionEvent.AXIS_Y);
                if (xaxis2 < -0.5f) {
                    directionPressed |= JOY_LEFT2;
                } else if (xaxis2 > 0.5f) {
                    directionPressed |= JOY_RIGHT2;
                }
                // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
                // UP and DOWN direction accordingly.
                if (yaxis2 < -0.5f) {
                    directionPressed |= JOY_UP2;
                } else if (yaxis2 > 0.5f) {
                    directionPressed |= JOY_DOWN2;
                }


                // Joy #1: Right Joystick
                // Check if the AXIS_HAT_X value is -1 or 1, and set the D-pad
                // LEFT and RIGHT direction accordingly.
                float xaxis1 = motionEvent.getAxisValue(MotionEvent.AXIS_Z);
                float yaxis1 = motionEvent.getAxisValue(MotionEvent.AXIS_RZ);
                if (xaxis1 < -0.5f) {
                    directionPressed |= JOY_LEFT1;
                } else if (xaxis1 > 0.5f) {
                    directionPressed |= JOY_RIGHT1;
                }
                // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
                // UP and DOWN direction accordingly.
                if (yaxis1 < -0.5f) {
                    directionPressed |= JOY_UP1;
                } else if (yaxis1 > 0.5f) {
                    directionPressed |= JOY_DOWN1;
                }
            }

            // If the input event is a KeyEvent, check its key code.
            else if (event instanceof KeyEvent) {

                // Use the key code to find the D-pad direction.
                final int keyCode = ((KeyEvent)event).getKeyCode();

                if (keyCode == KeyEvent.KEYCODE_DPAD_LEFT) {
                    directionPressed |= JOY_LEFT2;
                    processed = true;
                } else if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT) {
                    directionPressed |= JOY_RIGHT2;
                    processed = true;
                }

                if (keyCode == KeyEvent.KEYCODE_DPAD_UP) {
                    directionPressed |= JOY_UP2;
                    processed = true;
                } else if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN) {
                    directionPressed |= JOY_DOWN2;
                    processed = true;
                }
                if (keyCode == KeyEvent.KEYCODE_BUTTON_A || keyCode == KeyEvent.KEYCODE_BUTTON_THUMBL) {
                    directionPressed |= JOY_FIRE2;
                    processed = true;
                }

                if (keyCode == KeyEvent.KEYCODE_BUTTON_B || keyCode == KeyEvent.KEYCODE_BUTTON_THUMBR) {
                    directionPressed |= JOY_FIRE1;
                    processed = true;
                }
            }
            if (processed)
                return directionPressed;
            return -1;
        }
    }

    public class OUYAController extends Controller {
        // OUYA profile:
        // Dpad up, down, left, right: 104, 105, 109, 108
        // A, B, X, Y:  96, 99, 97, 98
        // stick left / right button: 102, 103
        // Menu: 107
        // L1, R1: 100, 101
        // L2, R2: 110 + ???, 106 + ???

        static final int OUYA_DPAD_UP = 104;
        static final int OUYA_DPAD_DOWN = 105;
        static final int OUYA_DPAD_LEFT = 109;
        static final int OUYA_DPAD_RIGHT = 108;
        static final int OUYA_BUTTON_A = 96;
        static final int OUYA_BUTTON_B = 99;
        static final int OUYA_STICK_BUTTON_LEFT = 102;
        static final int OUYA_STICK_BUTTON_RIGHT = 103;

        @Override
        public int getDirectionPressed(InputEvent event) {

            if (!isDpadDevice(event))
                return -1;

            int directionPressed = 0;

            // If the input event is a MotionEvent, check its hat axis values.
            if (event instanceof MotionEvent) {

                // Use the hat axis value to find the D-pad direction
                MotionEvent motionEvent = (MotionEvent) event;

                // Joy #2
                // Check if the AXIS_HAT_X value is -1 or 1, and set the D-pad
                // LEFT and RIGHT direction accordingly.
                float xaxis2 = motionEvent.getAxisValue(MotionEvent.AXIS_HAT_X);
                float yaxis2 = motionEvent.getAxisValue(MotionEvent.AXIS_HAT_Y);
                if (Float.compare(xaxis2, -1.0f) == 0) {
                    directionPressed |= JOY_LEFT2;
                } else if (Float.compare(xaxis2, 1.0f) == 0) {
                    directionPressed |= JOY_RIGHT2;
                }
                // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
                // UP and DOWN direction accordingly.
                if (Float.compare(yaxis2, -1.0f) == 0) {
                    directionPressed |= JOY_UP2;
                } else if (Float.compare(yaxis2, 1.0f) == 0) {
                    directionPressed |= JOY_DOWN2;
                }

                // Joy #2: Left Joystick
                // Check if the AXIS_X value is -1 or 1
                xaxis2 = motionEvent.getAxisValue(MotionEvent.AXIS_X);
                yaxis2 = motionEvent.getAxisValue(MotionEvent.AXIS_Y);
                if (xaxis2 < -0.5f) {
                    directionPressed |= JOY_LEFT2;
                } else if (xaxis2 > 0.5f) {
                    directionPressed |= JOY_RIGHT2;
                }
                // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
                // UP and DOWN direction accordingly.
                if (yaxis2 < -0.5f) {
                    directionPressed |= JOY_UP2;
                } else if (yaxis2 > 0.5f) {
                    directionPressed |= JOY_DOWN2;
                }

                // Joy #1
                // Check if the AXIS_HAT_X value is -1 or 1, and set the D-pad
                // LEFT and RIGHT direction accordingly.
                float xaxis1 = motionEvent.getAxisValue(MotionEvent.AXIS_RX);
                float yaxis1 = motionEvent.getAxisValue(MotionEvent.AXIS_RY);
                if (xaxis1 < -0.5f) {
                    directionPressed |= JOY_LEFT1;
                } else if (xaxis1 > 0.5f) {
                    directionPressed |= JOY_RIGHT1;
                }
                // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
                // UP and DOWN direction accordingly.
                if (yaxis1 < -0.5f) {
                    directionPressed |= JOY_UP1;
                } else if (yaxis1 > 0.5f) {
                    directionPressed |= JOY_DOWN1;
                }
            }

            // If the input event is a KeyEvent, check its key code.
            else if (event instanceof KeyEvent) {

                // Use the key code to find the D-pad direction.
                int keyCode = ((KeyEvent)event).getKeyCode();

                if (keyCode == OUYA_DPAD_LEFT) {
                    directionPressed |= JOY_LEFT2;
                } else if (keyCode == OUYA_DPAD_RIGHT) {
                    directionPressed |= JOY_RIGHT2;
                }
                if (keyCode == OUYA_DPAD_UP) {
                    directionPressed |= JOY_UP2;
                } else if (keyCode == OUYA_DPAD_DOWN) {
                    directionPressed |= JOY_DOWN2;
                }

                if (keyCode == OUYA_STICK_BUTTON_LEFT || keyCode == OUYA_BUTTON_A) {
                    directionPressed |= JOY_FIRE2;
                }
                if (keyCode == OUYA_STICK_BUTTON_RIGHT || keyCode == OUYA_BUTTON_B) {
                    directionPressed |= JOY_FIRE1;
                }
            }
            return directionPressed;
        }
    }

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

            i = i % 9;

            // sprites order: Top, Top-Right, Left, Right, Bottom-Left, Bottom, Bottom-Right, Top-Left, Fire
            // positions:
            final int pos_x[] = {/**/  0,  1,    -1, /**/ 1,   -1,  0,  1,   -1,  0};
            final int pos_y[] = {/**/ -1, -1,     0, /**/ 0,    1,  1,  1,   -1,  0};
            // rotations:
            final int rots[] =  {    -90,  0,   180, /**/ 0,  180, 90, 90,  -90,  0};
            // texture ids:
            final int texid[] = {      0,  1,     0,      0,    1,  0,  1,    1,  2};

            if (texid[i]==0)
                mBitmap = BitmapFactory.decodeResource(res, R.drawable.arrow_bold_right);
            else if (texid[i]==1)
                mBitmap = BitmapFactory.decodeResource(res, R.drawable.arrow_bold_top_right);
            else
                mBitmap = BitmapFactory.decodeResource(res, R.drawable.button);

            final int paddingX = (mIndex < 9) ? sw/4 : 3*sw/4;
            final int paddingY = sh/2;

            mPosX = pos_x[i] * mBitmap.getWidth() - mBitmap.getWidth()/2 + paddingX;
            mPosY = sh - mBitmap.getHeight() * (1-pos_y[i]) + mBitmap.getHeight()/2 - paddingY;
            mMatrix.setRotate(rots[i], mBitmap.getWidth()/2, mBitmap.getHeight()/2);
            if (i==8)
                mMatrix.setScale(0.6f, 0.6f, mBitmap.getWidth()/2, mBitmap.getHeight()/2);

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
