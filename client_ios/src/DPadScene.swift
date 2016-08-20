/****************************************************************************
 http://retro.moe/unijoysticle

 Copyright © 2016 Ricardo Quesada. All rights reserved.

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 ****************************************************************************/

import SpriteKit
import GameController

class DPadScene: ControllerScene {

    var buttons: [SKSpriteNode:UInt8] = [:]
    var labelBack:SKLabelNode? = nil
    var labelGController:SKLabelNode? = nil
    let STICK_THRESHLOLD:Float = 0.05
    var buttonBEnabled = BUTTON_B_ENABLED

    override func didMoveToView(view: SKView) {
        /* Setup your scene here */
//        for (index, value) in self.children.enumerate() {
//             print("Item \(index + 1): \(value)")
//        }

        let settings = NSUserDefaults.standardUserDefaults()

        let buttonBValue = settings.valueForKey(SETTINGS_BUTTON_B_KEY)
        if (buttonBValue != nil) {
            buttonBEnabled = buttonBValue as! Bool
        }

        let names_bits = [
            "SKSpriteNode_top": JoyBits.Up.rawValue,
            "SKSpriteNode_bottom": JoyBits.Down.rawValue,
            "SKSpriteNode_left": JoyBits.Left.rawValue,
            "SKSpriteNode_right": JoyBits.Right.rawValue,
            "SKSpriteNode_fire": JoyBits.Fire.rawValue,
            "SKSpriteNode_topright": JoyBits.Up.rawValue | JoyBits.Right.rawValue,
            "SKSpriteNode_topleft": JoyBits.Up.rawValue | JoyBits.Left.rawValue,
            "SKSpriteNode_bottomleft": JoyBits.Down.rawValue | JoyBits.Left.rawValue,
            "SKSpriteNode_bottomright": JoyBits.Down.rawValue | JoyBits.Right.rawValue]

        for (key,value) in names_bits {
            let sprite = childNodeWithName(key) as! SKSpriteNode!
            sprite.colorBlendFactor = 1
            sprite.color = UIColor.grayColor()
            assert(sprite != nil, "Invalid name")
            buttons[sprite] = value
        }

        labelBack = childNodeWithName("SKLabelNode_back") as! SKLabelNode?
        labelGController = childNodeWithName("SKLabelNode_controller") as! SKLabelNode?
        labelGController?.hidden = true

        //
        // Game Controller Code
        //
        let center = NSNotificationCenter.defaultCenter()
        center.addObserver(self, selector: #selector(DPadScene.connectControllers), name: GCControllerDidConnectNotification, object: nil)
        center.addObserver(self, selector: #selector(DPadScene.controllerDisconnected), name: GCControllerDidDisconnectNotification, object: nil)

        let controllers = GCController.controllers()
        if controllers.isEmpty {
            GCController.startWirelessControllerDiscoveryWithCompletionHandler({
                let controllers = GCController.controllers()
                if controllers.isEmpty {
                    print("No Controllers found :(")
                } else {
                    print("Controllers detected: \(controllers)")
                    self.enableGamecontroller()
                }
            })
        } else {
            print("Controllers detected: \(controllers)")
            enableGamecontroller()
        }
    }

    func connectControllers() {
        enableGamecontroller()
    }
    func controllerDisconnected() {
        labelGController?.hidden = true
        joyState = 0
        sendJoyState()
    }

    func enableGamecontroller() {
        let controllers = GCController.controllers()

        for controller in controllers {
            if controller.gamepad != nil {
                registerGamepad(controller)

                if controller.extendedGamepad != nil {
                    registerExtendedGamepad(controller)
                }
                labelGController?.hidden = false
                break
            }
        }
    }

    func registerGamepad(controller:GCController) {
        // gamepad
        controller.gamepad?.dpad.valueChangedHandler = { (dpad:GCControllerDirectionPad, xValue:Float, yValue:Float) in
            self.joyState &= ~(JoyBits.Down.rawValue | JoyBits.Left.rawValue | JoyBits.Right.rawValue)

            // if buttonB (fire) is not pressed, then turn it off
            if !self.buttonBEnabled {
                self.joyState &= ~JoyBits.Up.rawValue
            }
            if dpad.right.pressed {
                self.joyState |= JoyBits.Right.rawValue
            } else if dpad.left.pressed {
                self.joyState |= JoyBits.Left.rawValue
            }
            if !self.buttonBEnabled && dpad.up.pressed {
                self.joyState |= JoyBits.Up.rawValue
            } else if dpad.down.pressed {
                self.joyState |= JoyBits.Down.rawValue
            }
            self.repaintButtons()
        }

        // button A (fire)
        controller.gamepad?.buttonA.pressedChangedHandler = { (button:GCControllerButtonInput, value:Float, pressed:Bool) in
            if pressed {
                self.joyState |= JoyBits.Fire.rawValue
            } else {
                self.joyState &= ~JoyBits.Fire.rawValue
            }
            self.repaintButtons()
        }

        // button B (jump)
        controller.gamepad?.buttonB.pressedChangedHandler = { (button:GCControllerButtonInput, value:Float, pressed:Bool) in
            if self.buttonBEnabled {
                if pressed {
                    self.joyState |= JoyBits.Up.rawValue
                } else if controller.gamepad?.dpad.up.pressed == false {
                    // only turn off "fire" if both "up" and "button B" are off
                    self.joyState &= ~JoyBits.Up.rawValue
                }
                self.repaintButtons()
            }
        }
    }

    func registerExtendedGamepad(controller:GCController) {
        // left thumbstick
        controller.extendedGamepad?.leftThumbstick.valueChangedHandler = { (dpad:GCControllerDirectionPad, xValue:Float, yValue:Float) in
            self.joyState &= ~(JoyBits.DPad.rawValue)
            if xValue > self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.Right.rawValue
            } else if xValue < -self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.Left.rawValue
            }
            if yValue > self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.Up.rawValue
            } else if yValue < -self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.Down.rawValue
            }
            self.repaintButtons()
        }

        // right thumbstick
        controller.extendedGamepad?.rightThumbstick.valueChangedHandler = { (dpad:GCControllerDirectionPad, xValue:Float, yValue:Float) in
            self.joyState &= ~(JoyBits.DPad.rawValue)
            if xValue > self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.Right.rawValue
            } else if xValue < -self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.Left.rawValue
            }
            if yValue > self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.Up.rawValue
            } else if yValue < -self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.Down.rawValue
            }
            self.repaintButtons()
        }
    }

    func repaintButtons() {
        sendJoyState()
        for (sprite, bitmask) in buttons {
            // compare for exact values when testing the dpad
            // so that a diagonal is represented as a diagonal
            // and not as up+right+up_right_diagonal
            if (joyState & JoyBits.DPad.rawValue) != 0 {
                if bitmask == joyState & JoyBits.DPad.rawValue {
                    sprite.color = UIColor.redColor()
                } else {
                    sprite.color = UIColor.grayColor()
                }
            } else {
                // fire button
                if joyState & bitmask != 0 {
                    sprite.color = UIColor.redColor()
                } else {
                    sprite.color = UIColor.grayColor()
                }
            }
        }
    }

    //
    // Virtual D-Pad code
    //
    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
        /* Called when a touch begins */

        for touch in touches {

            let location = touch.locationInNode(self)
            if labelBack!.frame.contains(location) {
                self.view!.window!.rootViewController!.dismissViewControllerAnimated(false, completion: {
                    // reset state to avoid having the joystick pressed
                    self.joyState = 0
                    self.sendJoyState()

                    // re-enable it.
                    UIApplication.sharedApplication().idleTimerDisabled = false

                    // stop controllers discovery in case it still active
                    GCController.stopWirelessControllerDiscovery()
                })
            }

            enableTouch(location)
            sendJoyState()
        }
    }

    override func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {

            disableTouch(touch.previousLocationInNode(self))
            enableTouch(touch.locationInNode(self))
            sendJoyState()
        }
    }

    override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {

            disableTouch(touch.previousLocationInNode(self))
            disableTouch(touch.locationInNode(self))
            sendJoyState()
        }
    }

    override func touchesCancelled(touches: Set<UITouch>?, withEvent event: UIEvent?) {
        if (touches != nil) {
            for touch in touches! {

                disableTouch(touch.previousLocationInNode(self))
                disableTouch(touch.locationInNode(self))
                sendJoyState()
            }
        }
    }

    func enableTouch(location: CGPoint) {
        for (sprite, bitmask) in buttons {
            if sprite.frame.contains(location) {
                joyState = joyState | bitmask
                sprite.color = UIColor.redColor()
            }
        }
    }

    func disableTouch(location: CGPoint) {
        for (sprite, bitmask) in buttons {
            if sprite.frame.contains(location) {
                joyState = joyState & ~bitmask
                sprite.color = UIColor.grayColor()
            }
        }
    }
}
