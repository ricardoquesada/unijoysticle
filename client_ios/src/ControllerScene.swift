//
//  ControllerScene.swift
//  UniJoystiCle controller
//
//  Created by Ricardo Quesada on 4/20/16.
//  Copyright © 2016 Ricardo Quesada. All rights reserved.
//

import SpriteKit
import CFNetwork

class NetworkConnection {
    // ip address
    let fd = socket(AF_INET, SOCK_DGRAM, 0)     // DGRAM makes it UDP
    var toAddress = sockaddr_in()
    let serverPort:UInt16 = 6464

    init?(ipAddress:String = "unijoysticle.local") {
        // setup ip address
        toAddress.sin_len = UInt8(sizeofValue(toAddress))
        toAddress.sin_family = sa_family_t(AF_INET)
        toAddress.sin_port = in_port_t(serverPort.bigEndian)

//        if inet_pton(AF_INET, ipAddress, &toAddress.sin_addr) == 1 {
//            print("initialization Ok")
//        } else {
        let hostRef = CFHostCreateWithName(nil, ipAddress).takeRetainedValue()
        let resolved = CFHostStartInfoResolution(hostRef, CFHostInfoType.Addresses, nil)
        if resolved {
            var success: DarwinBoolean = false
            if let addresses = CFHostGetAddressing(hostRef, &success)?.takeUnretainedValue() as NSArray? {
                let dataFirst = addresses.firstObject!
                var addr = sockaddr()
                dataFirst.getBytes(&addr, length:sizeof(sockaddr))
                let addr4 = withUnsafePointer(&addr) { UnsafePointer<sockaddr_in>($0).memory }
                toAddress.sin_addr = addr4.sin_addr
            } else {
                return nil
            }
        } else {
            return nil
        }
    }

    func sendState(joyControl:UInt8, _ joyState:UInt8) {
        let data: [UInt8] = [joyControl, joyState]
        withUnsafePointer(&toAddress) {
            sendto(fd, data, data.count, 0, UnsafePointer($0), socklen_t(sizeofValue(toAddress)))
        }
    }
}

class ControllerScene: SKScene {

    // assign nodes to buttons
    enum JoyBits: UInt8 {
        case Up     = 0b00000001
        case Down   = 0b00000010
        case Left   = 0b00000100
        case Right  = 0b00001000
        case Fire   = 0b00010000
    }

    var joyState: UInt8 = 0
    var joyControl: UInt8 = 1   // joystick 0 or 1
    var net:NetworkConnection?

    func sendJoyState() {
        assert(net != nil, "net is nil")
        net!.sendState(joyControl, joyState)
    }

    override func update(currentTime: CFTimeInterval) {
        sendJoyState()
    }
}