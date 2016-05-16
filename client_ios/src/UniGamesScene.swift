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

    // accel tmp
    var labelX:SKLabelNode? = nil
    var labelY:SKLabelNode? = nil
    var labelZ:SKLabelNode? = nil
    var labelState:SKLabelNode? = nil
    var labelBack:SKLabelNode? = nil
    var zMax:Double = -100
    var zMin:Double = 100

    let operationQueue = NSOperationQueue()

    override func didMoveToView(view: SKView) {

        labelX = childNodeWithName("SKLabelNode_x") as! SKLabelNode?
        labelY = childNodeWithName("SKLabelNode_y") as! SKLabelNode?
        labelZ = childNodeWithName("SKLabelNode_z") as! SKLabelNode?
        labelState = childNodeWithName("SKLabelNode_state") as! SKLabelNode?
        labelBack = childNodeWithName("SKLabelNode_back") as! SKLabelNode?

        // accelerometer stuff
        if motionManager.accelerometerAvailable == true {

            motionManager.startDeviceMotionUpdatesToQueue(operationQueue, withHandler: {
                data, error in
                self.rotationRate = data!.rotationRate
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

        self.labelX!.text = String(format:"x = %.2f", rotationRate.x)
        self.labelY!.text = String(format:"y = %.2f", rotationRate.y)
        self.labelZ!.text = String(format:"z = %.2f", rotationRate.z)
        self.labelState!.text = pad(String(joyState, radix: 2), toSize: 8)

//        print("min: \(zMin), max: \(zMax)")
        print(self.labelState!.text)
//        print(self.rotationRate);

        let movementThreshold = 0.8
        let buttonThreshold = 3.8

        // Jumping? The press button
        if rotationRate.x < -buttonThreshold {
            joyState |= JoyBits.Fire.rawValue
        }
        // hold it pressed until jump starts descend
        else if rotationRate.x >= 0 {
            joyState &= ~JoyBits.Fire.rawValue
        }

        // rotationRate.x (up and down) controls joy Up & Down for the unicycle game
        // rotationRate.x > 0 == Joy down
        // rotationRate.x < 0 == Joy up
        if (rotationRate.x < -movementThreshold) {
            joyState &= ~JoyBits.Down.rawValue
            joyState |= JoyBits.Up.rawValue
        }
        else if (rotationRate.x > movementThreshold) {
            joyState &= ~JoyBits.Up.rawValue
            joyState |= JoyBits.Down.rawValue
        }
        else if (rotationRate.x > -0.05 && rotationRate.x < 0.05) {
            // not moving. turn off up and down
            joyState &= ~(JoyBits.Up.rawValue | JoyBits.Down.rawValue)
        }

        // accel X (left and right) controls joy Left & Right for the unicycle game
        // Accel.X > 0 == Joy Right
        // Accel.X < 0 == Joy Left
        if (rotationRate.z < -movementThreshold) {
            joyState &= ~JoyBits.Left.rawValue
            joyState |= JoyBits.Right.rawValue
        }
        else if (rotationRate.z > movementThreshold) {
            joyState &= ~JoyBits.Right.rawValue
            joyState |= JoyBits.Left.rawValue
        }
        else if (rotationRate.z > -0.05 && rotationRate.z < 0.05) {
            // not moving. turn off left and right
            joyState &= ~(JoyBits.Right.rawValue | JoyBits.Left.rawValue)
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
