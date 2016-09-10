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
import CFNetwork

class NetworkConnection {

    static var addresses:[String:sockaddr_in] = [:]

    // ip address
    let fd = socket(AF_INET, SOCK_DGRAM, 0)     // DGRAM makes it UDP
    var toAddress = sockaddr_in()
    let serverPort:UInt16 = 6464

    init?(serverName:String = SERVER_IP_ADDRESS) {
        // Use CFHost instead of inet_pton, since we need to use mDNS
        if let addr = NetworkConnection.addresses[serverName] {
            self.toAddress = addr
        } else {
            let hostRef = CFHostCreateWithName(nil, serverName).takeRetainedValue()
            let resolved = CFHostStartInfoResolution(hostRef, CFHostInfoType.Addresses, nil)
            if resolved {
                var success: DarwinBoolean = false
                if let addresses = CFHostGetAddressing(hostRef, &success)?.takeUnretainedValue() as NSArray? {
                    let dataFirst = addresses.firstObject!
                    var addr = sockaddr()
                    dataFirst.getBytes(&addr, length:sizeof(sockaddr))
                    let addr4 = withUnsafePointer(&addr) { UnsafePointer<sockaddr_in>($0).memory }

                    self.toAddress.sin_len = UInt8(sizeofValue(self.toAddress))
                    self.toAddress.sin_family = sa_family_t(AF_INET)
                    self.toAddress.sin_addr = addr4.sin_addr
                    self.toAddress.sin_port = in_port_t(serverPort.bigEndian)

                    NetworkConnection.addresses[serverName] = self.toAddress
                } else {
                    return nil
                }
            } else {
                return nil
            }
        }
    }

    func sendState(joyControl:UInt8, _ joyState:UInt8) {
        let data: [UInt8] = [joyControl, joyState]
        withUnsafePointer(&toAddress) {
            sendto(fd, data, data.count, 0, UnsafePointer($0), socklen_t(sizeofValue(toAddress)))
        }
    }

    func sendState2(data:[UInt8]) {
        assert(data.count == 4, "Invalid data");
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
        case DPad   = 0b00001111
        case All    = 0b00011111
    }

    var net:NetworkConnection?


    var protoVersion = 1                        // default version

    // Protocol Version 1
    var joyState: UInt8 = 0
    var joyControl: UInt8 = 1                   // joystick 0 or 1

    // Protocol version 2
    struct ProtoHeader {
        var version:UInt8 = 2
        var joyControl:UInt8 = 0b00000011       // two joysticks
        var joyState1:UInt8 = 0
        var joyState2:UInt8 = 0
    }

    var protoHeader:ProtoHeader = ProtoHeader()

    func sendJoyState() {
        assert(net != nil, "net is nil")
        
        if protoVersion == 1 {
            net!.sendState(joyControl, joyState)
        } else {
            let data:[UInt8] = [protoHeader.version, protoHeader.joyControl, protoHeader.joyState1, protoHeader.joyState2]
            net!.sendState2(data)
        }
    }

    override func update(currentTime: CFTimeInterval) {
        sendJoyState()
    }
}