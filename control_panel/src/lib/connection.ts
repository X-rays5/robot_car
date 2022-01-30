export class Connection {
    /**
     * @param reconnect Reconnect automatically on connection loss
     */
    constructor(reconnect: boolean) {
        this.#reconnect = reconnect;
    }

    /**
     * @param reconnect Reconnect automatically on connection loss
     */
    set reconnect(reconnect: boolean) {
        this.#reconnect = reconnect;
    }

    async getDevice(options?: RequestDeviceOptions): Promise<BluetoothDevice> {
        await navigator.bluetooth.requestDevice(options).then(device => {
            console.log('Found device ', device);
            this.device = device;
        })
        return this.device;
    }

    async connectDevice(): Promise<boolean> {
        if (typeof this.device !== 'undefined') {
            return await this.device.gatt.connect().then(r => {
                r.connected ? console.log(`Connected to ${r.device.name}`) : console.log(`Failed to connect to ${r.device.name}`);
                return r.connected;
            })
        } else {
            return false;
        }
    }

    set setDisconnectCB(CB: () => never) {
        this.#disconnectCB = CB;
    }

    disconnectDevice() {
        this.device.gatt.disconnect();
    }

    _onDisconnect() {
        console.log(`Lost connection to ${this.device.name}`);
        if (this.#disconnectCB) {
            this.#disconnectCB();
        }
        if (this.#reconnect) {
            console.log(`Attempting reconnect to ${this.device.name}`);
            this.connectDevice().then(connected => {
                if (connected) {
                    console.log(`Reconnected to ${this.device.name}`);
                } else {
                    console.log(`Failed to reconnect to ${this.device.name}`);
                }
            })
        }
    }

    getPrimaryService(service: BluetoothServiceUUID) {
        return this.device.gatt.getPrimaryService(service);
    }

    getPrimaryServices(service?: BluetoothServiceUUID) {
        return this.device.gatt.getPrimaryServices(service);
    }

    isConnected(): boolean {
       return typeof this.device !== 'undefined' ? this.device.gatt.connected : false;
    }

    device: BluetoothDevice;
    #disconnectCB: () => never;
    #reconnect: boolean;
}