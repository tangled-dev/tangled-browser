function onFrameReady() {
    console.log("inframe ready");
    chrome.send('initialize', []);
}

function onLoadNodeApiConfig(apiConfig) {
    console.log("[onLoadNodeApiConfig]", apiConfig);
    const millixFrame = document.getElementById('millix_frame');
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
    let iframe = document.getElementById('millix_frame');
    
    let page = window.location.pathname.substring(1) + window.location.search + window.location.hash;

    if (iframe) {
        iframe.src = `chrome-untrusted://millix/${page}`
    } else {
        iframe = document.createElement('iframe');
        iframe.id = 'millix_frame';
        iframe.allow = "clipboard-write"
        iframe.onload = onFrameReady;
        iframe.src = `chrome-untrusted://millix/${page}`
        document.body.appendChild(iframe);
    }
}

window.onload = refreshIframe


function locationHashChanged() {
    if (location.hash !== "") {
        refreshIframe();
    }
}
window.onhashchange = locationHashChanged;