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
    var joyControl: UInt8 = 0

    // ip address
    let ipAddress = "10.0.0.24"
    let udpPort = 6464
    let fd = socket(AF_INET, SOCK_DGRAM, 0)     // DGRAM makes it UDP
    var toAddress = sockaddr_in()

    override func didMoveToView(view: SKView) {
        /* Setup your scene here */
        for (index, value) in self.children.enumerate() {
             print("Item \(index + 1): \(value)")
        }
        self.dpadSprite = self.childNodeWithName("SKSpriteNode_0")


        toAddress.sin_len = UInt8(sizeofValue(toAddress))
        toAddress.sin_family = sa_family_t(AF_INET)
        toAddress.sin_port = in_port_t(Int16(6464).bigEndian)

        if inet_pton(AF_INET, self.ipAddress, &toAddress.sin_addr) == 1 {
            print("initialization Ok")
        }
    }
    
    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
       /* Called when a touch begins */
        
        for touch in touches {
            let location = touch.locationInNode(self)
            print(location)

            joyState = 255
            joyControl = touch.locationInView(self.view).x < (touch.view?.frame.width)!/2 ? 0 : 1
            sendState()
        }
    }

    override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for _ in touches {
            joyState = 0
            sendState()
        }
    }
   
    override func update(currentTime: CFTimeInterval) {

        // send joy status every update since UDP doesn't have resend and it is possible
        // that some packets are lost

        sendState()
    }

    func sendState() {
        let data: [UInt8] = [joyControl, joyState]
        withUnsafePointer(&toAddress) {
            sendto(fd, data, data.count, 0, UnsafePointer($0), socklen_t(sizeofValue(toAddress)))
        }
    }
}
