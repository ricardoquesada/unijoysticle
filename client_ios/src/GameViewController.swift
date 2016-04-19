//
//  GameViewController.swift
//  c64controller
//
//  Created by Ricardo Quesada on 3/25/16.
//  Copyright (c) 2016 Ricardo Quesada. All rights reserved.
//

import UIKit
import SpriteKit

class GameViewController: UIViewController {

    var selectedScene:String? = nil

    override func viewDidLoad() {
        super.viewDidLoad()
        var scene:SKScene? = nil

        if selectedScene == "DPadScene" {
            scene = DPadScene(fileNamed: "DPadScene")
        } else {
            scene = UniGamesScene(fileNamed: "UniGamesScene")
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
