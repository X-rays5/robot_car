import {isJsonString} from '$lib/utility';

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
            this.device.addEventListener('gattserverdisconnected', () => {
                console.log(`Lost bluetooth connection`);
                if (this.#disconnectCB) {
                    this.#disconnectCB();
                }
                if (this.#reconnect) {
                    console.log(`Attempting to reconnect`);
                    let tries = 0;
                    const interval = setInterval(async () => {
                        await this.connectDevice().then(connected => {
                            tries += 1;
                            if (connected) {
                                console.log(`Reconnected to ${this.device.name}`);
                                clearInterval(interval);
                            } else {
                                console.log(`Failed to reconnect`);
                                if (tries >= 30) {
                                    console.log(`Failed to reconnect within 30 seconds`);
                                    clearInterval(interval);
                                }
                            }
                        }).catch(()=>{;}); // don't care for exceptions
                    }, 1000);
                }
            });
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
        this.#notification_characteristic.startNotifications().then(() => {
            this.#notification_characteristic.addEventListener('characteristicvaluechanged', (event) => {
                const decoder = new TextDecoder();
                // eslint-disable-next-line @typescript-eslint/ban-ts-comment
                // @ts-ignore
                let msg = decoder.decode(event.target.value);
                msg = msg.trim();

                // check if message is split over multiple packets
                if (msg.length > 0) {
                    if (isJsonString(msg)) {
                        this.#msg_queue.push(msg);
                    } else {
                        // Timed out
                        if (this.#msg_queue_constructing_timeout !== 0 && Date.now() - this.#msg_queue_constructing_timeout > 50) {
                            this.#msg_queue_constructing = '';
                        }
                        // check if start of message
                        if (this.#msg_queue_constructing.length === 0) {
                            if (msg.startsWith('{') || msg.startsWith('[')) {
                                this.#msg_queue_constructing = msg;
                                this.#msg_queue_constructing_timeout = Date.now();
                            } else {
                                console.warn(`Message not starting with { or [: ${msg}`);
                            }
                        } else {
                            this.#msg_queue_constructing += this.#msg_queue_constructing.concat(msg);
                            // check if end of message
                            if (isJsonString(this.#msg_queue_constructing)) {
                                this.#msg_queue.push(this.#msg_queue_constructing);
                                this.#msg_queue_constructing = '';
                                this.#msg_queue_constructing_timeout = 0;
                            } else {
                                this.#msg_queue_constructing_timeout = Date.now();
                            }
                        }
                    }
                }
            });
        });
    }

    set setDisconnectCB(CB: () => never) {
        this.#disconnectCB = CB;
    }

    disconnectDevice() {
        this.device.gatt.disconnect();
    }

    #events: Set<string> = new Set(['forward', 'back', 'left', 'right', 'stop', 'ultra-left', 'ultra-right', 'ultra-stop', 'ultra-reset']);
    writeValue(event_type: string) {
        if (this.#events.has(event_type)) {
            event_type += ';';
            const encoder = new TextEncoder();
            const data = encoder.encode(event_type);
            if (data.byteLength > 20) {
                console.error('Cannot write more than 20 bytes');
                return;
            }
            this.#write_characteristic.writeValueWithoutResponse(data).catch(e => {
                console.log(e);
            });
        } else {
            throw new Error('Invalid event type');
        }
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
        this.#notification_characteristic = characteristic;
    }

    get getNotificationCharacteristic(): BluetoothRemoteGATTCharacteristic {
        return this.#notification_characteristic;
    }

    get lastMessage(): string {
        if (this.#msg_queue.length > 0) {
            return this.#msg_queue.shift();
        } else {
            return undefined;
        }
    }

    messageAvailable(): boolean {
        return this.#msg_queue.length > 0;
    }

    device: BluetoothDevice;
    #disconnectCB: () => never;
    #reconnect: boolean;
    #write_characteristic: BluetoothRemoteGATTCharacteristic;
    #notification_characteristic: BluetoothRemoteGATTCharacteristic;
    #msg_queue: string[] = [];
    #msg_queue_constructing = '';
    #msg_queue_constructing_timeout = 0;
}