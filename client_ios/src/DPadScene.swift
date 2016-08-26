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

class DPadScene: ControllerScene, iCadeEventDelegate {

    var buttons_sprites = [SKSpriteNode]()
    // ASSERT(buttons_names same_order_as buttons_bitmask)
    // An OrderedDictionary could be used instead. Will be more robust
    let buttons_names = ["SKSpriteNode_top",
                         "SKSpriteNode_bottom",
                         "SKSpriteNode_left",
                         "SKSpriteNode_right",
                         "SKSpriteNode_fire",
                         "SKSpriteNode_topright",
                         "SKSpriteNode_topleft",
                         "SKSpriteNode_bottomleft",
                         "SKSpriteNode_bottomright"]
    let buttons_bitmaks = [JoyBits.Up.rawValue,
                           JoyBits.Down.rawValue,
                           JoyBits.Left.rawValue,
                           JoyBits.Right.rawValue,
                           JoyBits.Fire.rawValue,
                           JoyBits.Up.rawValue | JoyBits.Right.rawValue,
                           JoyBits.Up.rawValue | JoyBits.Left.rawValue,
                           JoyBits.Down.rawValue | JoyBits.Left.rawValue,
                           JoyBits.Down.rawValue | JoyBits.Right.rawValue]

    var labelBack:SKLabelNode? = nil
    var labelGController:SKLabelNode? = nil
    let STICK_THRESHLOLD:Float = 0.05
    var buttonBEnabled = BUTTON_B_ENABLED
    var swapABEnabled = SWAP_A_B_ENABLED

    override func didMoveToView(view: SKView) {
        /* Setup your scene here */
//        for (index, value) in self.children.enumerate() {
//             print("Item \(index + 1): \(value)")
//        }

        let settings = NSUserDefaults.standardUserDefaults()

        // Button B enabled?
        let buttonBValue = settings.valueForKey(SETTINGS_BUTTON_B_KEY)
        if (buttonBValue != nil) {
            buttonBEnabled = buttonBValue as! Bool
        }
        // Swap Buttons A & B?
        let swapABValue = settings.valueForKey(SETTINGS_SWAP_A_B_KEY)
        if (swapABValue != nil) {
            swapABEnabled = swapABValue as! Bool
        }

        for name in buttons_names {
            let sprite = childNodeWithName(name) as! SKSpriteNode!
            sprite.colorBlendFactor = 1
            sprite.color = UIColor.grayColor()
            assert(sprite != nil, "Invalid name")
            buttons_sprites.append(sprite)
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

        // iCade setup
        let icadeView = iCadeReaderView()
        icadeView.active = true
        icadeView.delegate = self
        self.view?.addSubview(icadeView)
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

            // if buttonB (fire) is not enabled, then turn it off
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
            if self.buttonBEnabled && self.swapABEnabled {
                // Jump Configuration
                if pressed {
                    self.joyState |= JoyBits.Up.rawValue
                } else {
                    self.joyState &= ~JoyBits.Up.rawValue
                }
            } else {
                // Shoot Configuration
                if pressed {
                    self.joyState |= JoyBits.Fire.rawValue
                } else {
                    self.joyState &= ~JoyBits.Fire.rawValue
                }
            }
            self.repaintButtons()
        }

        // button B (jump)
        controller.gamepad?.buttonB.pressedChangedHandler = { (button:GCControllerButtonInput, value:Float, pressed:Bool) in
            // only if Button B is enabled
            if !self.buttonBEnabled {
                return
            }

            if self.swapABEnabled {
                // Shoot Configuration
                if pressed {
                    self.joyState |= JoyBits.Fire.rawValue
                } else {
                    self.joyState &= ~JoyBits.Fire.rawValue
                }
            } else {
                // Jump Configuration
                if pressed {
                    self.joyState |= JoyBits.Up.rawValue
                } else {
                    self.joyState &= ~JoyBits.Up.rawValue
                }
            }
            self.repaintButtons()
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

        for (index, bitmask) in buttons_bitmaks.enumerate() {
            if (bitmask & JoyBits.DPad.rawValue != 0) {
                // testing dpad bitmask
                if bitmask == (joyState & JoyBits.DPad.rawValue) {
                    buttons_sprites[index].color = UIColor.redColor()
                } else {
                    buttons_sprites[index].color = UIColor.grayColor()
                }
            } else {
                if bitmask == (joyState & JoyBits.Fire.rawValue) {
                    buttons_sprites[index].color = UIColor.redColor()
                } else {
                    buttons_sprites[index].color = UIColor.grayColor()
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
        for (index, sprite) in buttons_sprites.enumerate() {
            if sprite.frame.contains(location) {
                joyState = joyState | buttons_bitmaks[index]
                sprite.color = UIColor.redColor()
            }
        }
    }

    func disableTouch(location: CGPoint) {
        for (index, sprite) in buttons_sprites.enumerate() {
            if sprite.frame.contains(location) {
                joyState = joyState & ~buttons_bitmaks[index]
                sprite.color = UIColor.grayColor()
            }
        }
    }

    //
    // iCade Delegate
    //
    func stateChanged(state:UInt16) -> Void {
        var extendedState = state

        // If Button "B" enabled, then deactivate the "joy up". You can only jump with Button B.
        // And if it is disabled, then deactivate Button B
        if !self.buttonBEnabled {
            extendedState &= ~iCadeButtons.ButtonB.rawValue
        } else {
            extendedState &= ~iCadeButtons.JoystickUp.rawValue
        }

        // not swapped: A=Fire, B=Jump
        if !swapABEnabled {
            if extendedState & iCadeButtons.ButtonB.rawValue != 0 {
                extendedState |= iCadeButtons.JoystickUp.rawValue
            }
        } else {
            // swapped: A=Jump, B=Fire
            if extendedState & iCadeButtons.ButtonA.rawValue != 0 {
                extendedState &= ~iCadeButtons.ButtonA.rawValue
                extendedState |= iCadeButtons.JoystickUp.rawValue
            }
            if extendedState & iCadeButtons.ButtonB.rawValue != 0 {
                extendedState |= iCadeButtons.ButtonA.rawValue
            }
        }

        joyState = (UInt8)(extendedState & 0b00011111)
        repaintButtons()
    }
    func buttonDown(state:UInt16) -> Void {
        // empty line. just to comply with the protocol
    }
    func buttonUp(state:UInt16) -> Void {
        // empty line. just to comply with the protocol
    }

}
