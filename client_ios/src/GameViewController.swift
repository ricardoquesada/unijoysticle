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

class GameViewController: UIViewController, UIAlertViewDelegate {

    var selectedScene:String? = nil
    var selectedJoystick:UInt8 = 0

    func alertView(alertView: UIAlertView, didDismissWithButtonIndex buttonIndex: Int) {
        dismissViewControllerAnimated(true, completion: {
            print("finished")
        })
    }
    override func viewDidLoad() {
        super.viewDidLoad()
        var scene:ControllerScene? = nil

        var serverAddr = "unijoysticle.local"
        let serverValue = NSUserDefaults.standardUserDefaults().valueForKey("ipaddress")
        if serverValue != nil {
            serverAddr = serverValue as! String
        }
        let netConnection = NetworkConnection(serverName: serverAddr)
        if netConnection == nil {
            let alert = UIAlertView(title: "Invalid server", message: "Server not found: " + serverAddr, delegate: self, cancelButtonTitle: "Ok", otherButtonTitles: "")
            alert.show()
            return
        }

        if selectedScene == "DPadScene" {
            scene = DPadScene(fileNamed: "DPadScene")
        } else if selectedScene == "UniJoystiCleScene" {
            scene = UniGamesScene(fileNamed: "UniGamesScene")
        } else {
            assert(false, "Invalid scene")
        }
        
        if scene != nil {
            // Configure the view.
            let skView = self.view as! SKView
            skView.showsFPS = false
            skView.showsNodeCount = false
            
            /* Sprite Kit applies additional optimizations to improve rendering performance */
            skView.ignoresSiblingOrder = true
            
            /* Set the scale mode to scale to fit the window */
            scene!.scaleMode = .AspectFill

            // FIXME: this should be part of the ControllerScene constructor
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
