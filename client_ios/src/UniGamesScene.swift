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
    var attitude: CMAttitude? = nil

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

            // update at 60Hz... 50hz should be for PAL, but lets do it for NTSC which is faster
            motionManager.deviceMotionUpdateInterval = 1.0/60.0
            motionManager.startDeviceMotionUpdatesToQueue(operationQueue, withHandler: {
                data, error in
                if error == nil {
                    self.rotationRate = data!.rotationRate
                    self.userAcceleration = data!.userAcceleration
                    self.attitude = data!.attitude
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

        self.labelX!.text = String(format:"x = %.2f", rotationRate.x)
        self.labelY!.text = String(format:"y = %.2f", rotationRate.y)
        self.labelZ!.text = String(format:"z = %.2f", rotationRate.z)
        self.labelState!.text = pad(String(joyState, radix: 2), toSize: 8)

        var movementThreshold = 0.3
        if self.attitude != nil {
            movementThreshold += abs(attitude!.roll)
        }
        let buttonThreshold = 0.8

//        print("min: \(zMin), max: \(zMax)")
//        print(self.labelState!.text)
//        print(self.userAcceleration);
        if abs(self.userAcceleration.z) > 0.5 {
            print(self.userAcceleration)
        }


        // Jumping? The press button
        if userAcceleration.z < -buttonThreshold
            && abs(userAcceleration.x) < 0.3 {          // to avoid confusing jump with rotation
            joyState |= JoyBits.Fire.rawValue
        }
        // hold it pressed until jump starts descend
        else if userAcceleration.z >= 0 {
            joyState &= ~JoyBits.Fire.rawValue
        }

        // if Fire don't do anything else
        if joyState & JoyBits.Fire.rawValue != 0 {
            joyState = JoyBits.Fire.rawValue
            super.update(currentTime)
            return
        }

        // userAcceleration.z (up and down) controls joy Up & Down for the unicycle game
        // userAcceleration.z > 0 == Joy down
        // userAcceleration.z < 0 == Joy up
        if (userAcceleration.z < -movementThreshold) {
            joyState &= ~JoyBits.Down.rawValue
            joyState |= JoyBits.Up.rawValue
        }
        else if (userAcceleration.z > movementThreshold) {
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
        if (userAcceleration.x > movementThreshold) {
            joyState &= ~JoyBits.Left.rawValue
            joyState |= JoyBits.Right.rawValue
        }
        else if (userAcceleration.x < -movementThreshold) {
            joyState &= ~JoyBits.Right.rawValue
            joyState |= JoyBits.Left.rawValue
        }
        else if (userAcceleration.x > -0.05 && userAcceleration.x < 0.05) {
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
