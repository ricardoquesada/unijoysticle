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

// Code taken from here:
// https://developer.apple.com/library/ios/samplecode/AccelerometerGraph/Introduction/Intro.html

import Foundation
import CoreMotion

// Returns the input value clamped to the lower and upper limits.
// Taken from here: https://gist.github.com/leemorgan/bf1a0a1a8b2c94bce310
func clamp<T: Comparable>(value: T, lower: T, upper: T) -> T {
    return min(max(value, lower), upper)
}
func norm(x:Double, y:Double, z:Double) -> Double
{
    return sqrt(x * x + y * y + z * z);
}

class  AccelerometerFilter {

    var x:Double = 0
    var y:Double = 0
    var z:Double = 0
    var adaptive:Bool = false

    let accelerometerMinStep = 0.02
    let accelerometerNoiseAttenuation = 3.0

    func addAcceleration(acceleration: CMAcceleration) {
        x = acceleration.x;
        y = acceleration.y;
        z = acceleration.z;
    }
}

// See http://en.wikipedia.org/wiki/Low-pass_filter for details low pass filtering
class LowpassFilter: AccelerometerFilter {

    var dt: Double = 0
    var RC: Double = 0
    var filterConstant: Double = 0

    init(sampleRate: Double, cutoffFrequency: Double) {
        dt = 1.0 / sampleRate
        RC = 1.0 / cutoffFrequency;
        filterConstant = dt / (dt + RC);
    }

    override func addAcceleration(acceleration: CMAcceleration) {

        var alpha = filterConstant

        if(adaptive)
        {
            let d = clamp(fabs(norm(x, y: y, z: z) - norm(acceleration.x, y: acceleration.y, z: acceleration.z)) / accelerometerMinStep - 1.0, lower: 0.0, upper: 1.0);
            alpha = (1.0 - d) * filterConstant / accelerometerNoiseAttenuation + d * filterConstant;
        }

        x = acceleration.x * alpha + x * (1.0 - alpha);
        y = acceleration.y * alpha + y * (1.0 - alpha);
        z = acceleration.z * alpha + z * (1.0 - alpha);
    }
}

// See http://en.wikipedia.org/wiki/High-pass_filter for details on high pass filtering
class HighpassFilter: AccelerometerFilter {

    var dt: Double = 0
    var RC: Double = 0
    var filterConstant: Double = 0
    var lastX: Double = 0
    var lastY: Double = 0
    var lastZ: Double = 0

    init(sampleRate: Double, cutoffFrequency: Double) {
        dt = 1.0 / sampleRate;
        RC = 1.0 / cutoffFrequency;
        filterConstant = RC / (dt + RC);
    }

    override func addAcceleration(acceleration: CMAcceleration) {
        var alpha = filterConstant;

        if (adaptive)
        {
            let d = clamp(fabs(norm(x, y: y, z: z) - norm(acceleration.x, y: acceleration.y, z: acceleration.z)) / accelerometerMinStep - 1.0, lower: 0.0, upper: 1.0);
            alpha = d * filterConstant / accelerometerNoiseAttenuation + (1.0 - d) * filterConstant;
        }

        x = alpha * (x + acceleration.x - lastX);
        y = alpha * (y + acceleration.y - lastY);
        z = alpha * (z + acceleration.z - lastZ);
    
        lastX = acceleration.x;
        lastY = acceleration.y;
        lastZ = acceleration.z;
    }
}

class NoFilter: AccelerometerFilter {
    var lastX: Double = 0
    var lastY: Double = 0
    var lastZ: Double = 0
    var filter: Double = 0.05

    init(filterFactor: Double) {
        filter = filterFactor
    }

    override func addAcceleration(acceleration: CMAcceleration) {

        if (adaptive) {
            x = acceleration.x * filter + (1 - filter) * lastX
            y = acceleration.y * filter + (1 - filter) * lastY
            z = acceleration.z * filter + (1 - filter) * lastZ
        } else {
            x = acceleration.x
            y = acceleration.y
            z = acceleration.z
        }

        lastX = x
        lastY = y
        lastZ = z
    }
}

