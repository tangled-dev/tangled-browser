let CHILD_FRAME_ID = 'chrome-untrusted://millix-ws/'

// UNCOMMENT ON DEV MODE ONLY
/*
CHILD_FRAME_ID = "*";
const cr = {
    define(n, f) {
        globalThis[n] = f();
    },
    addWebUIListener() { }
}

const loadTimeData = {
    getBoolean() { return false }
}

chrome.send = () => { }

const NODE_ID = undefined;
const NODE_SIGNATURE = undefined;
if(!NODE_ID || !NODE_SIGNATURE){
    throw Error("NODE_ID and NODE_SIGNATURE must be defined");
}
setTimeout(() => {
    millix_bar.connectToWallet({ node_id: NODE_ID, node_signature: NODE_SIGNATURE })
    millix_bar.activateWallet()
    millix_bar.refreshThemeStyles({ is_dark_theme: true })
}, 1000)
*/
// END DEV MODE BLOCK

cr.define('millix_bar', function () {
    'use strict';

    let millixAPIFrame;
    let lastKnownTransaction = undefined;
    let reloadTimeout = undefined;
    let fetchAdvertisementTimeout = undefined;
    let transactionTimestampUpdateTimeout = undefined;
    let totalAdvertisementPaymentTimeout = undefined;
    const ADVERTISEMENT_ROTATION_TIME = 30000;
    const ADVERTISEMENT_ROTATION_TIME_EMPTY = 5000;
    let walletLocked = true;
    let unlockFromBar = false;
    const audioDeposit = new Audio('/deposit.mp3');
    let stopAdvertisementRendering = false;
    let advertisementRenderingTimestampStart = undefined;
    let advertisementRenderingTimestampPaused = undefined;
    let advertisementPaymentTimestampLast = undefined;
    let advertisementPaymentTotal = undefined;
    let sessionStart = undefined;
    let isDarkTheme = false;

    function refreshThemeStyles(data) {
        if (data.is_dark_theme) {
            document.body.classList.add('dark');
        } else {
            document.body.classList.remove('dark');
        }

        isDarkTheme = data.is_dark_theme;
    }

    function onApiFrameReady() {
        console.log("[onApiFrameReady]");
        millixAPIFrame = document.getElementById('frame_millix_api');
    }

    function connectToWallet(apiConfig) {
        console.log("[activateWallet]");
        if (!millixAPIFrame) {
            setTimeout(() => connectToWallet(apiConfig), 500);
            return;
        }
        console.log("[activateWallet] sent");

        millixAPIFrame.contentWindow.postMessage({
            type: 'api_config',
            ...apiConfig
        }, CHILD_FRAME_ID);

        millixAPIFrame.contentWindow.postMessage({
            type: 'get_session'
        }, CHILD_FRAME_ID);

        millixAPIFrame.contentWindow.postMessage({
            type: 'api_check'
        }, CHILD_FRAME_ID);
    }

    function showMillixWallet() {
        chrome.send('showMillixWallet');
    }

    function refreshMillixWallet() {
        chrome.send('showMillixWallet', ['refresh']);
    }

    function showNewAdvertisement(advertisement) {
        const $headline = $('#advertisement_headline');
        const $targetPhrase = $('#advertisement_deck');

        if (walletLocked) {
            $headline.text('');
            $targetPhrase.text('');
            $headline.prop("href", undefined);
            $targetPhrase.prop("href", undefined);

            $headline.prop("title", undefined);
            $targetPhrase.prop("title", undefined);

            $headline.off('click');
            $targetPhrase.off('click');

            advertisementRenderingTimestampStart = undefined;
            return;
        }

        !!advertisementPaymentTimestampLast && console.log("last ad payment", moment(new Date(advertisementPaymentTimestampLast)).fromNow())
        if (!advertisement || !advertisement.advertisement_url) {
            const aMinuteAgo = Date.now() - 60000;
            const twoMinutesAgo = Date.now() - 120000;
            const thirtyMinutesAgo = Date.now() - 1800000;
            const aDayAgo = Date.now() - 86400000;
            if (thirtyMinutesAgo > sessionStart && (!!advertisementPaymentTimestampLast && aDayAgo > advertisementPaymentTimestampLast)) {
                $headline.text('unable to find ads');
                $targetPhrase.text('please click "help -> report issue" to resolve');
            } else {
                $headline.text('searching for ads...');

                if (aMinuteAgo > sessionStart && (!!advertisementPaymentTimestampLast && twoMinutesAgo > advertisementPaymentTimestampLast)) {
                    $targetPhrase.text(`your last ad payment was received ${moment(new Date(advertisementPaymentTimestampLast)).fromNow()}` + (advertisementPaymentTotal ? `, you have earned ${advertisementPaymentTotal.toLocaleString('en-US')} millix in the past 24 hours.` : ''));
                } else {
                    $targetPhrase.text(advertisementPaymentTotal ? `you have earned ${advertisementPaymentTotal.toLocaleString('en-US')} millix in the past 24 hours.` : '');
                }
            }

            $headline.css('font-weight', 'normal');
            $headline.css('color', isDarkTheme ? 'rgba(115,115,115,0.8)' : 'lightgray');
            $targetPhrase.css('font-weight', 'normal');
            $targetPhrase.css('color', isDarkTheme ? 'rgba(115,115,115,0.8)' : 'lightgray');

            $headline.prop("href", undefined);
            $targetPhrase.prop("href", undefined);

            $headline.prop("title", undefined);
            $targetPhrase.prop("title", undefined);

            $headline.off('click');
            $targetPhrase.off('click');

            advertisementRenderingTimestampStart = undefined;

            if (!stopAdvertisementRendering) {
                clearTimeout(fetchAdvertisementTimeout);
                fetchAdvertisementTimeout = setTimeout(() => fetchAdvertisement(), ADVERTISEMENT_ROTATION_TIME_EMPTY);
            }

        } else {
            $headline.css('font-weight', '');
            $headline.css('color', '');
            $targetPhrase.css('font-weight', '');
            $targetPhrase.css('color', '');

            if ($(".arrow-icon").hasClass('open')) { //ads container not visible
                return;
            }

            let domain;
            try {
                domain = new URL(advertisement.advertisement_url).host;
            } catch (e) {
                console.warn("invalid advertisement url", e);
                return;
            }

            if (domain.startsWith("www.")) {
                domain = domain.substring(4);
            }

            let hasHeadline = false;
            let hasDeck = false;

            advertisement.attributes.forEach(attribute => {
                if (attribute.attribute_type === "advertisement_headline" && attribute.value != undefined) {
                    hasHeadline = true;
                    $headline.text(attribute.value);
                } else if (attribute.attribute_type === "advertisement_deck" && attribute.value != undefined) {
                    hasDeck = true;
                    $targetPhrase.text(`${attribute.value} - ${domain}`);
                }
            });

            if (!hasHeadline || !hasDeck) {
                return fetchAdvertisement(); // get a new ads
            }

            $headline.prop("href", advertisement.advertisement_url);
            $targetPhrase.prop("href", advertisement.advertisement_url);

            $headline.prop("title", domain);
            $targetPhrase.prop("title", domain);

            $headline.off('click');
            $targetPhrase.off('click');
            $headline.on('click', () => chrome.send('showMillixWallet', ['new_tab', advertisement.advertisement_url]));
            $targetPhrase.on('click', () => chrome.send('showMillixWallet', ['new_tab', advertisement.advertisement_url]));

            advertisementRenderingTimestampStart = Date.now();
        }
    }

    function disableAdvertisementFetch() {
        clearTimeout(fetchAdvertisementTimeout);
        showNewAdvertisement(null);
    }

    function fetchAdvertisement() {
        clearTimeout(fetchAdvertisementTimeout);

        if ($(".arrow-icon").hasClass('open')) { //ads container not visible
            fetchAdvertisementTimeout = setTimeout(() => fetchAdvertisement(), ADVERTISEMENT_ROTATION_TIME_EMPTY);
            return;
        }

        millixAPIFrame.contentWindow.postMessage({
            type: 'get_next_tangled_advertisement'
        }, CHILD_FRAME_ID);
        fetchAdvertisementTimeout = setTimeout(() => fetchAdvertisement(), ADVERTISEMENT_ROTATION_TIME);
    }

    function updateLastTransactionTimestamp(scheduleOnly) {
        clearTimeout(transactionTimestampUpdateTimeout);

        if (!scheduleOnly) {
            millixAPIFrame.contentWindow.postMessage({
                type: 'get_last_transaction_timestamp'
            }, CHILD_FRAME_ID);
        }

        transactionTimestampUpdateTimeout = setTimeout(() => updateLastTransactionTimestamp(), 10000);
    }

    function updateTotalAdvertisementPayment(scheduleOnly) {
        clearTimeout(totalAdvertisementPaymentTimeout);

        if (!scheduleOnly) {
            millixAPIFrame.contentWindow.postMessage({
                type: 'get_total_advertisement_payment'
            }, CHILD_FRAME_ID);
        }

        totalAdvertisementPaymentTimeout = setTimeout(() => updateTotalAdvertisementPayment(), 60000);
    }

    function activateWallet() {
        if (!walletLocked) {
            return;
        }

        sessionStart = Date.now();
        advertisementPaymentTimestampLast = Date.now();
        advertisementPaymentTotal = null;

        updateLastTransactionTimestamp();
        updateTotalAdvertisementPayment();


        $('#wallet_unlock').addClass('hidden');
        $('#wallet_restart').addClass('hidden');
        $('#wallet').removeClass('hidden');
        $('#btn_expand').removeClass('hidden');

        walletLocked = false;
        updateNodeStat(null);

        if (unlockFromBar) {
            refreshMillixWallet();
        }

        console.log("[activateWallet]");
        millixAPIFrame.contentWindow.postMessage({
            type: 'read_stat_start'
        }, CHILD_FRAME_ID);
        unlockFromBar = false;

        setTimeout(() => fetchAdvertisement(), 2000);
    }

    function deactivateWallet() {
        updateNodeStat(null);
        walletLocked = true;
        console.log("[deactivateWallet]");
        $('#wallet').addClass('hidden');
        $('#btn_expand').addClass('hidden');
        $('#wallet_restart').addClass('hidden');
        $('#wallet_unlock').removeClass('hidden');
        expandView(false);

        millixAPIFrame.contentWindow.postMessage({
            type: 'read_stat_stop'
        }, CHILD_FRAME_ID);

        disableAdvertisementFetch();
    }

    function doNodeRestart() {
        clearTimeout(reloadTimeout);
        $('#btn-restart-label').text('restarting');
        $('#btn-restart-icon').removeClass('hidden');
        $('#btn-restart-icon-power').addClass('hidden');
        $('#btn-restart-icon-spinner').removeClass('hidden');
        $('#wallet_restart > .btn').addClass('btn-disabled');
        chrome.send('restartMillixNode');
    }

    function restartWallet() {
        refreshMillixWallet();
        updateNodeStat(null);
        walletLocked = true;
        console.log("[restartWallet]");
        $('#wallet').addClass('hidden');
        $('#btn_expand').addClass('hidden');
        $('#wallet_unlock').addClass('hidden');
        let counter = 10;

        const updateRestartTimeout = () => {
            $('#btn-restart-label').text(`restart node (${counter}s)`);
            counter--;
            if (counter == 0) {
                doNodeRestart();
            } else {
                reloadTimeout = setTimeout(() => {
                    updateRestartTimeout();
                }, 1000);
            }
        }
        updateRestartTimeout();

        $('#btn-restart-icon').removeClass('hidden');
        $('#btn-restart-icon-power').removeClass('hidden');
        $('#btn-restart-icon-spinner').addClass('hidden');
        $('#wallet_restart > .btn').removeClass('btn-disabled');
        $('#wallet_restart').removeClass('hidden');
        expandView(false);
    }

    function updateNodeStat(nodeStat) {
        if (walletLocked && nodeStat) {
            return;
        }

        if (!nodeStat) {
            $('#balance_stable').text('');
            $('#balance_pending').text('');
            $('#peer_count').text('');
            $('#log_count').text('');
            $('#backlog_count').text('');
            $('#transaction_count').text('');
            lastKnownTransaction = undefined;
        } else {
            $('#balance_stable').text(nodeStat.balance.stable.toLocaleString());
            $('#balance_pending').text(nodeStat.balance.unstable.toLocaleString());

            $('#peer_count').text(nodeStat.network.peer_count.toLocaleString());
            $('#log_count').text(nodeStat.log.log_count.toLocaleString());
            $('#backlog_count').text(nodeStat.log.backlog_count.toLocaleString());
            $('#transaction_count').text(nodeStat.transaction.transaction_wallet_count.toLocaleString());
        }
    }

    function onMillixBarMessage(data) {
        console.log("[onMillixBarMessage] ", data);
        if (data.type === 'wallet_update_state') {
            if (data.action.type === 'UNLOCK_WALLET') {
                activateWallet();
            } else if (data.action.type === 'LOCK_WALLET') {
                deactivateWallet();
            } else if (data.action.type === 'UPDATE_NOTIFICATION_VOLUME') {
                changeVolume(data.action.payload);
            }
        } else if (data.type === 'api_config_update') {
            connectToWallet(data.config);
        }
    }

    /**
     * 
     */
    function initialize() {
        console.log("[initialize]");

        refreshThemeStyles({ is_dark_theme: loadTimeData.getBoolean('is_dark_theme') });

        $('#wallet').click(() => showMillixWallet());
        $('#wallet_unlock').click(() => showMillixWallet());
        $('#wallet_network').click(() => chrome.send('showMillixWallet', ['peers']));
        $('#wallet_log').click(() => chrome.send('showMillixWallet', ['event-log']));
        $('#wallet_backlog').click(() => chrome.send('showMillixWallet', ['backlog']));
        $('#wallet_transaction').click(() => chrome.send('showMillixWallet', ['transaction-list']));
        $('#wallet_restart').click(() => {
            doNodeRestart();
        });

        cr.addWebUIListener("onThemeChanged", refreshThemeStyles.bind(this));
    }

    function onLastTransactionUpdate(lastTransaction) {
        if (walletLocked) {
            return;
        }

        if (!lastKnownTransaction) {
            lastKnownTransaction = lastTransaction;
            return;
        } else if (lastKnownTransaction.transaction_id === lastTransaction.transaction_id) {
            return;
        }

        // check if we should notify user
        lastKnownTransaction = lastTransaction;
        millixAPIFrame.contentWindow.postMessage({
            type: 'get_transaction',
            transaction_id: lastKnownTransaction.transaction_id,
            shard_id: lastKnownTransaction.shard_id,
        }, CHILD_FRAME_ID);
    }

    function onLastTransactionTimestampUpdate(data) {
        if (!data.timestamp) {
            advertisementPaymentTimestampLast = undefined;
        } else {
            advertisementPaymentTimestampLast = data.timestamp * 1000;
        }
        updateLastTransactionTimestamp(true);
    }

    function onTotalAdvertisementPaymentUpdate(data) {
        advertisementPaymentTotal = data.total;
        updateTotalAdvertisementPayment(true);
    }

    function changeVolume(data) {
        let volume;
        if (data) {
            volume = data.volume / 100.;
        } else {
            volume = 1.0;
        }
        audioDeposit.volume = volume;
    }


    function onTransaction(transaction) {
        if (walletLocked) {
            return;
        }
        // check if we should notify user
        audioDeposit.currentTime = 0;
        audioDeposit.play();
    }

    function expandView(expanded) {
        const $arrow = $(".arrow-icon");
        const $adsHolder = $('#ads_holder');
        const $walletHolder = $('#wallet_holder');
        const $expandableViews = $(".expandable-view");

        if (expanded) {
            $arrow.addClass("open");
            $adsHolder.hide();
            $walletHolder.css("width", "");
            $walletHolder[0].classList.replace("col-4", "col-12");
            $("body").css("min-width", "");
            $expandableViews.removeClass("hidden");
        } else {
            $arrow.removeClass("open");
            $expandableViews.addClass("hidden");
            $walletHolder[0].classList.replace("col-12", "col-4");
            $walletHolder.css("width", "255px");
            $("body").css("min-width", "700px");
            $adsHolder.show();
        }
    }

    function onVisibilityChange() {
        if (document.hidden) {
            stopAdvertisementRendering = true;
            advertisementRenderingTimestampPaused = Date.now();
            clearInterval(fetchAdvertisementTimeout);
            console.log("pausing advertisement rendering")
        } else {
            stopAdvertisementRendering = false;
            console.log("resuming advertisement rendering")
            if (advertisementRenderingTimestampStart) {
                const remainingAdvertisementRendingTime = ADVERTISEMENT_ROTATION_TIME - (advertisementRenderingTimestampPaused - advertisementRenderingTimestampStart);
                if (remainingAdvertisementRendingTime > 0) {
                    console.log(Math.floor(remainingAdvertisementRendingTime / 1000) + " seconds remaining");
                    fetchAdvertisementTimeout = setTimeout(() => fetchAdvertisement(), remainingAdvertisementRendingTime);
                    return;
                }
            }
            console.log("prepare to get a new ad");
            fetchAdvertisement();
        }
    }

    // Return an object with all of the exports.
    return {
        initialize,
        connectToWallet,
        deactivateWallet,
        restartWallet,
        activateWallet,
        onMillixBarMessage,
        updateNodeStat,
        onApiFrameReady,
        onLastTransactionUpdate,
        onLastTransactionTimestampUpdate,
        onTotalAdvertisementPaymentUpdate,
        refreshThemeStyles,
        expandView,
        onTransaction,
        showNewAdvertisement,
        onVisibilityChange
    };
});

window.addEventListener('message', ({ data }) => {
    console.log("[millix_bar]", data);
    switch (data.type) {
        case 'last_transaction':
            millix_bar.onLastTransactionUpdate(data.data);
            break;
        case 'last_transaction_timestamp':
            millix_bar.onLastTransactionTimestampUpdate(data.data);
            break;
        case 'total_advertisment_payment':
            millix_bar.onTotalAdvertisementPaymentUpdate(data.data);
            break;
        case 'new_transaction':
            millix_bar.onTransaction(data.data);
            break;
        case 'millix_session':
            if (data.data.api_status === 'fail') {
                chrome.send('updateMillixWallet', [{ type: 'wallet_update_state', from_bar: true, action: { type: 'LOCK_WALLET' } }]);
                millix_bar.deactivateWallet();
            } else {
                chrome.send('updateMillixWallet', [{ type: 'wallet_update_state', from_bar: true, action: { type: 'UNLOCK_WALLET' } }]);
                millix_bar.activateWallet(data.data.wallet);
            }
            break;
        case 'node_stat':
            millix_bar.updateNodeStat(data.data);
            break;
        case 'node_error':
            millix_bar.restartWallet();
            break;
        case 'node_restarted':
            millix_bar.deactivateWallet();
            break;
        case 'initialize':
            chrome.send('initialize', []);
            break;
        case 'next_tangled_advertisement':
            const advertisement = data.data;
            console.log('[advertisement] new advertisement found', advertisement);
            millix_bar.showNewAdvertisement(advertisement);
            break;
    }

});

document.addEventListener('DOMContentLoaded', millix_bar.initialize);

document.addEventListener("visibilitychange", millix_bar.onVisibilityChange);

$(document).ready(() => {
    $(".arrow-icon").click(function () {
        const $this = $(this);
        if ($this.hasClass("open")) {
            millix_bar.expandView(false);
        } else {
            millix_bar.expandView(true);
        }
    });
});