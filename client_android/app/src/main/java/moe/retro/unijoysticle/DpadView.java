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
import android.graphics.Canvas;
import android.os.Build;
import android.util.AttributeSet;
import android.util.Log;
import android.view.InputDevice;
import android.view.InputEvent;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.View;
import android.hardware.input.InputManager;

// OUYA profile:
// Dpad up: 104
// Dpad down: 105
// Dpad left: 109
// Dpad right: 108
// A:  96
// B: 99
// X: 97
// Y: 98
// stick left button: 102
// stick right button: 103
// Menu: 107
// L1: 100
// L2: 110 + ???
// R1: 101
// R2: 106 + ???


/*
 * A trivial joystick based physics game to demonstrate joystick handling. If
 * the game controller has a vibrator, then it is used to provide feedback when
 * a bullet is fired or the ship crashes into an obstacle. Otherwise, the system
 * vibrator is used for that purpose.
 */
@TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
public class DpadView extends View implements InputManager.InputDeviceListener {

    private static final String TAG = DpadView.class.getSimpleName();

    final static int JOY_UP       = 1 << 0;
    final static int JOY_DOWN     = 1 << 1;
    final static int JOY_LEFT     = 1 << 2;
    final static int JOY_RIGHT    = 1 << 3;
    final static int JOY_FIRE     = 1 << 4;

    private final InputManager mInputManager;

    public DpadView(Context context, AttributeSet attrs) {
        super(context, attrs);

        setFocusable(true);
        setFocusableInTouchMode(true);

        mInputManager = (InputManager) this.getContext().getSystemService(Context.INPUT_SERVICE);
        mInputManager.registerInputDeviceListener(this, null);
        findControllers();
    }

    void findControllers() {
        int[] deviceIds = mInputManager.getInputDeviceIds();
        for (int deviceId : deviceIds) {
            InputDevice dev = mInputManager.getInputDevice(deviceId);
            int sources = dev.getSources();
            // if the device is a gamepad/joystick, create a ship to represent it
            if (((sources & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
                    ((sources & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK)) {
                // if the device has a gamepad or joystick
                Log.d(TAG, "Joystick / Gamepad found:" + deviceId + dev.getName());
            }
        }
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {
        int deviceId = event.getDeviceId();
        if (deviceId != -1) {
            Log.d(TAG, "onKeyDown. keyCode:" + keyCode);
            int dpadValue = getDirectionPressed(event);
            if (dpadValue != -1) {
                Log.d(TAG, "Dpad value: " + dpadValue);
                return true;
            }
        }

        return super.onKeyDown(keyCode, event);
    }

    @Override
    public boolean onKeyUp(int keyCode, KeyEvent event) {
        int deviceId = event.getDeviceId();
        if (deviceId != -1) {
            Log.d(TAG, "onKeyUp. keyCode:" + keyCode);
            int dpadValue = getDirectionPressed(event);
            if (dpadValue != -1) {
                Log.d(TAG, "Dpad value: " + dpadValue);
                return true;
            }
        }
        return super.onKeyUp(keyCode, event);
    }

    @Override
    public boolean onGenericMotionEvent(MotionEvent event) {

        int dpadValue = getDirectionPressed(event);
        if (dpadValue != -1) {
            if (dpadValue != 0) {
                Log.d(TAG, "Dpad value: " + dpadValue);
                return true;
            }
        }

        // Check that the event came from a joystick or gamepad since a generic
        // motion event could be almost anything. API level 18 adds the useful
        // event.isFromSource() helper function.
        int eventSource = event.getSource();
        if ((((eventSource & InputDevice.SOURCE_GAMEPAD) == InputDevice.SOURCE_GAMEPAD) ||
                ((eventSource & InputDevice.SOURCE_JOYSTICK) == InputDevice.SOURCE_JOYSTICK))
                && event.getAction() == MotionEvent.ACTION_MOVE) {
            int id = event.getDeviceId();
            if (id != -1) {
                Log.d(TAG, "onGenericMotionEvent. event:" + event.toString());
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
    }

    /**
     * Any gamepad button + the spacebar or DPAD_CENTER will be used as the fire
     * key.
     *
     * @param keyCode
     * @return true of it's a fire key.
     */
    private static boolean isFireKey(int keyCode) {
        return KeyEvent.isGamepadButton(keyCode)
                || keyCode == KeyEvent.KEYCODE_DPAD_CENTER
                || keyCode == KeyEvent.KEYCODE_SPACE;
    }

    /*
     * When an input device is added, we add a ship based upon the device.
     * @see
     * com.example.inputmanagercompat.InputManagerCompat.InputDeviceListener
     * #onInputDeviceAdded(int)
     */
    @Override
    public void onInputDeviceAdded(int deviceId) {
        Log.d(TAG, "onInputDeviceAdded: " + deviceId + ", " + InputDevice.getDevice(deviceId).getName());
    }

    /*
     * This is an unusual case. Input devices don't typically change, but they
     * certainly can --- for example a device may have different modes. We use
     * this to make sure that the ship has an up-to-date InputDevice.
     * @see
     * com.example.inputmanagercompat.InputManagerCompat.InputDeviceListener
     * #onInputDeviceChanged(int)
     */
    @Override
    public void onInputDeviceChanged(int deviceId) {
        Log.d(TAG, "onInputDeviceChanged: " + deviceId + ", " + InputDevice.getDevice(deviceId).getName());
    }

    /*
     * Remove any ship associated with the ID.
     * @see
     * com.example.inputmanagercompat.InputManagerCompat.InputDeviceListener
     * #onInputDeviceRemoved(int)
     */
    @Override
    public void onInputDeviceRemoved(int deviceId) {
        Log.d(TAG, "onInputDeviceRemoved: " + deviceId + ", " + InputDevice.getDevice(deviceId).getName());
    }

    // Code taken from here:
    // https://developer.android.com/training/game-controllers/controller-input.html
    public int getDirectionPressed(InputEvent event) {

        int directionPressed = -1;

        if (!isDpadDevice(event)) {
            return -1;
        }

        // If the input event is a MotionEvent, check its hat axis values.
        if (event instanceof MotionEvent) {

            directionPressed = 0;

            // Use the hat axis value to find the D-pad direction
            MotionEvent motionEvent = (MotionEvent) event;
            float xaxis = motionEvent.getAxisValue(MotionEvent.AXIS_HAT_X);
            float yaxis = motionEvent.getAxisValue(MotionEvent.AXIS_HAT_Y);

            // Check if the AXIS_HAT_X value is -1 or 1, and set the D-pad
            // LEFT and RIGHT direction accordingly.
            if (Float.compare(xaxis, -1.0f) == 0) {
                directionPressed |= JOY_LEFT;
            } else if (Float.compare(xaxis, 1.0f) == 0) {
                directionPressed |= JOY_RIGHT;
            }
            // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
            // UP and DOWN direction accordingly.
            if (Float.compare(yaxis, -1.0f) == 0) {
                directionPressed |= JOY_UP;
            } else if (Float.compare(yaxis, 1.0f) == 0) {
                directionPressed |= JOY_DOWN;
            }
        }

        // If the input event is a KeyEvent, check its key code.
        else if (event instanceof KeyEvent) {

            // Use the key code to find the D-pad direction.
            KeyEvent keyEvent = (KeyEvent) event;
            if (keyEvent.getKeyCode() == KeyEvent.KEYCODE_DPAD_LEFT) {
                directionPressed |= JOY_LEFT;
            } else if (keyEvent.getKeyCode() == KeyEvent.KEYCODE_DPAD_RIGHT) {
                directionPressed |= JOY_RIGHT;
            } else if (keyEvent.getKeyCode() == KeyEvent.KEYCODE_DPAD_UP) {
                directionPressed = JOY_UP;
            } else if (keyEvent.getKeyCode() == KeyEvent.KEYCODE_DPAD_DOWN) {
                directionPressed = JOY_DOWN;
            } else if (keyEvent.getKeyCode() == KeyEvent.KEYCODE_DPAD_CENTER) {
                // do nothing
            }
        }
        return directionPressed;
    }

    public boolean isDpadDevice(InputEvent event) {
        // Check that input comes from a device with directional pads.
        if ((event.getSource() & InputDevice.SOURCE_DPAD)
                != InputDevice.SOURCE_DPAD) {
            return true;
        } else {
            return false;
        }
    }
}
