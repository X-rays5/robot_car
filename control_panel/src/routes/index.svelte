<script lang="ts">
    import {Connection} from "$lib/connection";

    const service_uuid = 0xffe0;
    const notification_uuid = 0xffe1;
    const write_uuid = 0xffe2;

    let conn: Connection
    let conn_connected = false;
    let msg = '{"event": "forward"}';

    function connValid(): boolean {
        return typeof conn !== 'undefined';
    }


    async function connectBluetooth() {
        conn = new Connection(false);
        await conn.getDevice({
            filters: [
                {manufacturerData: [{companyIdentifier: 0x5344}]},
            ],
            optionalServices: [
                service_uuid
            ]
        });
        await conn.connectDevice().then(connected => {
            conn_connected = connected;
            if (connected) {
                console.log(`Connected to ${conn.device.name}`);
            }
        })
        await conn.getPrimaryService(service_uuid).then(service => {
            service.getCharacteristic(notification_uuid).then(characteristic => {
                conn.setNotificationCharacteristic = characteristic;
            }).catch(e => {
                console.log(e);
            });
            service.getCharacteristic(write_uuid).then(characteristic => {
                conn.setWriteCharacteristic = characteristic;
            }).catch(e => {
                console.log(e);
            });
        });
    }

    async function disconnectBluetooth() {
        if (connValid()) {
            conn.disconnectDevice();
            conn_connected = false;
        }
    }

    async function sendMessage() {
        if (connValid()) {
            await conn.writeValue(msg);
        }
    }
</script>
<div>
    <h4>Currently connected: {conn_connected}</h4>
    <button type="button" class="btn btn-primary" on:click={connectBluetooth}>Connect to device</button>
    <button type="button" class="btn btn-primary" on:click={disconnectBluetooth}>Disconnect device</button>
    <br/>
    <input bind:value={msg}/>
    <button type="button" class="btn btn-primary" on:click={sendMessage}>Send dbg msg</button>
</div>