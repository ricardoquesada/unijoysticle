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

    let pickerData: [String] = ["Ashes to Ashes",
                                "Final Countdown",
                                "Pop Goes the World",
                                "Jump",
                                "Enola Gay",
                                "Billie Jean",
                                "Another Day in Paradise",
                                "Wind of Change"]

    enum HomeCommands: UInt8 {
        case nothing = 0
        case song0 = 1
        case song1 = 2
        case song2 = 3
        case song3 = 4
        case song4 = 5
        case song5 = 6
        case song6 = 7
        case song7 = 8
        case songStop = 9
        case songPlay = 10
        case songPause = 11
        case songResume = 12
        case songNext = 13
        case songPrev = 14
        case dimmer0 = 15
        case dimmer25 = 16
        case dimmer50 = 17
        case dimmer75 = 18
        case dimmer100 = 19
        case alarmOff = 20
        case alarmOn = 21
        case reserved0 = 22
        case reserved1 = 23
        case reserved2 = 24
        case reserved3 = 25
        case reserved4 = 26
        case reserved5 = 27
        case reserved6 = 28
        case reserved7 = 29
        case reserved8 = 30
        case reserved9 = 31
    }
    var dimmerLastValue:Float = 0
    var netConnection:NetworkConnection? = nil
    var userServer:String = SERVER_IP_ADDRESS

    @IBOutlet weak var dimmerLabel: UILabel!
    @IBOutlet weak var musicPickerView: UIPickerView!
    @IBOutlet weak var dimmerSlider: UISlider!
    @IBOutlet weak var alertSwitch: UISwitch!

    override func viewDidLoad() {
        super.viewDidLoad()

        musicPickerView.delegate = self
        musicPickerView.dataSource = self

        let prefValue = UserDefaults.standard.value(forKey: SETTINGS_IP_ADDRESS_KEY)
        if prefValue != nil {
            userServer = prefValue as! String
        }
        netConnection = NetworkConnection(serverName: userServer)
    }

    override func viewDidAppear(_ animated: Bool) {
        super.viewDidAppear(animated)

        if netConnection == nil {
            let alertController = UIAlertController(title: "Invalid server", message: "Server not found: " + userServer, preferredStyle: .alert)
            let defaultAction = UIAlertAction(title: "OK", style: .default, handler: { uiAlertController in
                _ = self.navigationController?.popViewController(animated: true)
            })
            alertController.addAction(defaultAction)
            present(alertController, animated: true, completion: nil)
            return
        } else {
        }
    }

    func sendJoyState(_ joy2Value:UInt8) {
        for _ in 1...2 {
            let version:UInt8 = 2     // should be 2
            let joysticks:UInt8 = 3    // 1 and 2
            let data:[UInt8] = [version, joysticks, 0, joy2Value]
            netConnection!.sendState2(data)
        }
    }

    @IBAction func alertValueChanged(_ sender: AnyObject) {
        print("\(alertSwitch.isOn)")
        if alertSwitch.isOn {
            sendJoyState(HomeCommands.alarmOn.rawValue)
        } else {
            sendJoyState(HomeCommands.alarmOff.rawValue)
        }
    }
    @IBAction func dimmerValueChanged(_ sender: AnyObject) {
        // step: 25
        let steppedValue = round(dimmerSlider.value / 25) * 25
        dimmerLabel.text = "\(steppedValue)"
        dimmerSlider.value = steppedValue

        if dimmerLastValue != steppedValue {
            print("\(steppedValue)")
            dimmerLastValue = steppedValue

            let offset:UInt8 = (UInt8)(steppedValue) / 25
            sendJoyState(HomeCommands.dimmer0.rawValue + offset)
        }
    }

    // UIPickerView Data Source Protocol
    func numberOfComponents(in pickerView: UIPickerView) -> Int {
        return 1
    }
    func pickerView(_ pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        return pickerData.count
    }
    
    // UIPickerView Delegate
    func pickerView(_ pickerView: UIPickerView, titleForRow row: Int, forComponent component: Int) -> String? {
        return pickerData[row]
    }

    func pickerView(_ pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
        print("\(pickerData[row])")
        sendJoyState(HomeCommands.song0.rawValue + (UInt8)(row))

    }

}
