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

import UIKit
import SpriteKit

class GameViewController: UIViewController {

    var selectedScene:String? = nil
    var selectedJoystick:UInt8 = 0
    var netConnection:NetworkConnection? = nil
    var userServer:String = "192.168.4.1"

    override func viewDidLoad() {
        super.viewDidLoad()

        let prefValue = NSUserDefaults.standardUserDefaults().valueForKey("ipaddress")
        if prefValue != nil {
            userServer = prefValue as! String
        }
        netConnection = NetworkConnection(serverName: userServer)
    }

    override func viewDidAppear(animated: Bool) {
        super.viewDidDisappear(animated)
        var scene:ControllerScene? = nil

        if netConnection == nil {
            let alertController = UIAlertController(title: "Invalid server", message: "Server not found: " + userServer, preferredStyle: .Alert)
            let defaultAction = UIAlertAction(title: "OK", style: .Default, handler: { uiAlertController in
                self.dismissViewControllerAnimated(false, completion: nil)
                })
            alertController.addAction(defaultAction)
            presentViewController(alertController, animated: true, completion: nil)
            return
        }

        if selectedScene == "DPadScene" {
            scene = DPadScene(fileNamed: "DPadScene")
        } else if selectedScene == "UniJoystiCleScene" {
            scene = UniGamesScene(fileNamed: "UniGamesScene")
        } else if selectedScene == "GravityScene" {
            scene = GravityScene(fileNamed: "GravityScene")
        } else if selectedScene == "LinearScene" {
            scene = LinearScene(fileNamed: "LinearScene")
        } else {
            assert(false, "Invalid scene")
        }
        
        if scene != nil {

            // don't turn off screen. usuful when in unijoysticle mode
            UIApplication.sharedApplication().idleTimerDisabled = true

            // Configure the view.
            let skView = self.view as! SKView
            skView.showsFPS = false
            skView.showsNodeCount = false
            
            /* Sprite Kit applies additional optimizations to improve rendering performance */
            if selectedScene == "GravityScene" {
                skView.ignoresSiblingOrder = false
            } else {
                skView.ignoresSiblingOrder = true
            }
            
            /* Set the scale mode to scale to fit the window */
            scene!.scaleMode = .AspectFit

            // FIXME: this should be part of the ControllerScene constructor
            // either there is a bug in SpriteKit, or I don't know how to override
            // Scene.
            scene!.joyControl = selectedJoystick
            scene!.net = netConnection
            skView.presentScene(scene)
        } else {
            print("Invalid scene")
        }
    }

    override func didReceiveMemoryWarning() {
        super.didReceiveMemoryWarning()
        // Release any cached data, images, etc that aren't in use.
    }

    override func prefersStatusBarHidden() -> Bool {
        return true
    }
}
