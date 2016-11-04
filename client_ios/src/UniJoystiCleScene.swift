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
import CoreMotion

func radiansToDegrees(_ rads: Double) -> Double {
    return rads * 180 / M_PI
}

class UniJoystiCleScene: ControllerScene {

    var jumpThreshold = JUMP_THRESHOLD
    var movementThreshold = MOVEMENT_THRESHOLD
    var wheelRotationRate = ROTATION_RATE

    let motionManager = CMMotionManager()
    var userAcceleration = CMAcceleration()

    // accel tmp
    var buttons: [SKSpriteNode:UInt8] = [:]

    var labelBack:SKLabelNode = SKLabelNode()
    var doTheJump:Bool = false

    let operationQueue = OperationQueue()

    override func didMove(to view: SKView) {

        // read settings
        let settings = UserDefaults.standard
        let handicapValue = settings.value(forKey: SETTINGS_ROTATION_RATE_KEY)
        if (handicapValue != nil) {
            wheelRotationRate = Double(handicapValue as! Float)
        }
        let jumpValue = settings.value(forKey: SETTINGS_JUMP_THRESHOLD_KEY)
        if (jumpValue != nil) {
            jumpThreshold = Double(jumpValue as! Float)
        }
        let movementValue = settings.value(forKey: SETTINGS_MOVEMENT_THRESHOLD_KEY)
        if (movementValue != nil) {
            movementThreshold = Double(movementValue as! Float)
        }

        // setup labes and other stuff
        labelBack = childNode(withName: "SKLabelNode_back") as! SKLabelNode!

        let names_bits = [
                           "SKSpriteNode_left": JoyBits.left.rawValue,
                           "SKSpriteNode_top": JoyBits.up.rawValue,
                           "SKSpriteNode_bottom": JoyBits.down.rawValue,
                           "SKSpriteNode_right": JoyBits.right.rawValue,
                           "SKSpriteNode_fire": JoyBits.fire.rawValue]

        for (key,value) in names_bits {
            let sprite = childNode(withName: key) as! SKSpriteNode!
            sprite?.colorBlendFactor = 1
            sprite?.color = UIColor.gray
            assert(sprite != nil, "Invalid name")
            buttons[sprite!] = value
        }


        // accelerometer stuff
        if motionManager.isAccelerometerAvailable == true {

            // jump is very sensitive, should be as fast as possible
            motionManager.deviceMotionUpdateInterval = 1.0/120.0
            motionManager.startDeviceMotionUpdates(to: operationQueue, withHandler: {
                data, error in
                if error == nil {
                    self.userAcceleration = data!.userAcceleration

                    // since motionManager frequency != game frequency, record
                    // the jump state in case it is lost
                    if (self.userAcceleration.z < -self.jumpThreshold) {
                        self.doTheJump = true
                    }
                }
            })
        }
    }

    // taken from here: http://stackoverflow.com/a/26181323/1119460
    func pad(_ string : String, toSize: Int) -> String {
        var padded = string
        for _ in 0..<toSize - string.characters.count {
            padded = "0" + padded
        }
        return padded
    }

    override func update(_ currentTime: TimeInterval) {


        let prevState = joyState
//        print("min: \(zMin), max: \(zMax)")
//        print(self.labelState!.text)
//        print(self.userAcceleration);
//        if self.userAcceleration.z < -jumpThreshold {
//            print(self.userAcceleration)
//            print(self.rotationRate)
//        }

        var angle = 0.0
        var validAngle = false
        if abs(userAcceleration.y) > movementThreshold || abs(userAcceleration.z) > movementThreshold {
            angle = atan2(-self.userAcceleration.y, self.userAcceleration.z)
            if angle < 0 {
                angle += (2 * M_PI)
            }
            angle = (radiansToDegrees(angle) * self.wheelRotationRate).truncatingRemainder(dividingBy: 360)
            validAngle = true
//            print (angle)
        }

        // Jumping? The press button
        if doTheJump || userAcceleration.z < -jumpThreshold {
            /* && abs(self.userAcceleration.y) < noMovementThreshold { */
            joyState |= JoyBits.fire.rawValue
            doTheJump = false
        }
        // hold it pressed until jump starts descend
        else if userAcceleration.z >= 0 {
            joyState &= ~JoyBits.fire.rawValue
        }

        // if Fire don't do anything else
        if joyState & JoyBits.fire.rawValue != 0 {
            joyState = JoyBits.fire.rawValue
        }

        // start clean
        joyState &= ~(JoyBits.right.rawValue | JoyBits.left.rawValue | JoyBits.up.rawValue | JoyBits.down.rawValue)

        // allow jumping and moving at the same time. Don't use "else"
        if (validAngle) {

            // Z and X movements are related in a pedal
            // When X has its peak, it means that Z is almost 0.
            // And when Z has its peak, it means X is almost 0, at least if the unicycle were stationary

            // userAcceleration.z (up and down) controls joy Up & Down for the unicycle game
            // userAcceleration.z > 0 == Joy down
            // userAcceleration.z < 0 == Joy up
            if (angle > (90-67.5) && angle < (90+67.5)) {
                joyState &= ~JoyBits.down.rawValue
                joyState |= JoyBits.up.rawValue
            }
            else if (angle > (270-67.5) && angle < (270+67.5)){
                joyState &= ~JoyBits.up.rawValue
                joyState |= JoyBits.down.rawValue
            }

            // userAcceleration.z y (left and right) controls joy Left & Right for the unicycle game
            // userAcceleration.z.y > 0 == Joy Right
            // userAcceleration.z.y < 0 == Joy Left
            if (angle > (360-67.5) || angle < 0 + 67.5) {
                joyState &= ~JoyBits.left.rawValue
                joyState |= JoyBits.right.rawValue
            }
            else if (angle > (180-67.5) && angle < (180+67.5)) {
                joyState &= ~JoyBits.right.rawValue
                joyState |= JoyBits.left.rawValue
            }
        }

        if prevState != joyState {
            sendJoyState()

            // update labels and sprites
            for (sprite, bitmask) in buttons {
                if (joyState & bitmask) != 0 {
                    sprite.color = UIColor.red
                }
                else {
                    sprite.color = UIColor.gray
                }
            }
        }
    }

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        for touch in touches {
            let location = touch.location(in: self)
            if labelBack.frame.contains(location) {
                self.view!.window!.rootViewController!.dismiss(animated: false, completion: {
                    // reset state to avoid having the joystick pressed
                    self.joyState = 0
                    self.sendJoyState()

                    // re-enable it.
                    UIApplication.shared.isIdleTimerDisabled = false
                })
            }
        }
    }
}
