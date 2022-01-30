<script lang="ts">
    import {Connection} from "$lib/connection";

    let conn: Connection

    function connValid(): boolean {
        return typeof conn !== 'undefined';
    }

    async function connectBluetooth() {
        conn = new Connection(false);
        await conn.getDevice({
            filters: [
                {namePrefix: 'ELEGOO'}
            ]
        });
        await conn.connectDevice().then(connected => {
            if (connected) {
                console.log(`Connected to ${conn.device.name}`);
            }
        })
    }

    async function disconnectBluetooth() {
        if (connValid()) {
            conn.disconnectDevice();
        }
    }

    function deviceConnected(): boolean {
        return connValid() ? conn.isConnected() : false;
    }
</script>

<div>
    <h5>Currently connected: {deviceConnected()}</h5>
    <button type="button" class="btn btn-primary" on:click={connectBluetooth}>Connect to device</button>
    <button type="button" class="btn btn-primary" on:click={disconnectBluetooth}>Disconnect device</button>
</div>