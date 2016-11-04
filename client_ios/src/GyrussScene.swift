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

// from: http://stackoverflow.com/a/25089974
func circleInRect(_ rect:CGRect) -> CGPath {
    // Adjust position so path is centered in shape
    let adjustedRect = CGRect(x: rect.origin.x-rect.size.width/2, y: rect.origin.y-rect.size.height/2, width: rect.size.width, height: rect.size.height);
    let bezierPath = UIBezierPath(ovalIn: adjustedRect)
    return bezierPath.cgPath;
}

class GyrussScene: ControllerScene {

    let ANGLE_COVER = 45.0 + 45.0 / 2
    let GRAVITY = 9.8
    var gravityFactor = GRAVITY_FACTOR
    let motionManager = CMMotionManager()
    var userAcceleration = CMAcceleration()

    // accel tmp
    var buttons: [SKSpriteNode:UInt8] = [:]

    var labelBack:SKLabelNode = SKLabelNode()
    var spriteFire:SKSpriteNode = SKSpriteNode()
    var spriteBall:SKSpriteNode = SKSpriteNode()
    // will be used to calculate the angle. this will be used as the "center"
    var centerPos:CGPoint = CGPoint.zero

    let operationQueue = OperationQueue()

    override func didMove(to view: SKView) {

        // read settings
        let settings = UserDefaults.standard

        let gravityValue = settings.value(forKey: SETTINGS_GRAVITY_FACTOR_KEY)
        if (gravityValue != nil) {
            gravityFactor = Double(gravityValue as! Float)
        }

        // setup sprites
        labelBack = childNode(withName: "SKLabelNode_back") as! SKLabelNode!
        spriteFire = childNode(withName: "SKSpriteNode_fire") as! SKSpriteNode!
        spriteBall = childNode(withName: "SKSpriteNode_ball") as! SKSpriteNode!

        let circleSprite = childNode(withName: "SKSpriteNode_outerCircle")

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

        var frame = circleSprite?.frame
        frame?.origin = CGPoint.zero
        let path = circleInRect(frame!)
        let body = SKPhysicsBody(edgeLoopFrom: path)
        circleSprite?.physicsBody = body
        centerPos = (circleSprite?.position)!

        // accelerometer stuff
        if motionManager.isAccelerometerAvailable == true {

            // jump is very sensitive, should be as fast as possible
            motionManager.deviceMotionUpdateInterval = 1.0/120.0
            motionManager.startDeviceMotionUpdates(to: operationQueue, withHandler: {
                data, error in
                if error == nil {
                    self.userAcceleration = data!.gravity
                }
            })
        }
    }

    override func update(_ currentTime: TimeInterval) {

        let prevState = joyState;

        self.physicsWorld.gravity.dx = CGFloat(self.userAcceleration.y * GRAVITY * self.gravityFactor)
        self.physicsWorld.gravity.dy = -CGFloat(self.userAcceleration.x * GRAVITY * self.gravityFactor)

        let currentPos:CGPoint = spriteBall.position
        let adjustedPosition = CGPoint(x:currentPos.x - centerPos.x, y:currentPos.y - centerPos.y)
        var angle = 0.0
        angle = atan2(Double(adjustedPosition.y), Double(adjustedPosition.x))
        if angle < 0 {
            angle += (2 * M_PI)
        }
        angle = radiansToDegrees(angle)


        // start clean
        joyState &= ~(JoyBits.right.rawValue | JoyBits.left.rawValue | JoyBits.up.rawValue | JoyBits.down.rawValue)

        // Z and X movements are related in a pedal
        // When X has its peak, it means that Z is almost 0.
        // And when Z has its peak, it means X is almost 0, at least if the unicycle were stationary

        // userAcceleration.z (up and down) controls joy Up & Down for the unicycle game
        // userAcceleration.z > 0 == Joy down
        // userAcceleration.z < 0 == Joy up
        if (angle > (90-ANGLE_COVER) && angle < (90+ANGLE_COVER)) {
            joyState &= ~JoyBits.down.rawValue
            joyState |= JoyBits.up.rawValue
        }
        else if (angle > (270-ANGLE_COVER) && angle < (270+ANGLE_COVER)){
            joyState &= ~JoyBits.up.rawValue
            joyState |= JoyBits.down.rawValue
        }

        // userAcceleration.z y (left and right) controls joy Left & Right for the unicycle game
        // userAcceleration.z.y > 0 == Joy Right
        // userAcceleration.z.y < 0 == Joy Left
        if (angle > (360-ANGLE_COVER) || angle < 0 + ANGLE_COVER) {
            joyState &= ~JoyBits.left.rawValue
            joyState |= JoyBits.right.rawValue
        }
        else if (angle > (180-ANGLE_COVER) && angle < (180+ANGLE_COVER)) {
            joyState &= ~JoyBits.right.rawValue
            joyState |= JoyBits.left.rawValue
        }

        if joyState != prevState {
            sendJoyState();

            // update sprite colors
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
            } else if spriteFire.frame.contains(location) {
                joyState |= JoyBits.fire.rawValue
            }
        }
    }
    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        for touch in touches {
            let location = touch.location(in: self)
            if spriteFire.frame.contains(location) {
                joyState &= ~JoyBits.fire.rawValue
            }
        }
    }

}
