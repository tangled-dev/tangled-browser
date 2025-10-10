let millixFrame;

function onFrameReady() {
    console.log("inframe ready");
    chrome.send('initialize', []);
    setTimeout(refreshNotificationVolume, 1000);
}

function refreshNotificationVolume() {
    if (!millixFrame) {
        console.log("frame not ready yet");
        return setTimeout(refreshNotificationVolume, 10000);
    }

    millixFrame.contentWindow.postMessage({
        type: 'refresh_notification_volume'
    }, 'chrome-untrusted://millix/');

    setTimeout(refreshNotificationVolume, 10000);
}

function onLoadNodeApiConfig(apiConfig) {
    if (!millixFrame) {
        console.log("frame not ready yet");
        return;
    }
    console.log("[onLoadNodeApiConfig]", apiConfig);
    millixFrame.contentWindow.postMessage({
        type: 'update_api',
        ...apiConfig
    }, 'chrome-untrusted://millix/');
}

window.addEventListener('message', ({ data }) => {
    console.log("[millix_app]", data);
    switch (data.type) {
        case 'wallet_update_state':
            chrome.send('updateMillixWallet', [data]);
            break;
        case 'wallet_notification_volume':
            chrome.send('updateMillixWallet', [data]);
            break;
    }
});


function refreshIframe() {
    millixFrame = document.getElementById('millix_frame');

    let page = window.location.pathname.substring(1) + window.location.search + window.location.hash;

    if (millixFrame) {
        millixFrame.src = `chrome-untrusted://millix/${page}`
    } else {
        millixFrame = document.createElement('iframe');
        millixFrame.id = 'millix_frame';
        millixFrame.allow = "clipboard-write"
        millixFrame.onload = onFrameReady;
        millixFrame.src = `chrome-untrusted://millix/${page}`
        document.body.appendChild(millixFrame);
    }
}

window.onload = refreshIframe


function locationHashChanged() {
    if (location.hash !== "") {
        refreshIframe();
    }
}
window.onhashchange = locationHashChanged;