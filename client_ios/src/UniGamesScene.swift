/****************************************************************************
 http://retro.moe/unijoysticle

 Copyright 2016 Ricardo Quesada

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
import CoreMotion

class UniGamesScene: ControllerScene {

    let motionManager = CMMotionManager()
    var rotationRate = CMRotationRate()
    var userAcceleration = CMAcceleration()

    // accel tmp
    var buttons: [SKSpriteNode:UInt8] = [:]

    var labelX:SKLabelNode? = nil
    var labelY:SKLabelNode? = nil
    var labelZ:SKLabelNode? = nil
    var labelBack:SKLabelNode? = nil
    var zMax:Double = -100
    var zMin:Double = 100

    let operationQueue = NSOperationQueue()

    override func didMoveToView(view: SKView) {

        labelX = childNodeWithName("SKLabelNode_x") as! SKLabelNode?
        labelY = childNodeWithName("SKLabelNode_y") as! SKLabelNode?
        labelZ = childNodeWithName("SKLabelNode_z") as! SKLabelNode?
        labelBack = childNodeWithName("SKLabelNode_back") as! SKLabelNode?

        let names_bits = [
                           "SKSpriteNode_left": JoyBits.Left.rawValue,
                           "SKSpriteNode_top": JoyBits.Up.rawValue,
                           "SKSpriteNode_bottom": JoyBits.Down.rawValue,
                           "SKSpriteNode_right": JoyBits.Right.rawValue,
                           "SKSpriteNode_fire": JoyBits.Fire.rawValue]

        for (key,value) in names_bits {
            let sprite = childNodeWithName(key) as! SKSpriteNode!
            sprite.colorBlendFactor = 1
            sprite.color = UIColor.grayColor()
            assert(sprite != nil, "Invalid name")
            buttons[sprite] = value
        }


        // accelerometer stuff
        if motionManager.accelerometerAvailable == true {

            // update at 60Hz... 50hz should be for PAL, but lets do it for NTSC which is faster
            motionManager.deviceMotionUpdateInterval = 1.0/60.0
            motionManager.startDeviceMotionUpdatesToQueue(operationQueue, withHandler: {
                data, error in
                if error == nil {
                    self.rotationRate = data!.rotationRate
                    self.userAcceleration = data!.userAcceleration
                }
            })
        }
    }

    // taken from here: http://stackoverflow.com/a/26181323/1119460
    func pad(string : String, toSize: Int) -> String {
        var padded = string
        for _ in 0..<toSize - string.characters.count {
            padded = "0" + padded
        }
        return padded
    }

    override func update(currentTime: CFTimeInterval) {

        let buttonThreshold = 1.2
        let movementThreshold = 0.5
        let lateralThreshold = 1.0
        let noMovementThreshold = movementThreshold / 2

//        print("min: \(zMin), max: \(zMax)")
//        print(self.labelState!.text)
//        print(self.userAcceleration);
        if self.userAcceleration.z < -buttonThreshold {
            print(self.userAcceleration)
        }


        // Jumping? The press button
        if userAcceleration.z < -buttonThreshold &&
            abs(self.userAcceleration.y) > lateralThreshold { // lateral movment to simulate jump without conflict
            joyState |= JoyBits.Fire.rawValue
        }
        // hold it pressed until jump starts descend
        else if userAcceleration.z >= 0 {
            joyState &= ~JoyBits.Fire.rawValue
        }

        // if Fire don't do anything else
        if joyState & JoyBits.Fire.rawValue != 0 {
            joyState = JoyBits.Fire.rawValue
        }
        else {

            // Z and X movements are related in a pedal
            // When X has its peak, it means that Z is almost 0.
            // And when Z has its peak, it means X is almost 0, at least if the unicycle were stationary

            // userAcceleration.z (up and down) controls joy Up & Down for the unicycle game
            // userAcceleration.z > 0 == Joy down
            // userAcceleration.z < 0 == Joy up
            if (userAcceleration.z < -movementThreshold && abs(userAcceleration.x) < noMovementThreshold) {
                joyState &= ~JoyBits.Down.rawValue
                joyState |= JoyBits.Up.rawValue
            }
            else if (userAcceleration.z > movementThreshold && abs(userAcceleration.x) < noMovementThreshold) {
                joyState &= ~JoyBits.Up.rawValue
                joyState |= JoyBits.Down.rawValue
            }
            else if (userAcceleration.z > -0.05 && userAcceleration.z < 0.05) {
                // not moving. turn off up and down
                joyState &= ~(JoyBits.Up.rawValue | JoyBits.Down.rawValue)
            }

            // accel X (left and right) controls joy Left & Right for the unicycle game
            // Accel.X > 0 == Joy Right
            // Accel.X < 0 == Joy Left
            if (userAcceleration.x > movementThreshold && abs(userAcceleration.z) < noMovementThreshold) {
                joyState &= ~JoyBits.Left.rawValue
                joyState |= JoyBits.Right.rawValue
            }
            else if (userAcceleration.x < -movementThreshold && abs(userAcceleration.z) < noMovementThreshold) {
                joyState &= ~JoyBits.Right.rawValue
                joyState |= JoyBits.Left.rawValue
            }
            else if (userAcceleration.x > -0.05 && userAcceleration.x < 0.05) {
                // not moving. turn off left and right
                joyState &= ~(JoyBits.Right.rawValue | JoyBits.Left.rawValue)
            }
        }


        // update labels and sprites
        self.labelX!.text = String(format:"x = %.2f", userAcceleration.x)
        self.labelY!.text = String(format:"y = %.2f", userAcceleration.y)
        self.labelZ!.text = String(format:"z = %.2f", userAcceleration.z)

        for (sprite, bitmask) in buttons {
            if (joyState & bitmask) != 0 {
                sprite.color = UIColor.redColor()
            }
            else {
                sprite.color = UIColor.grayColor()
            }
        }

        // send joy status every update since UDP doesn't have resend and it is possible
        // that some packets are lost
        super.update(currentTime)
    }

    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {
            let location = touch.locationInNode(self)
            if labelBack!.frame.contains(location) {
                self.view!.window!.rootViewController!.dismissViewControllerAnimated(false, completion: {
                    // reset state to avoid having the joystick pressed
                    self.joyState = 0
                    self.sendJoyState()

                    // re-enable it.
                    UIApplication.sharedApplication().idleTimerDisabled = false
                })
            }
        }
    }
}
