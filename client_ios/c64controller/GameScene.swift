//
//  GameScene.swift
//  c64controller
//
//  Created by Ricardo Quesada on 3/25/16.
//  Copyright (c) 2016 Ricardo Quesada. All rights reserved.
//

import SpriteKit

class GameScene: SKScene {

    var dpadSprite: SKNode?
    var joyState: UInt8 = 0
    var joyControl: UInt8 = 1

    // ip address
    let ipAddress = "10.0.0.24"
    let udpPort = 6464
    let fd = socket(AF_INET, SOCK_DGRAM, 0)     // DGRAM makes it UDP
    var toAddress = sockaddr_in()
    var buttons: [SKNode:UInt8] = [:]

    enum JoyBits: UInt8 {
        case Top    = 0b00000001
        case Bottom = 0b00000010
        case Left   = 0b00000100
        case Right  = 0b00001000
        case Fire   = 0b00010000
    }

    override func didMoveToView(view: SKView) {
        /* Setup your scene here */
        for (index, value) in self.children.enumerate() {
             print("Item \(index + 1): \(value)")
        }

        toAddress.sin_len = UInt8(sizeofValue(toAddress))
        toAddress.sin_family = sa_family_t(AF_INET)
        toAddress.sin_port = in_port_t(Int16(6464).bigEndian)

        if inet_pton(AF_INET, self.ipAddress, &toAddress.sin_addr) == 1 {
            print("initialization Ok")
        }

        // left column
        let topleft = childNodeWithName("SKSpriteNode_topleft")
        assert(topleft != nil, "invalid topleft")
        buttons[topleft!] = JoyBits.Top.rawValue | JoyBits.Left.rawValue

        let left = childNodeWithName("SKSpriteNode_left")
        assert(left != nil, "invalid left")
        buttons[left!] =  JoyBits.Left.rawValue

        let bottomleft = childNodeWithName("SKSpriteNode_bottomleft")
        assert(bottomleft != nil, "invalid bottomleft")
        buttons[bottomleft!] =  JoyBits.Bottom.rawValue | JoyBits.Left.rawValue

        // center column
        let top = childNodeWithName("SKSpriteNode_top")
        assert(top != nil, "invalid top")
        buttons[top!] = JoyBits.Top.rawValue

        let bottom = childNodeWithName("SKSpriteNode_bottom")
        assert(bottom != nil, "invalid bottom")
        buttons[bottom!] =  JoyBits.Bottom.rawValue

        // right column
        let topright = childNodeWithName("SKSpriteNode_topright")
        assert(topright != nil, "invalid topright")
        buttons[topright!] = JoyBits.Top.rawValue | JoyBits.Right.rawValue

        let right = childNodeWithName("SKSpriteNode_right")
        assert(right != nil, "invalid right")
        buttons[right!] =  JoyBits.Right.rawValue

        let bottomright = childNodeWithName("SKSpriteNode_bottomright")
        assert(bottomright != nil, "invalid bottomright")
        buttons[bottomright!] =  JoyBits.Bottom.rawValue | JoyBits.Right.rawValue

        // fire
        let fire = childNodeWithName("SKSpriteNode_fire")
        assert(fire != nil, "invalid fire")
        buttons[fire!] =  JoyBits.Fire.rawValue
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
