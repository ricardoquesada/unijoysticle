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

class CommodoreHomeViewController: UITableViewController, UIPickerViewDelegate, UIPickerViewDataSource {

    @IBOutlet weak var musicPickerView: UIPickerView!
    @IBOutlet weak var dimmerSlider: UISlider!
    @IBOutlet weak var alertSwitch: UISwitch!

    override func viewDidLoad() {
        super.viewDidLoad()

        musicPickerView.delegate = self
        musicPickerView.dataSource = self
    }

    @IBAction func alertValueChanged(sender: AnyObject) {
    }
    @IBAction func dimmerValueChanged(sender: AnyObject) {
    }

    // UIPickerView Data Source Protocol
    func numberOfComponentsInPickerView(pickerView: UIPickerView) -> Int {
        return 8
    }

    // returns the # of rows in each component..
    func pickerView(pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        return 0
    }
}
