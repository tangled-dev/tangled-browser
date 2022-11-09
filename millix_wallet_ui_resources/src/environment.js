export const SHOW_API_ERROR_IN_MODAL = false;

// main net
const NODE_ID        = ''; // to be used in src/js/api/index.js
const NODE_SIGNATURE = ''; // to be used in src/js/api/index.js

const NFT_VIEW_LINK_ORIGIN = 'tangled://millix';

const environment = {
    SHOW_API_ERROR_IN_MODAL,
    NODE_ID,
    NODE_SIGNATURE,
    NFT_VIEW_LINK_ORIGIN
};

// test net
// environment['NODE_ID']                = '';
// environment['NODE_SIGNATURE']         = '';
// environment['GENESIS_TRANSACTION_ID'] = 'BbYAZLcxbx6adN3KwHZSTGjE6VpeDhiJ3ZPrXs6EMAGqDPfi5';
// environment['GENESIS_SHARD_ID']       = 'AyAC3kjLtjM4vktAJ5Xq6mbXKjzEqXoSsmGhhgjnkXUvjtF2M';
// environment['MODE_TEST_NETWORK']      = true;

export default environment;
