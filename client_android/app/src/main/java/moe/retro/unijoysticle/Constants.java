/*
 * Copyright (C) 2016 Ricardo Quesada
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package moe.retro.unijoysticle;

class Constants {
    static final String key_serverAddress = "key_serverAddress";
    static final String key_serverStats = "key_serverStats";

    static final String key_movementThreshold="key_movementThreshold";
    static final String key_jumpThreshold="key_jumpThreshold";
    static final String key_rotationRatio="key_rotationRatio";

    static final String key_enableButtonB = "key_enableButtonB";
    static final String key_swapButtonsAB = "key_swapButtonsAB";

    static final float default_movementThreshold = 0.4f;
    static final float default_jumpThreshold = 2.1f;
    static final float default_rotationRatio = 1.0f;

    static final float gravity = 9.81f;
}
