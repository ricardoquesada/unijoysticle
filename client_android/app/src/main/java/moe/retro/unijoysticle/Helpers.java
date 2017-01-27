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

import android.content.Context;
import android.net.nsd.NsdManager;
import android.net.nsd.NsdServiceInfo;
import android.util.Log;

import java.net.InetAddress;


class Helpers {
    private static final String TAG = "Helpers";
    private static int sFinishedResolved = 0;
    private static InetAddress sServerInetAddress = null;
    private static boolean sInProgress = false;
    private static NsdManager.ResolveListener sResolveListener = new NsdManager.ResolveListener() {
        @Override
        public void onResolveFailed(NsdServiceInfo serviceInfo, int errorCode) {
            Log.e(TAG, "Resolve failed: " + errorCode);
            sFinishedResolved = -1;
            sInProgress = false;
        }

        @Override
        public void onServiceResolved(NsdServiceInfo serviceInfo) {
            Log.e(TAG, "Resolve Succeeded. " + serviceInfo);
            sServerInetAddress = serviceInfo.getHost();
            sFinishedResolved = 1;
            sInProgress = false;
        }
    };
    private static NsdServiceInfo sServiceInfo = new NsdServiceInfo();

    static InetAddress resolveUniJoystiCleLocal(Context context) {

        if (!sInProgress) {
            sServerInetAddress = null;
            sInProgress = true;

            NsdManager nsdManager = (NsdManager) context.getSystemService(Context.NSD_SERVICE);
            sServiceInfo.setServiceName("unijoysticle");
            sServiceInfo.setServiceType("_unijoysticle._udp");
            nsdManager.resolveService(sServiceInfo, sResolveListener);
        }

        sFinishedResolved = 0;
        int tries = 0;
        while (sFinishedResolved == 0) {
            try {
                Thread.sleep(100);
            } catch (InterruptedException e) {
                e.printStackTrace();
                sFinishedResolved = -1;
            }
            tries++;
            // 2s to resolve it... if it can't fail
            if (tries == 20) {
                sFinishedResolved = -1;
                // should cancel resolve... not sure how to do it.
            }
        }
        return sServerInetAddress;
    }
}
