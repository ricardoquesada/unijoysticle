/****************************************************************************
 http://retro.moe/unijoysticle

 Copyright 2016 Ricardo Quesada

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

class DPadScene: ControllerScene {

    var buttons: [SKSpriteNode:UInt8] = [:]
    var labelBack:SKLabelNode? = nil

    override func didMoveToView(view: SKView) {
        /* Setup your scene here */
        for (index, value) in self.children.enumerate() {
             print("Item \(index + 1): \(value)")
        }

        let names_bits = [ "SKSpriteNode_topleft": JoyBits.Up.rawValue | JoyBits.Left.rawValue,
                           "SKSpriteNode_left": JoyBits.Left.rawValue,
                           "SKSpriteNode_bottomleft": JoyBits.Down.rawValue | JoyBits.Left.rawValue,
                            "SKSpriteNode_top": JoyBits.Up.rawValue,
                            "SKSpriteNode_bottom": JoyBits.Down.rawValue,
                            "SKSpriteNode_topright": JoyBits.Up.rawValue | JoyBits.Right.rawValue,
                            "SKSpriteNode_right": JoyBits.Right.rawValue,
                            "SKSpriteNode_bottomright": JoyBits.Down.rawValue | JoyBits.Right.rawValue,
                            "SKSpriteNode_fire": JoyBits.Fire.rawValue]

        for (key,value) in names_bits {
            let sprite = childNodeWithName(key) as! SKSpriteNode!
            sprite.colorBlendFactor = 1
            sprite.color = UIColor.blackColor()
            assert(sprite != nil, "Invalid name")
            buttons[sprite] = value
        }

        labelBack = childNodeWithName("SKLabelNode_back") as! SKLabelNode?
    }

    override func touchesBegan(touches: Set<UITouch>, withEvent event: UIEvent?) {
        /* Called when a touch begins */

        for touch in touches {

            let location = touch.locationInNode(self)
            if labelBack!.frame.contains(location) {
                self.view?.window!.rootViewController?.dismissViewControllerAnimated(true, completion: {
                    // reset state to avoid having the joystick pressed
                    self.joyState = 0
                    self.sendJoyState()
                })
            }

            enableTouch(location)

            sendJoyState()
        }
    }

    override func touchesMoved(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {

            disableTouch(touch.previousLocationInNode(self))
            enableTouch(touch.locationInNode(self))

            sendJoyState()
        }
    }

    override func touchesEnded(touches: Set<UITouch>, withEvent event: UIEvent?) {
        for touch in touches {

            disableTouch(touch.previousLocationInNode(self))
            disableTouch(touch.locationInNode(self))

            sendJoyState()
        }
    }

    override func touchesCancelled(touches: Set<UITouch>?, withEvent event: UIEvent?) {
        if (touches != nil) {
            for touch in touches! {

                disableTouch(touch.previousLocationInNode(self))
                disableTouch(touch.locationInNode(self))

                sendJoyState()
            }
        }
    }


    func enableTouch(location: CGPoint) {
        for (sprite, bitmaks) in buttons {
            if sprite.frame.contains(location) {
                joyState = joyState | bitmaks
                sprite.color = UIColor.blueColor()
            }
        }
    }

    func disableTouch(location: CGPoint) {
        for (sprite, bitmaks) in buttons {
            if sprite.frame.contains(location) {
                joyState = joyState & ~bitmaks
                sprite.color = UIColor.blackColor()
            }
        }
    }
}
