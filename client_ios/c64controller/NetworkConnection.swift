//
//  NetworkConnection.swift
//  c64controller
//
//  Created by Ricardo Quesada on 4/3/16.
//  Copyright © 2016 Ricardo Quesada. All rights reserved.
//

import Foundation

class NetworkConnection {
    // ip address
    let fd = socket(AF_INET, SOCK_DGRAM, 0)     // DGRAM makes it UDP
    var toAddress = sockaddr_in()

    init(ipAddress:String, port:UInt16) {
        // setup ip address
        toAddress.sin_len = UInt8(sizeofValue(toAddress))
        toAddress.sin_family = sa_family_t(AF_INET)
        toAddress.sin_port = in_port_t(port.bigEndian)

        if inet_pton(AF_INET, ipAddress, &toAddress.sin_addr) == 1 {
            print("initialization Ok")
        }
    }

    func sendState(joyControl:UInt8, _ joyState:UInt8) {
        let data: [UInt8] = [joyControl, joyState]
        withUnsafePointer(&toAddress) {
            sendto(fd, data, data.count, 0, UnsafePointer($0), socklen_t(sizeofValue(toAddress)))
        }
    }
}