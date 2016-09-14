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
func circleInRect(rect:CGRect) -> CGPath {
    // Adjust position so path is centered in shape
    let adjustedRect = CGRectMake(rect.origin.x-rect.size.width/2, rect.origin.y-rect.size.height/2, rect.size.width, rect.size.height);
    let bezierPath = UIBezierPath(ovalInRect: adjustedRect)
    return bezierPath.CGPath;
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
    var centerPos:CGPoint = CGPointZero

    let operationQueue = NSOperationQueue()

    override func didMoveToView(view: SKView) {

        // read settings
        let settings = NSUserDefaults.standardUserDefaults()

        let gravityValue = settings.valueForKey(SETTINGS_GRAVITY_FACTOR_KEY)
        if (gravityValue != nil) {
            gravityFactor = Double(gravityValue as! Float)
        }

        // setup sprites
        labelBack = (childNodeWithName("SKLabelNode_back") as! SKLabelNode?)!
        spriteFire = (childNodeWithName("SKSpriteNode_fire") as! SKSpriteNode?)!
        spriteBall = (childNodeWithName("SKSpriteNode_ball") as! SKSpriteNode?)!

        let circleSprite = childNodeWithName("SKSpriteNode_outerCircle")

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

        var frame = circleSprite?.frame
        frame?.origin = CGPointZero
        let path = circleInRect(frame!)
        let body = SKPhysicsBody(edgeLoopFromPath: path)
        circleSprite?.physicsBody = body
        centerPos = (circleSprite?.position)!

        // accelerometer stuff
        if motionManager.accelerometerAvailable == true {

            // jump is very sensitive, should be as fast as possible
            motionManager.deviceMotionUpdateInterval = 1.0/120.0
            motionManager.startDeviceMotionUpdatesToQueue(operationQueue, withHandler: {
                data, error in
                if error == nil {
                    self.userAcceleration = data!.gravity
                }
            })
        }
    }

    override func update(currentTime: CFTimeInterval) {

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
        joyState &= ~(JoyBits.Right.rawValue | JoyBits.Left.rawValue | JoyBits.Up.rawValue | JoyBits.Down.rawValue)

        // Z and X movements are related in a pedal
        // When X has its peak, it means that Z is almost 0.
        // And when Z has its peak, it means X is almost 0, at least if the unicycle were stationary

        // userAcceleration.z (up and down) controls joy Up & Down for the unicycle game
        // userAcceleration.z > 0 == Joy down
        // userAcceleration.z < 0 == Joy up
        if (angle > (90-ANGLE_COVER) && angle < (90+ANGLE_COVER)) {
            joyState &= ~JoyBits.Down.rawValue
            joyState |= JoyBits.Up.rawValue
        }
        else if (angle > (270-ANGLE_COVER) && angle < (270+ANGLE_COVER)){
            joyState &= ~JoyBits.Up.rawValue
            joyState |= JoyBits.Down.rawValue
        }

        // userAcceleration.z y (left and right) controls joy Left & Right for the unicycle game
        // userAcceleration.z.y > 0 == Joy Right
        // userAcceleration.z.y < 0 == Joy Left
        if (angle > (360-ANGLE_COVER) || angle < 0 + ANGLE_COVER) {
            joyState &= ~JoyBits.Left.rawValue
            joyState |= JoyBits.Right.rawValue
        }
        else if (angle > (180-ANGLE_COVER) && angle < (180+ANGLE_COVER)) {
            joyState &= ~JoyBits.Right.rawValue
            joyState |= JoyBits.Left.rawValue
        }

        if joyState != prevState {
            sendJoyState();

            // update sprite colors
            for (sprite, bitmask) in buttons {
                if (joyState & bitmask) != 0 {
                    sprite.color = UIColor.redColor()
                }
                else {
                    sprite.color = UIColor.grayColor()
                }
            }
        }
    }

    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {
            let location = touch.locationInNode(self)
            if labelBack.frame.contains(location) {
                self.view!.window!.rootViewController!.dismissViewControllerAnimated(false, completion: {
                    // reset state to avoid having the joystick pressed
                    self.joyState = 0
                    self.sendJoyState()
                    
                    // re-enable it.
                    UIApplication.sharedApplication().idleTimerDisabled = false
                })
            } else if spriteFire.frame.contains(location) {
                joyState |= JoyBits.Fire.rawValue
            }
        }
    }
    override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {
            let location = touch.locationInNode(self)
            if spriteFire.frame.contains(location) {
                joyState &= ~JoyBits.Fire.rawValue
            }
        }
    }

}
