/*
 Copyright (C) 2011 by Stuart Carnie
 
 Ported from Objective-C to Swift by Ricardo Quesada

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

import UIKit

enum iCadeButtons:UInt16 {

    case JoystickNone       = 0b00000000
    case JoystickUp         = 0b00000001
    case JoystickDown       = 0b00000010
    case JoystickLeft       = 0b00000100
    case JoystickRight      = 0b00001000

    case JoystickUpRight    = 0b00001001
    case JoystickDownRight  = 0b00001010
    case JoystickUpLeft     = 0b00000101
    case JoystickDownLeft   = 0b00000110

    case ButtonA            = 0b00010000
    case ButtonB            = 0b00100000
    case ButtonC            = 0b01000000
    case ButtonD            = 0b10000000
    case ButtonE            = 0b100000000
    case ButtonF            = 0b1000000000
    case ButtonG            = 0b10000000000
    case ButtonH            = 0b100000000000
}

protocol iCadeEventDelegate {
    func stateChanged(state:UInt16) -> Void
    func buttonDown(state:UInt16) -> Void
    func buttonUp(state:UInt16) -> Void
}

class iCadeReaderView: UIView, UIKeyInput {

    /*
     Ordered to make it compatible with c64 joystick

     UP ON,OFF  = w,e
     DN ON,OFF  = x,z
     LT ON,OFF  = a,q
     RT ON,OFF  = d,c
     A  ON,OFF  = y,t

     B  ON,OFF  = h,r
     C  ON,OFF  = u,f
     D  ON,OFF  = j,n
     E  ON,OFF  = i,m
     F  ON,OFF  = k,p
     G  ON,OFF  = o,g
     H  ON,OFF  = l,v
     */

    // FIXME: String should be more optimal in theory, but the API is confusing.
    // Let's keep it simple with Arrays
    static let ON_STATES:Array  = ["w","x","a","d","y","h","u","j","i","k","o","l"]
    static let OFF_STATES:Array = ["e","z","q","c","t","r","f","n","m","p","g","v"]

    static var cycleResponder:Int = 0

    var delegate:iCadeEventDelegate? = nil

    var newInputView:UIView!
    override var inputView: UIView? {
        get {
            return newInputView
        }
    }

    var active:Bool = true {
        didSet {
            if (active) {
                becomeFirstResponder()
            } else {
                resignFirstResponder()
            }
        }
    }
    var joyState:UInt16 = 0

    override init (frame: CGRect) {
        super.init(frame: frame)
        newInputView = UIView(frame: CGRect.zero)
        let notiCenter = NSNotificationCenter.defaultCenter()
        notiCenter.addObserver(self, selector: #selector(iCadeReaderView.didEnterBackground), name: UIApplicationDidEnterBackgroundNotification, object: nil)
        notiCenter.addObserver(self, selector: #selector(iCadeReaderView.didBecomeActive), name: UIApplicationDidBecomeActiveNotification, object: nil)
    }

    convenience init () {
        self.init(frame:CGRect.zero)
    }

    required init(coder aDecoder: NSCoder) {
        fatalError("This class does not support NSCoding")
    }

    func didEnterBackground() -> Void {
        if active {
            resignFirstResponder()
        }
    }

    func didBecomeActive() -> Void {
        if active {
            becomeFirstResponder()
        }
    }

    override func canBecomeFirstResponder() -> Bool {
        return true
    }

    //
    // UIKeyInput protocol implementation
    //
    func hasText() -> Bool {
        return false
    }

    func insertText(text: String) {
        var stateChanged = false
        let indexOn = iCadeReaderView.ON_STATES.indexOf(text)
        if (indexOn != nil) {
            joyState |= (UInt16)(1 << indexOn!)
            stateChanged = true;
            if delegate != nil {
                delegate!.buttonDown((UInt16)(1 << indexOn!))
            }
        } else {
            let indexOff = iCadeReaderView.OFF_STATES.indexOf(text)
            if (indexOff != nil) {
                joyState &= ~(UInt16)(1 << indexOff!)
                stateChanged = true;
                if delegate != nil {
                    delegate!.buttonUp((UInt16)(1 << indexOff!))
                }
            }
        }

        if stateChanged && delegate != nil {
            delegate!.stateChanged(joyState)
        }

        iCadeReaderView.cycleResponder += 1
        if (iCadeReaderView.cycleResponder > 20) {
            // necessary to clear a buffer that accumulates internally
            iCadeReaderView.cycleResponder = 0;
            resignFirstResponder();
            becomeFirstResponder();
        }
    }

    func deleteBackward() {
        // This space intentionally left blank to complete protocol
    }
}