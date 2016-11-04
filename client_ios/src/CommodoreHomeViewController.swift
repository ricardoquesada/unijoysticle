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
        case Nothing = 0
        case Song0 = 1
        case Song1 = 2
        case Song2 = 3
        case Song3 = 4
        case Song4 = 5
        case Song5 = 6
        case Song6 = 7
        case Song7 = 8
        case SongStop = 9
        case SongPlay = 10
        case SongPause = 11
        case SongResume = 12
        case SongNext = 13
        case SongPrev = 14
        case Dimmer0 = 15
        case Dimmer25 = 16
        case Dimmer50 = 17
        case Dimmer75 = 18
        case Dimmer100 = 19
        case AlarmOff = 20
        case AlarmOn = 21
        case Reserved0 = 22
        case Reserved1 = 23
        case Reserved2 = 24
        case Reserved3 = 25
        case Reserved4 = 26
        case Reserved5 = 27
        case Reserved6 = 28
        case Reserved7 = 29
        case Reserved8 = 30
        case Reserved9 = 31
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

        let prefValue = NSUserDefaults.standardUserDefaults().valueForKey(SETTINGS_IP_ADDRESS_KEY)
        if prefValue != nil {
            userServer = prefValue as! String
        }
        netConnection = NetworkConnection(serverName: userServer)
    }

    override func viewDidAppear(animated: Bool) {
        super.viewDidAppear(animated)

        if netConnection == nil {
            let alertController = UIAlertController(title: "Invalid server", message: "Server not found: " + userServer, preferredStyle: .Alert)
            let defaultAction = UIAlertAction(title: "OK", style: .Default, handler: { uiAlertController in
                self.navigationController?.popViewControllerAnimated(true)
            })
            alertController.addAction(defaultAction)
            presentViewController(alertController, animated: true, completion: nil)
            return
        } else {
        }
    }

    func sendJoyState(joy2Value:UInt8) {
        for _ in 1...2 {
            let version:UInt8 = 2     // should be 2
            let joysticks:UInt8 = 3    // 1 and 2
            let data:[UInt8] = [version, joysticks, 0, joy2Value]
            netConnection!.sendState2(data)
        }
    }

    @IBAction func alertValueChanged(sender: AnyObject) {
        print("\(alertSwitch.on)")
        if alertSwitch.on {
            sendJoyState(HomeCommands.AlarmOn.rawValue)
        } else {
            sendJoyState(HomeCommands.AlarmOff.rawValue)
        }
    }
    @IBAction func dimmerValueChanged(sender: AnyObject) {
        // step: 25
        let steppedValue = round(dimmerSlider.value / 25) * 25
        dimmerLabel.text = "\(steppedValue)"
        dimmerSlider.value = steppedValue

        if dimmerLastValue != steppedValue {
            print("\(steppedValue)")
            dimmerLastValue = steppedValue

            let offset:UInt8 = (UInt8)(steppedValue) / 25
            sendJoyState(HomeCommands.Dimmer0.rawValue + offset)
        }
    }

    // UIPickerView Data Source Protocol
    func numberOfComponentsInPickerView(pickerView: UIPickerView) -> Int {
        return 1
    }
    func pickerView(pickerView: UIPickerView, numberOfRowsInComponent component: Int) -> Int {
        return pickerData.count
    }
    
    // UIPickerView Delegate
    func pickerView(pickerView: UIPickerView, titleForRow row: Int, forComponent component: Int) -> String? {
        return pickerData[row]
    }

    func pickerView(pickerView: UIPickerView, didSelectRow row: Int, inComponent component: Int) {
        print("\(pickerData[row])")
        sendJoyState(HomeCommands.Song0.rawValue + (UInt8)(row))

    }

}
