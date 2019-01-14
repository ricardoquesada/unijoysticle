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
    let buttons_bitmaks = [JoyBits.up.rawValue,
                           JoyBits.down.rawValue,
                           JoyBits.left.rawValue,
                           JoyBits.right.rawValue,
                           JoyBits.fire.rawValue,
                           JoyBits.up.rawValue | JoyBits.right.rawValue,
                           JoyBits.up.rawValue | JoyBits.left.rawValue,
                           JoyBits.down.rawValue | JoyBits.left.rawValue,
                           JoyBits.down.rawValue | JoyBits.right.rawValue]

    var labelBack:SKLabelNode = SKLabelNode()
    var labelGController:SKLabelNode = SKLabelNode()
    let STICK_THRESHLOLD:Float = 0.05
    var buttonBEnabled = BUTTON_B_ENABLED
    var swapABEnabled = SWAP_A_B_ENABLED

    override func didMove(to view: SKView) {
        /* Setup your scene here */
//        for (index, value) in self.children.enumerate() {
//             print("Item \(index + 1): \(value)")
//        }

        let settings = UserDefaults.standard

        // Button B enabled?
        let buttonBValue = settings.value(forKey: SETTINGS_BUTTON_B_KEY)
        if (buttonBValue != nil) {
            buttonBEnabled = buttonBValue as! Bool
        }
        // Swap Buttons A & B?
        let swapABValue = settings.value(forKey: SETTINGS_SWAP_A_B_KEY)
        if (swapABValue != nil) {
            swapABEnabled = swapABValue as! Bool
        }

        for name in buttons_names {
            let sprite = childNode(withName: name) as! SKSpriteNode!
            sprite?.colorBlendFactor = 1
            sprite?.color = UIColor.gray
            assert(sprite != nil, "Invalid name")
            buttons_sprites.append(sprite!)
        }

        labelBack = childNode(withName: "SKLabelNode_back") as! SKLabelNode!
        labelGController = childNode(withName: "SKLabelNode_controller") as! SKLabelNode!
        labelGController.isHidden = true

        //
        // Game Controller Code
        //
        let center = NotificationCenter.default
        center.addObserver(self, selector: #selector(DPadScene.connectControllers), name: NSNotification.Name.GCControllerDidConnect, object: nil)
        center.addObserver(self, selector: #selector(DPadScene.controllerDisconnected), name: NSNotification.Name.GCControllerDidDisconnect, object: nil)

        let controllers = GCController.controllers()
        if controllers.isEmpty {
            GCController.startWirelessControllerDiscovery(completionHandler: {
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

    @objc func connectControllers() {
        enableGamecontroller()
    }
    @objc func controllerDisconnected() {
        labelGController.isHidden = true
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
                labelGController.isHidden = false
                break
            }
        }
    }

    func registerGamepad(_ controller:GCController) {
        // gamepad
        controller.gamepad?.dpad.valueChangedHandler = { (dpad:GCControllerDirectionPad, xValue:Float, yValue:Float) in
            self.joyState &= ~(JoyBits.down.rawValue | JoyBits.left.rawValue | JoyBits.right.rawValue)

            // if buttonB (fire) is not enabled, then turn it off
            if !self.buttonBEnabled {
                self.joyState &= ~JoyBits.up.rawValue
            }
            if dpad.right.isPressed {
                self.joyState |= JoyBits.right.rawValue
            } else if dpad.left.isPressed {
                self.joyState |= JoyBits.left.rawValue
            }
            if !self.buttonBEnabled && dpad.up.isPressed {
                self.joyState |= JoyBits.up.rawValue
            } else if dpad.down.isPressed {
                self.joyState |= JoyBits.down.rawValue
            }
            self.repaintButtons()
        }

        // button A (fire)
        controller.gamepad?.buttonA.pressedChangedHandler = { (button:GCControllerButtonInput, value:Float, pressed:Bool) in
            if self.buttonBEnabled && self.swapABEnabled {
                // Jump Configuration
                if pressed {
                    self.joyState |= JoyBits.up.rawValue
                } else {
                    self.joyState &= ~JoyBits.up.rawValue
                }
            } else {
                // Shoot Configuration
                if pressed {
                    self.joyState |= JoyBits.fire.rawValue
                } else {
                    self.joyState &= ~JoyBits.fire.rawValue
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
                    self.joyState |= JoyBits.fire.rawValue
                } else {
                    self.joyState &= ~JoyBits.fire.rawValue
                }
            } else {
                // Jump Configuration
                if pressed {
                    self.joyState |= JoyBits.up.rawValue
                } else {
                    self.joyState &= ~JoyBits.up.rawValue
                }
            }
            self.repaintButtons()
        }
    }

    func registerExtendedGamepad(_ controller:GCController) {
        // left thumbstick
        controller.extendedGamepad?.leftThumbstick.valueChangedHandler = { (dpad:GCControllerDirectionPad, xValue:Float, yValue:Float) in
            self.joyState &= ~(JoyBits.dPad.rawValue)
            if xValue > self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.right.rawValue
            } else if xValue < -self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.left.rawValue
            }
            if yValue > self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.up.rawValue
            } else if yValue < -self.STICK_THRESHLOLD {
                self.joyState |= JoyBits.down.rawValue
            }
            self.repaintButtons()
        }
    }

    func repaintButtons() {
        sendJoyState()

        for (index, bitmask) in buttons_bitmaks.enumerated() {
            if (bitmask & JoyBits.dPad.rawValue != 0) {
                // testing dpad bitmask
                if bitmask == (joyState & JoyBits.dPad.rawValue) {
                    buttons_sprites[index].color = UIColor.red
                } else {
                    buttons_sprites[index].color = UIColor.gray
                }
            } else {
                if bitmask == (joyState & JoyBits.fire.rawValue) {
                    buttons_sprites[index].color = UIColor.red
                } else {
                    buttons_sprites[index].color = UIColor.gray
                }
            }
        }
    }

    //
    // Virtual D-Pad code
    //
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        /* Called when a touch begins */

        for touch in touches {

            let location = touch.location(in: self)
            if labelBack.frame.contains(location) {
                self.view!.window!.rootViewController!.dismiss(animated: false, completion: {
                    // reset state to avoid having the joystick pressed
                    self.joyState = 0
                    self.sendJoyState()

                    // re-enable it.
                    UIApplication.shared.isIdleTimerDisabled = false

                    // stop controllers discovery in case it still active
                    GCController.stopWirelessControllerDiscovery()
                })
            }

            enableTouch(location)
            sendJoyState()
        }
    }

    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        for touch in touches {
            disableTouch(touch.previousLocation(in: self))
            enableTouch(touch.location(in: self))
            sendJoyState()
        }
    }

    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        for touch in touches {
            disableTouch(touch.previousLocation(in: self))
            disableTouch(touch.location(in: self))
            sendJoyState()
        }
    }

    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        for touch in touches {
            disableTouch(touch.previousLocation(in: self))
            disableTouch(touch.location(in: self))
            sendJoyState()
        }
    }

    func enableTouch(_ location: CGPoint) {
        for (index, sprite) in buttons_sprites.enumerated() {
            if sprite.frame.contains(location) {
                joyState = joyState | buttons_bitmaks[index]
                sprite.color = UIColor.red
            }
        }
    }

    func disableTouch(_ location: CGPoint) {
        for (index, sprite) in buttons_sprites.enumerated() {
            if sprite.frame.contains(location) {
                joyState = joyState & ~buttons_bitmaks[index]
                sprite.color = UIColor.gray
            }
        }
    }

    //
    // iCade Delegate
    //
    func stateChanged(_ state:UInt16) -> Void {
        var extendedState = state

        // If Button "B" enabled, then deactivate the "joy up". You can only jump with Button B.
        // And if it is disabled, then deactivate Button B
        if !self.buttonBEnabled {
            extendedState &= ~(iCadeButtons.buttonB.rawValue | iCadeButtons.buttonC.rawValue)
        } else {
            extendedState &= ~iCadeButtons.joystickUp.rawValue
        }

        if !swapABEnabled {
            // not swapped: A=Fire, B=Jump
            if extendedState & (iCadeButtons.buttonB.rawValue | iCadeButtons.buttonC.rawValue) != 0 {
                extendedState |= iCadeButtons.joystickUp.rawValue
            }
            if extendedState & (iCadeButtons.buttonA.rawValue | iCadeButtons.buttonD.rawValue) != 0 {
                extendedState |= iCadeButtons.buttonA.rawValue
            }
        } else {
            // swapped: A=Jump, B=Fire
            if extendedState & (iCadeButtons.buttonA.rawValue | iCadeButtons.buttonD.rawValue) != 0 {
                extendedState &= ~iCadeButtons.buttonA.rawValue
                extendedState |= iCadeButtons.joystickUp.rawValue
            }
            if extendedState & (iCadeButtons.buttonB.rawValue | iCadeButtons.buttonC.rawValue) != 0 {
                extendedState |= iCadeButtons.buttonA.rawValue
            }
        }

        joyState = (UInt8)(extendedState & 0b00011111)
        repaintButtons()
    }
    func buttonDown(_ state:UInt16) -> Void {
        // empty line. just to comply with the protocol
    }
    func buttonUp(_ state:UInt16) -> Void {
        // empty line. just to comply with the protocol
    }

}
