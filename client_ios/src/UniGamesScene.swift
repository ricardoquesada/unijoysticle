//
//  GameScene.swift
//  c64controller
//
//  Created by Ricardo Quesada on 3/25/16.
//  Copyright (c) 2016 Ricardo Quesada. All rights reserved.
//

import SpriteKit
import CoreMotion

class UniGamesScene: SKScene {

    // assign nodes to buttons
    enum JoyBits: UInt8 {
        case Up     = 0b00000001
        case Down   = 0b00000010
        case Left   = 0b00000100
        case Right  = 0b00001000
        case Fire   = 0b00010000
    }

    var joyState: UInt8 = 0
    var joyControl: UInt8 = 1

    // network
    let net = NetworkConnection()

    var motionManager = CMMotionManager()
    var rotationRate = CMRotationRate();

    // accel tmp
    var labelX:SKLabelNode? = nil
    var labelY:SKLabelNode? = nil
    var labelZ:SKLabelNode? = nil
    var labelState:SKLabelNode? = nil
    var labelBack:SKLabelNode? = nil
    var zMax:Double = -100
    var zMin:Double = 100
//    let accelFilter = HighpassFilter(sampleRate: 60, cutoffFrequency: 5.0)

    let operationQueue = NSOperationQueue()

    override func didMoveToView(view: SKView) {

        // accelerometer stuff
        if motionManager.accelerometerAvailable == true {

//            accelFilter.adaptive = false
            labelX = childNodeWithName("SKLabelNode_x") as! SKLabelNode?
            labelY = childNodeWithName("SKLabelNode_y") as! SKLabelNode?
            labelZ = childNodeWithName("SKLabelNode_z") as! SKLabelNode?
            labelState = childNodeWithName("SKLabelNode_state") as! SKLabelNode?
            labelBack = childNodeWithName("SKLabelNode_back") as! SKLabelNode?

//            motionManager.accelerometerUpdateInterval = 1/60
//            motionManager.startAccelerometerUpdatesToQueue(operationQueue, withHandler:{
//                data, error in
//
//                self.accelFilter.addAcceleration(data!.acceleration)
//
//                self.zMin = min(data!.acceleration.z, self.zMin)
//                self.zMax = max(data!.acceleration.z, self.zMax)
//            })

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

        // accel Z (up and down) controls joy Up & Down for the unicycle game
        // Accel.Z > 0 == Joy Up
        // Accel.Z < 0 == Joy Down
        let threshold = 0.5
        if (rotationRate.x < -threshold) {
            joyState &= ~JoyBits.Down.rawValue
            joyState |= JoyBits.Up.rawValue
        }
        else if (rotationRate.x > threshold) {
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
        if (rotationRate.z < -threshold) {
            joyState &= ~JoyBits.Left.rawValue
            joyState |= JoyBits.Right.rawValue
        }
        else if (rotationRate.z > threshold) {
            joyState &= ~JoyBits.Right.rawValue
            joyState |= JoyBits.Left.rawValue
        }
        else if (rotationRate.z > -0.05 && rotationRate.z < 0.05) {
            // not moving. turn off left and right
            joyState &= ~(JoyBits.Right.rawValue | JoyBits.Left.rawValue)
        }

        // send joy status every update since UDP doesn't have resend and it is possible
        // that some packets are lost
        net.sendState(joyControl, joyState)
    }

    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {
            let location = touch.locationInNode(self)
            if labelBack!.frame.contains(location) {
                self.view?.window!.rootViewController?.dismissViewControllerAnimated(true, completion: { 
                    print("finished")
                })
            }
        }
    }
}
