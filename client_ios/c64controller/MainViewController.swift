//
//  MainViewController.swift
//  c64controller
//
//  Created by Ricardo Quesada on 4/3/16.
//  Copyright © 2016 Ricardo Quesada. All rights reserved.
//

import UIKit

class MainViewController: UIViewController {

    override func prepareForSegue(segue: UIStoryboardSegue, sender: AnyObject?) {

        var sceneToLoad = "DPadScene"
        let segment:UISegmentedControl = view.viewWithTag(10) as! UISegmentedControl

        if (segment.selectedSegmentIndex == 0) {
            sceneToLoad = "DPadScene"
        } else {
            sceneToLoad = "UniGamesScene"
        }

        (segue.destinationViewController as! GameViewController).selectedScene = sceneToLoad
    }
}
