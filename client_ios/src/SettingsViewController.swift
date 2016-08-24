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

class SettingsViewController: UITableViewController, UITextFieldDelegate {

    @IBOutlet weak var ipaddress: UITextField!

    @IBOutlet weak var handicapSlider: UISlider!
    @IBOutlet weak var handicapLabel: UILabel!

    @IBOutlet weak var jumpSlider: UISlider!
    @IBOutlet weak var jumpLabel: UILabel!

    @IBOutlet weak var movementSlider: UISlider!
    @IBOutlet weak var movementLabel: UILabel!

    @IBOutlet weak var gravityLabel: UILabel!
    @IBOutlet weak var gravitySlider: UISlider!

    @IBOutlet weak var buttonBSwitch: UISwitch!
    @IBOutlet weak var swapABSwitch: UISwitch!

    let sliderStep:Float = 0.1

    override func viewDidLoad() {
        super.viewDidLoad()
        let settings = NSUserDefaults.standardUserDefaults()

        // ip address
        let addr = settings.valueForKey(SETTINGS_IP_ADDRESS_KEY)
        if (addr != nil) {
            ipaddress.text = addr as! String?
        }
        else {
            ipaddress.text = SERVER_IP_ADDRESS
        }

        // rotation rate
        let handicapValue = settings.valueForKey(SETTINGS_ROTATION_RATE_KEY)
        var handiFloat = Float(ROTATION_RATE)
        if (handicapValue != nil) {
            handiFloat = handicapValue as! Float
        }
        handicapSlider.setValue(handiFloat, animated: false)
        handicapLabel.text = "\(handiFloat)"

        // movement threshold
        let movementValue = settings.valueForKey(SETTINGS_MOVEMENT_THRESHOLD_KEY)
        var movementFloat = Float(MOVEMENT_THRESHOLD)
        if (movementValue != nil) {
            movementFloat = movementValue as! Float
        }
        movementSlider.setValue(movementFloat, animated: false)
        movementLabel.text = "\(movementFloat)"

        // jump threshold
        let jumpValue = settings.valueForKey(SETTINGS_JUMP_THRESHOLD_KEY)
        var jumpFloat = Float(JUMP_THRESHOLD)
        if (jumpValue != nil) {
            jumpFloat = jumpValue as! Float
        }
        jumpSlider.setValue(jumpFloat, animated: false)
        jumpLabel.text = "\(jumpFloat)"

        // gravity
        let gravityValue = settings.valueForKey(SETTINGS_GRAVITY_FACTOR_KEY)
        var gravityFloat = Float(GRAVITY_FACTOR)
        if (gravityValue != nil) {
            gravityFloat = gravityValue as! Float
        }
        gravitySlider.setValue(gravityFloat, animated: false)
        gravityLabel.text = "\(gravityFloat)"

        // button B
        let buttonBValue = settings.valueForKey(SETTINGS_BUTTON_B_KEY)
        var buttonBBool = BUTTON_B_ENABLED
        if (buttonBValue != nil) {
            buttonBBool = buttonBValue as! Bool
        }
        buttonBSwitch.setOn(buttonBBool, animated: false)

        // swap AB
        let swapABValue = settings.valueForKey(SETTINGS_SWAP_A_B_KEY)
        var swapABBool = SWAP_A_B_ENABLED
        if (swapABValue != nil) {
            swapABBool = swapABValue as! Bool
        }
        swapABSwitch.setOn(swapABBool, animated: false)
    }

    func textFieldShouldReturn(textField: UITextField) -> Bool {
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(textField.text, forKey: SETTINGS_IP_ADDRESS_KEY)
        textField.resignFirstResponder()
        return true
    }

    @IBAction func sliderValueChanged(sender: UISlider) {
        let steppedValue = round(handicapSlider.value / sliderStep) * sliderStep
        handicapLabel.text = "\(steppedValue)"
        handicapSlider.value = steppedValue
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(steppedValue, forKey: SETTINGS_ROTATION_RATE_KEY)
    }

    @IBAction func movementValueChanged(sender: AnyObject) {
        let steppedValue = round(movementSlider.value / sliderStep) * sliderStep
        movementLabel.text = "\(steppedValue)"
        movementSlider.value = steppedValue
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(steppedValue, forKey: SETTINGS_MOVEMENT_THRESHOLD_KEY)
    }
    @IBAction func jumpValueChanged(sender: AnyObject) {
        let steppedValue = round(jumpSlider.value / sliderStep) * sliderStep
        jumpLabel.text = "\(steppedValue)"
        jumpSlider.value = steppedValue
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(steppedValue, forKey: SETTINGS_JUMP_THRESHOLD_KEY)
    }

    @IBAction func gravityValueChanged(sender: AnyObject) {
        let steppedValue = round(gravitySlider.value / 0.5) * 0.5
        gravityLabel.text = "\(steppedValue)"
        gravitySlider.value = steppedValue
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(steppedValue, forKey: SETTINGS_GRAVITY_FACTOR_KEY)
    }

    @IBAction func buttonBValueChanged(sender: AnyObject) {
        let value = buttonBSwitch.on
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(value, forKey: SETTINGS_BUTTON_B_KEY)
        swapABSwitch.enabled = value;
    }

    @IBAction func swapABValueChanged(sender: AnyObject) {
        let value = swapABSwitch.on
        let settings = NSUserDefaults.standardUserDefaults()
        settings.setValue(value, forKey: SETTINGS_SWAP_A_B_KEY)
    }
}