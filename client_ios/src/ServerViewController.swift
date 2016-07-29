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

class ServerViewController: UIViewController, UITextFieldDelegate {

    @IBOutlet weak var ipaddress: UITextField!
    @IBOutlet weak var handicap: UISlider!
    @IBOutlet weak var handicapLabel: UILabel!

    let sliderStep:Float = 0.1

    override func viewDidLoad() {
        super.viewDidLoad()
        let settings = NSUserDefaults.standardUserDefaults()
        let addr = settings.valueForKey("ipaddress")
        if (addr != nil) {
            ipaddress.text = addr as! String?
        }
        let handicapValue = settings.valueForKey("handicap")
        var handiFloat:Float = 1.0
        if (handicapValue != nil) {
            handiFloat = handicapValue as! Float
        }
        handicap.setValue(handiFloat, animated: false)
        handicapLabel.text = "\(handiFloat)"
    }

    func textFieldShouldReturn(textField: UITextField) -> Bool {
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(textField.text, forKey: "ipaddress")
        textField.resignFirstResponder()
        return true
    }

    @IBAction func sliderValueChanged(sender: UISlider) {
        let steppedValue = round(handicap.value / sliderStep) * sliderStep
        handicapLabel.text = "\(steppedValue)"
        handicap.value = steppedValue
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(steppedValue, forKey: "handicap")
    }
}