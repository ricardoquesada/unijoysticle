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

class CommandoScene: ControllerScene, iCadeEventDelegate {

    var buttons_sprites = [SKSpriteNode]()
    // ASSERT(buttons_names same_order_as buttons_bitmask)
    // An OrderedDictionary could be used instead. Will be more robust
    let buttons_names = [
        "SKSpriteNode_top1",
        "SKSpriteNode_bottom1",
        "SKSpriteNode_left1",
        "SKSpriteNode_right1",
        "SKSpriteNode_fire1",
        "SKSpriteNode_topright1",
        "SKSpriteNode_topleft1",
        "SKSpriteNode_bottomleft1",
        "SKSpriteNode_bottomright1",

        "SKSpriteNode_top2",
        "SKSpriteNode_bottom2",
        "SKSpriteNode_left2",
        "SKSpriteNode_right2",
        "SKSpriteNode_fire2",
        "SKSpriteNode_topright2",
        "SKSpriteNode_topleft2",
        "SKSpriteNode_bottomleft2",
        "SKSpriteNode_bottomright2"
    ]
    let buttons_bitmaks = [
        // joy 1
        JoyBits.up.rawValue,
        JoyBits.down.rawValue,
        JoyBits.left.rawValue,
        JoyBits.right.rawValue,
        JoyBits.fire.rawValue,
        JoyBits.up.rawValue | JoyBits.right.rawValue,
        JoyBits.up.rawValue | JoyBits.left.rawValue,
        JoyBits.down.rawValue | JoyBits.left.rawValue,
        JoyBits.down.rawValue | JoyBits.right.rawValue,

        // joy 2
        JoyBits.up.rawValue,
        JoyBits.down.rawValue,
        JoyBits.left.rawValue,
        JoyBits.right.rawValue,
        JoyBits.fire.rawValue,
        JoyBits.up.rawValue | JoyBits.right.rawValue,
        JoyBits.up.rawValue | JoyBits.left.rawValue,
        JoyBits.down.rawValue | JoyBits.left.rawValue,
        JoyBits.down.rawValue | JoyBits.right.rawValue
    ]

    var labelBack:SKLabelNode = SKLabelNode()
    var labelGController:SKLabelNode = SKLabelNode()
    let STICK_THRESHLOLD:Float = 0.05

    override func didMove(to view: SKView) {
        /* Setup your scene here */
        //        for (index, value) in self.children.enumerate() {
        //             print("Item \(index + 1): \(value)")
        //        }

        for name in buttons_names {
            let sprite = childNode(withName: name) as! SKSpriteNode!
            sprite?.colorBlendFactor = 1
            sprite?.color = UIColor.gray
            assert(sprite != nil, "Invalid name")
            buttons_sprites.append(sprite!)
        }

        labelBack = childNode(withName: "SKLabelNode_back") as! SKLabelNode!
        labelGController = childNode(withName: "SKLabelNode_controller") as! SKLabelNode!
        labelGController.text = "Connect a Game Controller"

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

        // tell network to use version 2
        self.protoVersion = 2
    }

    func connectControllers() {
        enableGamecontroller()
    }
    func controllerDisconnected() {
        labelGController.text = "Connect a Game Controller"
        self.protoHeader.joyState1 = 0
        self.protoHeader.joyState2 = 0
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
                labelGController.text = "Game Controller Detected"
                break
            }
        }
    }

    func registerGamepad(_ controller:GCController) {
        // gamepad
        controller.gamepad?.dpad.valueChangedHandler = { (dpad:GCControllerDirectionPad, xValue:Float, yValue:Float) in
            self.protoHeader.joyState2 &= ~(JoyBits.down.rawValue | JoyBits.left.rawValue | JoyBits.right.rawValue)

            self.protoHeader.joyState2 &= ~JoyBits.up.rawValue

            if dpad.right.isPressed {
                self.protoHeader.joyState2 |= JoyBits.right.rawValue
            } else if dpad.left.isPressed {
                self.protoHeader.joyState2 |= JoyBits.left.rawValue
            }
            if dpad.up.isPressed {
                self.protoHeader.joyState2 |= JoyBits.up.rawValue
            } else if dpad.down.isPressed {
                self.protoHeader.joyState2 |= JoyBits.down.rawValue
            }
            self.repaintButtons()
        }

        // button A (fire)
        controller.gamepad?.buttonA.pressedChangedHandler = { (button:GCControllerButtonInput, value:Float, pressed:Bool) in
            // Shoot Configuration
            if pressed {
                self.protoHeader.joyState2 |= JoyBits.fire.rawValue
            } else {
                self.protoHeader.joyState2 &= ~JoyBits.fire.rawValue
            }
            self.repaintButtons()
        }

        // button B (jump)
        controller.gamepad?.buttonB.pressedChangedHandler = { (button:GCControllerButtonInput, value:Float, pressed:Bool) in
            if pressed {
                self.protoHeader.joyState1 |= JoyBits.fire.rawValue
            } else {
                self.protoHeader.joyState1 &= ~JoyBits.fire.rawValue
            }
            self.repaintButtons()
        }
    }

    func registerExtendedGamepad(_ controller:GCController) {
        // left thumbstick (joy #2)
        controller.extendedGamepad?.leftThumbstick.valueChangedHandler = { (dpad:GCControllerDirectionPad, xValue:Float, yValue:Float) in
            self.protoHeader.joyState2 &= ~(JoyBits.dPad.rawValue)
            if xValue > self.STICK_THRESHLOLD {
                self.protoHeader.joyState2 |= JoyBits.right.rawValue
            } else if xValue < -self.STICK_THRESHLOLD {
                self.protoHeader.joyState2 |= JoyBits.left.rawValue
            }
            if yValue > self.STICK_THRESHLOLD {
                self.protoHeader.joyState2 |= JoyBits.up.rawValue
            } else if yValue < -self.STICK_THRESHLOLD {
                self.protoHeader.joyState2 |= JoyBits.down.rawValue
            }
            self.repaintButtons()
        }

        // right thumbstick (joy #1)
        controller.extendedGamepad?.rightThumbstick.valueChangedHandler = { (dpad:GCControllerDirectionPad, xValue:Float, yValue:Float) in
            self.protoHeader.joyState1 &= ~(JoyBits.dPad.rawValue)
            if xValue > self.STICK_THRESHLOLD {
                self.protoHeader.joyState1 |= JoyBits.right.rawValue
            } else if xValue < -self.STICK_THRESHLOLD {
                self.protoHeader.joyState1 |= JoyBits.left.rawValue
            }
            if yValue > self.STICK_THRESHLOLD {
                self.protoHeader.joyState1 |= JoyBits.up.rawValue
            } else if yValue < -self.STICK_THRESHLOLD {
                self.protoHeader.joyState1 |= JoyBits.down.rawValue
            }
            self.repaintButtons()
        }
    }

    func repaintButtons() {
        sendJoyState()

        // joy #1
        for (index, bitmask) in buttons_bitmaks.enumerated() {

            let jState = index < 9 ? protoHeader.joyState1 : protoHeader.joyState2
            if (bitmask & JoyBits.dPad.rawValue != 0) {
                // testing dpad bitmask
                if bitmask == (jState & JoyBits.dPad.rawValue) {
                    buttons_sprites[index].color = UIColor.red
                } else {
                    buttons_sprites[index].color = UIColor.gray
                }
            } else {
                if bitmask == (jState & JoyBits.fire.rawValue) {
                    buttons_sprites[index].color = UIColor.red
                } else {
                    buttons_sprites[index].color = UIColor.gray
                }
            }
        }
    }

    //
    // Detect "back"
    //
    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        /* Called when a touch begins */

        for touch in touches {

            let location = touch.location(in: self)
            if labelBack.frame.contains(location) {
                self.view!.window!.rootViewController!.dismiss(animated: false, completion: {
                    // reset state to avoid having the joystick pressed
                    self.protoHeader.joyState1 = 0
                    self.protoHeader.joyState2 = 0
                    self.sendJoyState()

                    // re-enable it.
                    UIApplication.shared.isIdleTimerDisabled = false

                    // stop controllers discovery in case it still active
                    GCController.stopWirelessControllerDiscovery()
                })
            }
        }
    }

    //
    // iCade Delegate
    //
    func stateChanged(_ state:UInt16) -> Void {

        // Joy #2 dpad
        protoHeader.joyState2 = (UInt8)(state & 0b00001111)

        // Joy #2 button: either button A or D
        if state & (iCadeButtons.buttonA.rawValue | iCadeButtons.buttonD.rawValue) != 0 {
            protoHeader.joyState2 |= JoyBits.fire.rawValue
        }

        // Joy #1 (only button is supported since it doesn't have two sticks)
        // Either button B or C
        if state & (iCadeButtons.buttonB.rawValue | iCadeButtons.buttonC.rawValue) != 0 {
            protoHeader.joyState1 = JoyBits.fire.rawValue
        } else {
            protoHeader.joyState1 = 0
        }
        
        repaintButtons()
    }
    func buttonDown(_ state:UInt16) -> Void {
        // empty line. just to comply with the protocol
    }
    func buttonUp(_ state:UInt16) -> Void {
        // empty line. just to comply with the protocol
    }
}
