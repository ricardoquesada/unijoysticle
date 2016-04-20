//
//  ServerViewController.swift
//  UniJoystiCle controller
//
//  Created by Ricardo Quesada on 4/19/16.
//  Copyright © 2016 Ricardo Quesada. All rights reserved.
//

import UIKit

class ServerViewController: UIViewController, UITextFieldDelegate {

    @IBOutlet weak var ipaddress: UITextField!

    override func viewDidLoad() {
        super.viewDidLoad()
        let settings = NSUserDefaults.standardUserDefaults()
        let addr = settings.valueForKey("ipaddress")
        if (addr != nil) {
            ipaddress.text = addr as! String?
        }
    }

    func textFieldShouldReturn(textField: UITextField) -> Bool {
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(textField.text, forKey: "ipaddress")
        textField.resignFirstResponder()
        return true
    }
}