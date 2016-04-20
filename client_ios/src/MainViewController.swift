//
//  MainViewController.swift
//  c64controller
//
//  Created by Ricardo Quesada on 4/3/16.
//  Copyright © 2016 Ricardo Quesada. All rights reserved.
//

import UIKit

class MainViewController: UIViewController {

    @IBOutlet weak var controllerSegment: UISegmentedControl!
    @IBOutlet weak var joystickSegment: UISegmentedControl!

    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {

        if ((segue.destinationViewController as? GameViewController) != nil) {

            var sceneToLoad = "DPadScene"

            if (controllerSegment.selectedSegmentIndex == 0) {
                sceneToLoad = "DPadScene"
            } else {
                sceneToLoad = "UniJoystiCleScene"
            }

            let gameViewController = segue.destinationViewController as! GameViewController
            gameViewController.selectedScene = sceneToLoad
            gameViewController.selectedJoystick = UInt8(joystickSegment.selectedSegmentIndex)
        }
    }

    @IBAction func prepareForUnwind(segue: UIStoryboardSegue) {
    }
}
