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

class MainViewController: UIViewController {

    @IBOutlet weak var controllerSegment: UISegmentedControl!
    @IBOutlet weak var joystickSegment: UISegmentedControl!

    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {

        if ((segue.destinationViewController as? GameViewController) != nil) {

            var sceneToLoad = "DPadScene"

            if (controllerSegment.selectedSegmentIndex == 0) {
                sceneToLoad = "DPadScene"
            } else if (controllerSegment.selectedSegmentIndex == 1) {
                sceneToLoad = "UniJoystiCleScene"
            } else if (controllerSegment.selectedSegmentIndex == 2) {
                sceneToLoad = "GravityScene"
            } else if (controllerSegment.selectedSegmentIndex == 3) {
                sceneToLoad = "LinearScene"
            }

            let gameViewController = segue.destinationViewController as! GameViewController
            gameViewController.selectedScene = sceneToLoad
            gameViewController.selectedJoystick = UInt8(joystickSegment.selectedSegmentIndex)
        }
    }

    @IBAction func prepareForUnwind(segue: UIStoryboardSegue) {
    }
}
