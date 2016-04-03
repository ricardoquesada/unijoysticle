//
//  GameScene.swift
//  c64controller
//
//  Created by Ricardo Quesada on 3/25/16.
//  Copyright (c) 2016 Ricardo Quesada. All rights reserved.
//

import SpriteKit
import CoreMotion


class GameScene: SKScene {

    // assign nodes to buttons
    enum JoyBits: UInt8 {
        case Top    = 0b00000001
        case Bottom = 0b00000010
        case Left   = 0b00000100
        case Right  = 0b00001000
        case Fire   = 0b00010000
    }

    var joyState: UInt8 = 0
    var joyControl: UInt8 = 1

    // ip address
    let ipAddress = "10.0.0.24"
    let udpPort = 6464
    let fd = socket(AF_INET, SOCK_DGRAM, 0)     // DGRAM makes it UDP
    var toAddress = sockaddr_in()
    var buttons: [SKNode:UInt8] = [:]
    var motionManager = CMMotionManager()

    // accel tmp
    var labelX:SKLabelNode? = nil
    var labelY:SKLabelNode? = nil
    var labelZ:SKLabelNode? = nil
    var zMax:Double = -100
    var zMin:Double = 100
    let accelFilter = HighpassFilter(sampleRate: 60, cutoffFrequency: 5.0)

    let operationQueue = NSOperationQueue()

    override func didMoveToView(view: SKView) {
        /* Setup your scene here */
        for (index, value) in self.children.enumerate() {
             print("Item \(index + 1): \(value)")
        }

        // setup ip address
        toAddress.sin_len = UInt8(sizeofValue(toAddress))
        toAddress.sin_family = sa_family_t(AF_INET)
        toAddress.sin_port = in_port_t(Int16(6464).bigEndian)

        if inet_pton(AF_INET, self.ipAddress, &toAddress.sin_addr) == 1 {
            print("initialization Ok")
        }

        let names_bits = [ "SKSpriteNode_topleft": JoyBits.Top.rawValue | JoyBits.Left.rawValue,
                           "SKSpriteNode_left": JoyBits.Left.rawValue,
                           "SKSpriteNode_bottomleft": JoyBits.Bottom.rawValue | JoyBits.Left.rawValue,
                            "SKSpriteNode_top": JoyBits.Top.rawValue,
                            "SKSpriteNode_bottom": JoyBits.Bottom.rawValue,
                            "SKSpriteNode_topright": JoyBits.Top.rawValue | JoyBits.Right.rawValue,
                            "SKSpriteNode_right": JoyBits.Right.rawValue,
                            "SKSpriteNode_bottomright": JoyBits.Bottom.rawValue | JoyBits.Right.rawValue,
                            "SKSpriteNode_fire": JoyBits.Fire.rawValue]

        for (key,value) in names_bits {
            let node = childNodeWithName(key)
            assert(node != nil, "Invalid name")
            buttons[node!] = value
        }

        // accelerometer stuff
        if motionManager.accelerometerAvailable == true {

            accelFilter.adaptive = true
            labelX = childNodeWithName("SKLabelNode_x") as! SKLabelNode?
            labelY = childNodeWithName("SKLabelNode_y") as! SKLabelNode?
            labelZ = childNodeWithName("SKLabelNode_z") as! SKLabelNode?

            motionManager.accelerometerUpdateInterval = 1/60
            motionManager.startAccelerometerUpdatesToQueue(operationQueue, withHandler:{
                data, error in

                self.accelFilter.addAcceleration(data!.acceleration)

                self.zMin = min(data!.acceleration.z, self.zMin)
                self.zMax = max(data!.acceleration.z, self.zMax)
            })
            
        }
    }

    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
        /* Called when a touch begins */

        for touch in touches {

            enableTouch(touch.locationInNode(self))

            sendState()
        }
    }

    override func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {

            disableTouch(touch.previousLocationInNode(self))
            enableTouch(touch.locationInNode(self))

            sendState()
        }
    }

    override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {

            disableTouch(touch.previousLocationInNode(self))
            disableTouch(touch.locationInNode(self))

            sendState()
        }
    }

    override func touchesCancelled(touches: Set<UITouch>?, withEvent event: UIEvent?) {
        if (touches != nil) {
            for touch in touches! {

                disableTouch(touch.previousLocationInNode(self))
                disableTouch(touch.locationInNode(self))

                sendState()
            }
        }
    }

    override func update(currentTime: CFTimeInterval) {

        self.labelX!.text = String(format:"x = %.2f", accelFilter.x)
        self.labelY!.text = String(format:"y = %.2f", accelFilter.y)
        self.labelZ!.text = String(format:"z = %.2f", accelFilter.z)

        print("min: \(zMin), max: \(zMax)")

        // accel Z (up and down) controls joy left & right for the unicycle game
        // Accel.Z > 0 == Joy Left
        // Accel.Z < 0 == Joy Right
        let threshold = 0.25
        if (joyState & JoyBits.Left.rawValue == JoyBits.Left.rawValue) && (accelFilter.z < -threshold) {
            joyState &= ~JoyBits.Left.rawValue
            joyState |= JoyBits.Right.rawValue
        }
        else if (joyState & JoyBits.Right.rawValue == JoyBits.Left.rawValue) && (accelFilter.z > threshold) {
            joyState &= ~JoyBits.Right.rawValue
            joyState |= JoyBits.Left.rawValue
        }

        // send joy status every update since UDP doesn't have resend and it is possible
        // that some packets are lost
        sendState()
    }

    func enableTouch(location: CGPoint) {
        for (node, bitmaks) in buttons {
            if node.frame.contains(location) {
                joyState = joyState | bitmaks
            }
        }
    }

    func disableTouch(location: CGPoint) {
        for (node, bitmaks) in buttons {
            if node.frame.contains(location) {
                joyState = joyState & ~bitmaks
            }
        }
    }

    func sendState() {
        let data: [UInt8] = [joyControl, joyState]
        withUnsafePointer(&toAddress) {
            sendto(fd, data, data.count, 0, UnsafePointer($0), socklen_t(sizeofValue(toAddress)))
        }
    }
}
