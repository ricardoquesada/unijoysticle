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

import UIKit

class MainViewController: UIViewController {

    @IBOutlet weak var controllerSegment: UISegmentedControl!
    @IBOutlet weak var joystickSegment: UISegmentedControl!
    @IBOutlet weak var activityIndicator: UIActivityIndicatorView!

    override func viewWillAppear(_ animated: Bool) {
        super.viewWillAppear(animated)
        activityIndicator.stopAnimating()
    }

    override func prepare(for segue: UIStoryboardSegue, sender: Any?) {

        activityIndicator.startAnimating()

        // Possible identifiers:
        //  GameVC, SettingsVC, CommodoreHomeVC
        if segue.identifier == "GameVC" {

            var sceneToLoad = ""

            if (controllerSegment.selectedSegmentIndex == 0) {
                sceneToLoad = UNIJOYSTICLE_SCENE
            } else if (controllerSegment.selectedSegmentIndex == 1) {
                sceneToLoad = DPAD_SCENE
            } else if (controllerSegment.selectedSegmentIndex == 2) {
                sceneToLoad = COMMANDO_SCENE
            } else if (controllerSegment.selectedSegmentIndex == 3) {
                sceneToLoad = GYRUSS_SCENE
            }

            let gameViewController = segue.destination as! GameViewController
            gameViewController.selectedScene = sceneToLoad
            gameViewController.selectedJoystick = UInt8(joystickSegment.selectedSegmentIndex)
        }
    }

    @IBAction func startTouchUpInside(_ sender: AnyObject) {
        if (controllerSegment.selectedSegmentIndex == 4) {
            self.performSegue(withIdentifier: "CommodoreHomeVC", sender: sender)
        } else {
            self.performSegue(withIdentifier: "GameVC", sender: sender)
        }
    }

    @IBAction func controllerValueChanged(_ sender: AnyObject) {
        // In Commando and Commodore Home modes, disable Joysticks, since it will use both of them
        joystickSegment.isEnabled = !(controllerSegment.selectedSegmentIndex == 2 || controllerSegment.selectedSegmentIndex == 4)
    }
}
