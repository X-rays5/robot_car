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
            this.device = device;
        }).catch(e => {
            console.log(e);
            alert('The was a unknown error try disconnecting all other bluetooth devices');
        })
        return this.device;
    }

    async connectDevice(): Promise<boolean> {
        if (typeof this.device !== 'undefined') {
            return await this.device.gatt.connect().then(r => {
                return r.connected;
            })
        } else {
            return false;
        }
    }

    startNotifications() {
        this.#notification_characteristric.startNotifications().then(() => {
            this.#notification_characteristric.addEventListener('characteristicvaluechanged', (event) => {
                // eslint-disable-next-line @typescript-eslint/ban-ts-comment
                // @ts-ignore
                const value = event.target.value;
                const decoder = new TextDecoder();
                const data = decoder.decode(value);
                console.log(data);
                this.#msg_queue.push(data);
            });
        });
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

    static getUUID(unique: string): string {
        return `0000${unique}-0000-1000-8000-008005f9b34fb`;
    }

    writeValue(msg: string) {
        const msg_obj = JSON.parse(msg); // verify json
        const encoder = new TextEncoder();
        const data = encoder.encode(JSON.stringify(msg_obj));
        console.log(data.buffer);
        this.#write_characteristic.writeValueWithoutResponse(data.buffer).catch(e => {
            console.log(e);
        });
    }

    getPrimaryService(service: BluetoothServiceUUID) {
        return this.device.gatt.getPrimaryService(service);
    }

    isConnected(): boolean {
        return typeof this.device !== 'undefined' ? this.device.gatt.connected : false;
    }

    set setWriteCharacteristic(characteristic: BluetoothRemoteGATTCharacteristic) {
        this.#write_characteristic = characteristic;
    }

    get getWriteCharacteristic(): BluetoothRemoteGATTCharacteristic {
        return this.#write_characteristic;
    }

    set setNotificationCharacteristic(characteristic: BluetoothRemoteGATTCharacteristic) {
        this.#notification_characteristric = characteristic;
    }

    get getNotificationCharacteristic(): BluetoothRemoteGATTCharacteristic {
        return this.#notification_characteristric;
    }

    get lastMessage(): string {
        if (this.#msg_queue.length > 0) {
            const tmp = this.#msg_queue[0];
            this.#msg_queue.shift();
            return tmp
        } else {
            return '';
        }
    }

    messageAvailable(): boolean {
        return this.#msg_queue.length > 0;
    }

    device: BluetoothDevice;
    #disconnectCB: () => never;
    #reconnect: boolean;
    #write_characteristic: BluetoothRemoteGATTCharacteristic;
    #notification_characteristric: BluetoothRemoteGATTCharacteristic;
    #msg_queue: string[] = [];
}