//
//  GameScene.swift
//  c64controller
//
//  Created by Ricardo Quesada on 3/25/16.
//  Copyright (c) 2016 Ricardo Quesada. All rights reserved.
//

import SpriteKit

class DPadScene: SKScene {

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

    var buttons: [SKNode:UInt8] = [:]

    var labelBack:SKLabelNode? = nil

    // network
    let net = NetworkConnection(ipAddress: "10.0.1.4", port: 6464)

    override func didMoveToView(view: SKView) {
        /* Setup your scene here */
        for (index, value) in self.children.enumerate() {
             print("Item \(index + 1): \(value)")
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

        labelBack = childNodeWithName("SKLabelNode_back") as! SKLabelNode?
    }

    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
        /* Called when a touch begins */

        for touch in touches {

            let location = touch.locationInNode(self)
            if labelBack!.frame.contains(location) {
                self.view?.window!.rootViewController?.dismissViewControllerAnimated(true, completion: {
                    print("finished")
                })
            }


            enableTouch(touch.locationInNode(self))

            net.sendState(joyControl, joyState)
        }
    }

    override func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {

            disableTouch(touch.previousLocationInNode(self))
            enableTouch(touch.locationInNode(self))

            net.sendState(joyControl, joyState)
        }
    }

    override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {

            disableTouch(touch.previousLocationInNode(self))
            disableTouch(touch.locationInNode(self))

            net.sendState(joyControl, joyState)
        }
    }

    override func touchesCancelled(touches: Set<UITouch>?, withEvent event: UIEvent?) {
        if (touches != nil) {
            for touch in touches! {

                disableTouch(touch.previousLocationInNode(self))
                disableTouch(touch.locationInNode(self))

                net.sendState(joyControl, joyState)
            }
        }
    }

    override func update(currentTime: CFTimeInterval) {

        // send joy status every update since UDP doesn't have resend and it is possible
        // that some packets are lost
        net.sendState(joyControl, joyState)
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
}
