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
import android.os.Build;
import android.preference.PreferenceManager;
import android.util.AttributeSet;
import android.util.Log;
import java.util.ArrayList;

import android.util.SparseArray;
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
public class DpadView extends View implements InputManager.InputDeviceListener {

    private static final String TAG = DpadView.class.getSimpleName();

    private final static int JOY_UP       = 1 << 0;
    private final static int JOY_DOWN     = 1 << 1;
    private final static int JOY_LEFT     = 1 << 2;
    private final static int JOY_RIGHT    = 1 << 3;
    private final static int JOY_FIRE     = 1 << 4;
    // there is not such thing as button B in commodore 64, but
    // this bit is used as temp value when storing the game controller info.
    private final static int JOY_FIRE_B   = 1 << 5;

    private final InputManager mInputManager;
    private Controller mController = null;
    private SparseArray<PointF> mActivePointers;
    private final boolean mEnableButtonB;
    private final boolean mSwapButtonsAB;

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

    public DpadView(Context context, AttributeSet attrs) {
        super(context, attrs);

        mActivePointers = new SparseArray<PointF>();

        DpadActivity host = (DpadActivity) getContext();
        SharedPreferences preferences = PreferenceManager.getDefaultSharedPreferences(getContext());
        mEnableButtonB = preferences.getBoolean(host.getString(R.string.key_enableButtonB), false);
        mSwapButtonsAB = preferences.getBoolean(host.getString(R.string.key_swapButtonsAB), false);

        setFocusable(true);
        setFocusableInTouchMode(true);

        mInputManager = (InputManager) this.getContext().getSystemService(Context.INPUT_SERVICE);
        findControllers();

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

            DpadActivity host = (DpadActivity) getContext();
            host.setTitle(host.getString(R.string.action_bar_title_dpad) + ": " + devName);
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
                DpadActivity host = (DpadActivity) getContext();
                host.mJoyState |= dpadValue;
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
                DpadActivity host = (DpadActivity) getContext();
                host.mJoyState &= ~(byte)(dpadValue & 0xff);
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
                DpadActivity host = (DpadActivity) getContext();
                byte maskedValue = (byte) (dpadValue & 0b00001111);
                host.mJoyState &= 0b11110000;
                host.mJoyState |= maskedValue;
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
        int num = mSprites.size();
        for (int i = 0; i < num; i++) {
            Sprite sprite = mSprites.get(i);
            sprite.draw(canvas);
        }
    }

    private void repaintButtons() {
        DpadActivity host = (DpadActivity) getContext();
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
        DpadActivity host = (DpadActivity) getContext();
        host.setTitle(host.getString(R.string.action_bar_title_dpad));
    }

    //
    @Override
    public boolean onTouchEvent(MotionEvent e) {
        // MotionEvent reports input details from the touch screen
        // and other input controls. In this case, you are only
        // interested in events where the touch position changed.

        boolean handled = false;

        int actionMask = e.getActionMasked();

        switch (actionMask) {
            case MotionEvent.ACTION_MOVE:
                for (int size = e.getPointerCount(), i = 0; i < size; i++) {
                    int pid = e.getPointerId(i);
                    // get previous position and disable it
                    PointF oldP = mActivePointers.get(pid);
                    disableTouch(oldP.x, oldP.y);
                    float xx = e.getX(i);
                    float yy = e.getY(i);
                    mActivePointers.put(pid, new PointF(xx, yy));
                    handled |= enableTouch(xx, yy);
                }
                break;
            case MotionEvent.ACTION_DOWN: {
                // one finger, use pointer 0
                final int pointerId = e.getPointerId(0);
                final float x = e.getX();
                final float y = e.getY();
                mActivePointers.put(pointerId, new PointF(x, y));
                handled |= enableTouch(x, y);
                break;
            }
            case MotionEvent.ACTION_POINTER_DOWN: {
                final int pointerIndex = e.getActionIndex();
                final int pointerId = e.getPointerId(pointerIndex);
                final float x = e.getX(pointerIndex);
                final float y = e.getY(pointerIndex);
                mActivePointers.put(pointerId, new PointF(x, y));
                handled |= enableTouch(x, y);
                break;
            }
            case MotionEvent.ACTION_UP: {
                // one finger, use pointer 0
                final int pointerId = e.getPointerId(0);
                final float x = e.getX();
                final float y = e.getY();
                mActivePointers.delete(pointerId);
                handled |= disableTouch(x, y);
                break;
            }
            case MotionEvent.ACTION_POINTER_UP: {
                final int pointerIndex = e.getActionIndex();
                final int pointerId = e.getPointerId(pointerIndex);
                final float x = e.getX(pointerIndex);
                final float y = e.getY(pointerIndex);
                mActivePointers.delete(pointerId);
                handled |= disableTouch(x, y);
                break;
            }
            case MotionEvent.ACTION_CANCEL:
                for (int size = e.getPointerCount(), i = 0; i < size; i++) {
                    int pid = e.getPointerId(i);
                    mActivePointers.delete(pid);
                    handled |= disableTouch(e.getX(i), e.getY(i));
                }
                break;
        }
        if (handled) {
            DpadActivity host = (DpadActivity) getContext();
            host.sendJoyState();
            return true;
        }
        return super.onTouchEvent(e);
    }

    private boolean disableTouch(float x, float y) {
        boolean handled = false;
        DpadActivity host = (DpadActivity) getContext();
        for(int i=0; i<9; ++i) {
            final Sprite sprite = mSprites.get(i);
            if (sprite.containPoint(x,y)) {
                sprite.setColor(Color.WHITE);
                host.mJoyState &= ~mSpritesJoyBits[i];
                invalidate();
                handled = true;
            }
        }
        return handled;
    }

    private boolean enableTouch(float x, float y) {
        boolean handled = false;
        DpadActivity host = (DpadActivity) getContext();
        for(int i=0; i<9; ++i) {
            final Sprite sprite = mSprites.get(i);
            if (sprite.containPoint(x,y)) {
                sprite.setColor(Color.RED);
                host.mJoyState |= mSpritesJoyBits[i];
                invalidate();
                handled = true;
            }
        }
        return handled;
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

        int transformDpadValues(int directionPressed) {

            // if buttonB enabled, then the only way to jump is by either pressing A or B
            // and not Dpad UP
            if (mEnableButtonB) {
                directionPressed &= ~JOY_UP;

                if (mSwapButtonsAB) {
                    // A=Jump, B=Shoot
                    if ((directionPressed & JOY_FIRE) != 0)
                        directionPressed |= JOY_UP;
                    // after JOY_FIRE bit was tested, clean it
                    directionPressed &= ~JOY_FIRE;
                    if ((directionPressed & JOY_FIRE_B) != 0)
                        directionPressed |= JOY_FIRE;
                } else {
                    if ((directionPressed & JOY_FIRE_B) != 0)
                        directionPressed |= JOY_UP;
                }
            }

            // remove extra bits, like JOY_FIRE_B
            directionPressed &= 0b00011111;
            return directionPressed;
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
                float xaxis = motionEvent.getAxisValue(MotionEvent.AXIS_HAT_X);
                float yaxis = motionEvent.getAxisValue(MotionEvent.AXIS_HAT_Y);

                // Dpad
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

                // Left Joystick
                // Check if the AXIS_X value is -1 or 1
                xaxis = motionEvent.getAxisValue(MotionEvent.AXIS_X);
                yaxis = motionEvent.getAxisValue(MotionEvent.AXIS_Y);
                if (xaxis < -0.5f) {
                    directionPressed |= JOY_LEFT;
                } else if (xaxis > 0.5f) {
                    directionPressed |= JOY_RIGHT;
                }
                // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
                // UP and DOWN direction accordingly.
                if (yaxis < -0.5f) {
                    directionPressed |= JOY_UP;
                } else if (yaxis > 0.5f) {
                    directionPressed |= JOY_DOWN;
                }
            }

            // If the input event is a KeyEvent, check its key code.
            else if (event instanceof KeyEvent) {

                // Use the key code to find the D-pad direction.
                final int keyCode = ((KeyEvent)event).getKeyCode();

                if (keyCode == KeyEvent.KEYCODE_DPAD_LEFT) {
                    directionPressed |= JOY_LEFT;
                    processed = true;
                } else if (keyCode == KeyEvent.KEYCODE_DPAD_RIGHT) {
                    directionPressed |= JOY_RIGHT;
                    processed = true;
                }

                if (keyCode == KeyEvent.KEYCODE_DPAD_UP) {
                    directionPressed |= JOY_UP;
                    processed = true;
                } else if (keyCode == KeyEvent.KEYCODE_DPAD_DOWN) {
                    directionPressed |= JOY_DOWN;
                    processed = true;
                }
                if (keyCode == KeyEvent.KEYCODE_BUTTON_THUMBL || keyCode == KeyEvent.KEYCODE_BUTTON_A) {
                    directionPressed |= JOY_FIRE;
                    processed = true;
                }
                if (keyCode == KeyEvent.KEYCODE_BUTTON_B) {
                    directionPressed |= JOY_FIRE_B;
                    processed = true;
                }
            }
            if (processed)
                return transformDpadValues(directionPressed);
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

        @Override
        public int getDirectionPressed(InputEvent event) {

            if (!isDpadDevice(event))
                return -1;

            int directionPressed = 0;

            // If the input event is a MotionEvent, check its hat axis values.
            if (event instanceof MotionEvent) {

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

                // Left Joystick
                // Check if the AXIS_X value is -1 or 1
                xaxis = motionEvent.getAxisValue(MotionEvent.AXIS_X);
                yaxis = motionEvent.getAxisValue(MotionEvent.AXIS_Y);
                if (xaxis < -0.5f) {
                    directionPressed |= JOY_LEFT;
                } else if (xaxis > 0.5f) {
                    directionPressed |= JOY_RIGHT;
                }
                // Check if the AXIS_HAT_Y value is -1 or 1, and set the D-pad
                // UP and DOWN direction accordingly.
                if (yaxis < -0.5f) {
                    directionPressed |= JOY_UP;
                } else if (yaxis > 0.5f) {
                    directionPressed |= JOY_DOWN;
                }
            }

            // If the input event is a KeyEvent, check its key code.
            else if (event instanceof KeyEvent) {

                // Use the key code to find the D-pad direction.
                int keyCode = ((KeyEvent)event).getKeyCode();

                if (keyCode == OUYA_DPAD_LEFT) {
                    directionPressed |= JOY_LEFT;
                } else if (keyCode == OUYA_DPAD_RIGHT) {
                    directionPressed |= JOY_RIGHT;
                }
                if (keyCode == OUYA_DPAD_UP) {
                    directionPressed |= JOY_UP;
                } else if (keyCode == OUYA_DPAD_DOWN) {
                    directionPressed |= JOY_DOWN;
                }

                if (keyCode == OUYA_STICK_BUTTON_LEFT || keyCode == OUYA_BUTTON_A) {
                    directionPressed |= JOY_FIRE;
                }
                if (keyCode == OUYA_BUTTON_B) {
                    directionPressed |= JOY_FIRE_B;
                }
            }
            return transformDpadValues(directionPressed);
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

                final int padding = 2;
                mPosX = (pos_x[i] + 1) * mBitmap.getWidth() + padding;
                mPosY = sh - mBitmap.getHeight() * (3-(pos_y[i]+1)) - padding;
                mMatrix.setRotate(rots[i], mBitmap.getWidth()/2, mBitmap.getHeight()/2);
            }  else if(i == 8) {
                // fire button
                mBitmap = BitmapFactory.decodeResource(res, R.drawable.button);
                final int padding = 50;
                mPosX = sw - mBitmap.getWidth() - padding;
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
