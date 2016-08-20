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
import UIKit

class LinearScene: ControllerScene {

    // accel tmp
    var buttons: [SKSpriteNode:UInt8] = [:]

    var labelBack:SKLabelNode? = nil
    var labelSlider:SKLabelNode? = nil

    // will be used to calculate the angle. this will be used as the "center"
    var centerPos:CGPoint = CGPointZero
    let slider:UISlider = UISlider(frame: CGRect(x: 100, y: 100, width: 300, height: 20))

    override func didMoveToView(view: SKView) {
        // center slider according to view size
        var viewSize = view.frame.size
        viewSize.width = viewSize.width / 2 - 150
        viewSize.height = viewSize.height / 2 + 40
        let oldFrame = slider.frame
        slider.frame = CGRect(x:viewSize.width, y:viewSize.height, width:oldFrame.size.width, height:oldFrame.size.height)


        // slider (using UIKit one)
        self.view?.addSubview(slider)
        slider.minimumValue = 0
        slider.maximumValue = 31      // 2 ^ 5 - 1
        slider.continuous = true
        slider.setValue(0, animated: false)
        slider.addTarget(self, action: #selector(LinearScene.valueChanged(_:)), forControlEvents: .ValueChanged)


        // setup sprites
        labelBack = childNodeWithName("SKLabelNode_back") as! SKLabelNode?
        labelSlider = childNodeWithName("SKLabelNode_value") as! SKLabelNode?

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
    }

    func valueChanged(sender: UISlider) {
        // round the slider position to the nearest index of the numbers array
        let index = (Int)(slider.value)
        slider.setValue(Float(index), animated: false)
        labelSlider?.text = "\(index)"

        joyState = UInt8(index)
    }

    override func update(currentTime: CFTimeInterval) {

        for (sprite, bitmask) in buttons {
            if (joyState & bitmask) != 0 {
                sprite.color = UIColor.redColor()
            }
            else {
                sprite.color = UIColor.grayColor()
            }
        }

        // send joy status every update since UDP doesn't have resend and it is possible
        // that some packets are lost
        super.update(currentTime)
    }

    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {
            let location = touch.locationInNode(self)
            if labelBack!.frame.contains(location) {
                self.view!.window!.rootViewController!.dismissViewControllerAnimated(false, completion: {
                    // reset state to avoid having the joystick pressed
                    self.joyState = 0
                    self.sendJoyState()

                    // re-enable it.
                    UIApplication.sharedApplication().idleTimerDisabled = false
                })
            }
        }
    }

}
