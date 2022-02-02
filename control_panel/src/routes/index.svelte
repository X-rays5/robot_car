<!-- script -->
<script lang="ts">
    import {Connection} from "$lib/connection";

    const service_uuid = 0xffe0;
    const notification_uuid = 0xffe1;
    const write_uuid = 0xffe2;

    let conn: Connection
    let conn_connected = false;
    let connection_check_interval;
    let msg = '{"event": "forward"}';
    let last_msg_check_interval;

    let state = {
        ultrasonic: {
            position: 90,
        }
    }

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
        if (!last_msg_check_interval) {
            last_msg_check_interval = setInterval(() => {
                if (conn_connected) {
                    let msg = conn.lastMessage;
                    if (typeof msg !== 'undefined') {
                        while (typeof msg !== 'undefined') {
                            console.log(msg);
                            let msg_obj = JSON.parse(msg);
                            if (typeof msg_obj.error !== 'undefined') {
                                console.log(msg_obj.error);
                                msg = conn.lastMessage;
                                continue;
                            }
                            if (msg_obj.type === 'status') {
                                if (msg_obj.category === 'ultrasonic') {
                                    state.ultrasonic.position = msg_obj.data.pos;
                                } else {
                                    console.log(`Unknown message type: ${msg_obj.type}`);
                                }
                            }
                            msg = conn.lastMessage;
                        }
                    }
                }
            }, 100);
        }
        if (!connection_check_interval) {
            connection_check_interval = setInterval(() => {
                conn_connected = connValid() && conn.isConnected();
            }, 500);
        }
        const forward_btn = document.getElementById('forward-btn');
        const back_btn = document.getElementById('back-btn');
        const left_btn = document.getElementById('left-btn');
        const right_btn = document.getElementById('right-btn');
        const ultrasonic_left_btn = document.getElementById('ultrasonic-left-btn');
        const ultrasonic_right_btn = document.getElementById('ultrasonic-right-btn');

        forward_btn.onpointerdown = () => conn.writeValue('forward');
        forward_btn.onpointerup = () => conn.writeValue('stop');
        back_btn.onpointerdown = () => conn.writeValue('back');
        back_btn.onpointerup = () => conn.writeValue('stop');
        left_btn.onpointerdown = () => conn.writeValue('left');
        left_btn.onpointerup = () => conn.writeValue('stop');
        right_btn.onpointerdown = () => conn.writeValue('right');
        right_btn.onpointerup = () => conn.writeValue('stop');
        ultrasonic_left_btn.onpointerdown = () => conn.writeValue('ultra-left');
        ultrasonic_left_btn.onpointerup = () => conn.writeValue('ultra-stop');
        ultrasonic_right_btn.onpointerdown = () => conn.writeValue('ultra-right');
        ultrasonic_right_btn.onpointerup = () => conn.writeValue('ultra-stop');
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
</script>

<!-- HTML -->
<div>
    <h4>Currently connected: {conn_connected}</h4>
    <button type="button" class="btn btn-primary" on:click={connectBluetooth}>Connect to device</button>
    <button type="button" class="btn btn-primary" on:click={disconnectBluetooth}>Disconnect device</button>
    <br/>
    {#if conn_connected}
        <input bind:value={msg}/>
        <button type="button" class="btn btn-primary" on:click={sendMessage}>Send dbg msg</button>

        <h3>Drive Controls</h3>
        <div id="drive-controls">
            <p></p>
            <button type="button" id="forward-btn" class="out-btn ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-up"></i></button>
            <p></p>
            <button type="button" id="left-btn" class="ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-left"></i></button>
            <button type="button" class="ctrl-btn btn btn-primary" on:click={() => conn.writeValue('stop')}><i class="fa-solid fa-ban"></i></button>
            <button type="button" id="right-btn" class="ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-right"></i></button>
            <p></p>
            <button type="button" id="back-btn" class="out-btn ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-down"></i></button>
            <p></p>
        </div>
        <h3>Ultrasonic</h3>
        <div id="ultrasonic-controls">
            <button type="button" id="ultrasonic-left-btn" class="ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-left"></i></button>
            <button type="button" class="ctrl-btn btn btn-primary" on:click={() => conn.writeValue('ultra-reset')}><i class="fa-solid fa-ban"></i></button>
            <button type="button" id="ultrasonic-right-btn" class="ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-right"></i></button>
        </div>
    {/if}
</div>

<!-- CSS -->
<style lang="css">
    #drive-controls {
        justify-content: center;
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        grid-template-rows: [row] auto [row] auto [row] auto [row];
        max-width: 90%;
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

    #ultrasonic-controls {
        display: grid;
        grid-template-columns: repeat(3, 1fr);
        grid-template-rows: [row] auto;
        max-width: 90%;
        min-width: 0;
        min-height: 0;
    }
</style>