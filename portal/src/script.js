// var gateway = `ws://${window.location.hostname}/ws`;
const ws_gateway = `ws://chaynik.local/ws`;
const http_gateway = `http://chaynik.local`;
let ws;

const varTempArray = [60, 70, 80, 90];

const store = PetiteVue.reactive({
  connected: false,
  state: false,
  loading: true,
  isActive: false,
  varTempIndex: 0,
  varTempValue: 60,
});

PetiteVue.createApp({
  store,
  toggleActive() {
    this.store.isActive = !this.store.isActive;
    if (!this.store.isActive) {
      this.store.varTempIndex = 0;
      this.store.varTempValue = 60;
    } else {
      this.store.varTempIndex = -1;
      this.store.varTempValue = 100;
    }

    ws.send('onoff');
  },
  toggleVarTemperature() {
    if (!this.store.isActive) {
      this.store.isActive = true;
      ws.send('var');
      return;
    }
    if (this.store.varTempIndex === -1) {
      this.store.varTempIndex = 0;
      this.store.varTempValue = 60;
      ws.send('var');
    } else {
      this.store.varTempIndex++;
    }
    if (this.store.varTempIndex >= varTempArray.length) {
      this.store.varTempIndex = 0;
    }
    this.store.varTempValue = varTempArray[this.store.varTempIndex];
    ws.send('var');
  },
}).mount();

PetiteVue.nextTick().then(() => {
  console.log('mounted');
  initWebSocket();
});

function initWebSocket() {
  ws = new sockette(ws_gateway, {
    timeout: 5e3,
    maxAttempts: 10,
    onopen: (e) => {
      while (!store.loading) {
        break;
      }
      console.log('Connected!', e);
      store.connected = true;
    },
    onmessage: (e) => {
      console.log('Received:', e);
      store.isActive = e.data === '0' ? false : true;
    },
    onreconnect: (e) => console.log('Reconnecting...', e),
    onmaximum: (e) => console.log('Stop Attempting!', e),
    onclose: (e) => {
      console.log('Closed!', e);
      store.connected = false;
    },
    onerror: (e) => console.log('Error:', e),
  });
}
