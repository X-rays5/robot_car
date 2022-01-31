<!-- script -->
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
                conn.startNotifications();
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
        if (connValid() && conn.isConnected()) {
            await conn.writeValue(msg);
        }
    }

    async function sendEvent(event: string) {
        if (connValid() && conn.isConnected()) {
            conn.writeValue(event);
        }
    }
</script>

<!-- HTML -->
<div>
    <h4>Currently connected: {conn_connected}</h4>
    <button type="button" class="btn btn-primary" on:click={connectBluetooth}>Connect to device</button>
    <button type="button" class="btn btn-primary" on:click={disconnectBluetooth}>Disconnect device</button>
    <br/>
    {#if conn_connected}
        <p>Last notification received: {conn.lastMessage}</p>
        <input bind:value={msg}/>
        <button type="button" class="btn btn-primary" on:click={sendMessage}>Send dbg msg</button>
        <div class="controls">
            <p></p>
            <button type="button" class="out-btn ctrl-btn btn btn-primary" on:click={() => sendEvent('forward')}><i class="fa-solid fa-arrow-up"></i></button>
            <p></p>
            <button type="button" class="ctrl-btn btn btn-primary" on:click={() => sendEvent('left')}><i class="fa-solid fa-arrow-left"></i></button>
            <button type="button" class="ctrl-btn btn btn-primary" on:click={() => sendEvent('stop')}><i class="fa-solid fa-ban"></i></button>
            <button type="button" class="ctrl-btn btn btn-primary" on:click={() => sendEvent('right')}><i class="fa-solid fa-arrow-right"></i></button>
            <p></p>
            <button type="button" class="out-btn ctrl-btn btn btn-primary" on:click={() => sendEvent('back')}><i class="fa-solid fa-arrow-down"></i></button>
            <p></p>
        </div>
    {/if}
</div>

<!-- CSS -->
<style lang="css">
    .controls {
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        grid-template-rows: [row] auto [row] auto [row] auto [row];
        max-width: 50%;
        min-width: 0;
        min-height: 0;
    }

    .ctrl-btn {
        overflow: hidden;
        min-width: 0;
        margin: 3px;
    }

    .out-btn {
        width: 140%;
        margin-left: -20%;
    }
</style>