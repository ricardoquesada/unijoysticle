//
//  MainViewController.swift
//  c64controller
//
//  Created by Ricardo Quesada on 4/3/16.
//  Copyright © 2016 Ricardo Quesada. All rights reserved.
//

import UIKit

class MainViewController: UIViewController {

    @IBOutlet weak var segmentControl: UISegmentedControl!

    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {

        if ((segue.destinationViewController as? GameViewController) != nil) {

            var sceneToLoad = "DPadScene"

            if (segmentControl.selectedSegmentIndex == 0) {
                sceneToLoad = "DPadScene"
            } else {
                sceneToLoad = "UniGamesScene"
            }

            (segue.destinationViewController as! GameViewController).selectedScene = sceneToLoad
        }
    }

    @IBAction func prepareForUnwind(segue: UIStoryboardSegue) {
    }
}
