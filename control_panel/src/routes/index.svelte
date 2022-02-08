<!-- script -->
<script lang="ts">
    import {Connection} from "$lib/connection";
    import {onMount} from "svelte";

    const service_uuid = 0xffe0;
    const notification_uuid = 0xffe1;
    const write_uuid = 0xffe2;

    let conn: Connection
    let conn_connected = false;
    let connection_check_interval;
    let last_msg_check_interval;
    let is_landscape = true;
    let ignore_landscape_warning = false;

    let state = {
        ultrasonic: {
            position: 0,
            distance: 0,
        },
        linetracking: {
            left: false,
            middle: false,
            right: false,
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
                    let msg = conn.lastMessage();
                    while (typeof msg !== 'undefined') {
                        let msg_obj = JSON.parse(msg);
                        if (typeof msg_obj.error !== 'undefined') {
                            console.error(msg_obj.error);
                            msg = conn.lastMessage();
                            continue;
                        }
                        if (msg_obj.type === 'status') {
                            state.ultrasonic.position = msg_obj.ultrasonic.pos ?? 0;
                            state.ultrasonic.distance = msg_obj.ultrasonic.distance ?? 0;
                            state.linetracking.left = msg_obj.linetracking[0] ?? false;
                            state.linetracking.middle = msg_obj.linetracking[1] ?? false;
                            state.linetracking.right = msg_obj.linetracking[2] ?? false;
                            console.log(state);
                        }
                        msg = conn.lastMessage();
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

    async function keyDown(e) {
        switch (e.keyCode) {
            case 65:
            case 37:
                await conn.writeValue('left');
                break;
            case 87:
            case 38:
                await conn.writeValue('forward');
                break;
            case 68:
            case 39:
                await conn.writeValue('right');
                break;
            case 83:
            case 40:
                await conn.writeValue('back');
                break;
            case 81:
                await conn.writeValue('ultra-left');
                break;
            case 69:
                await conn.writeValue('ultra-right');
                break;
        }
    }

    async function keyUp(e) {
        switch (e.keyCode) {
            case 65:
            case 37:
            case 87:
            case 38:
            case 68:
            case 39:
            case 83:
            case 40:
                await conn.writeValue('stop');
                break;
            case 81:
            case 69:
                await conn.writeValue('ultra-stop');
                break;
        }
    }

    onMount(() => {
        window.addEventListener('gamepadconnected', (e) => {
            const gamepad = e.gamepad;
            if (gamepad.mapping === 'standard') {
                console.info(`%cFound supported gamepad: ${gamepad.id}`, 'color: white');
            } else {
                console.warn(`Unsupported gamepad ${gamepad.id}`);
            }
        });
        setInterval(() => {
            if (connValid() && conn.isConnected()) {
                if (navigator.getGamepads().length > 0) {
                    for (let i = 0; i < navigator.getGamepads().length; i++) {
                        const gamepad = navigator.getGamepads()[i];
                        if (gamepad) {
                            if (gamepad.mapping === 'standard') {
                                if (gamepad.axes[0] > 0.2) {
                                    conn.writeValue('right');
                                } else if (gamepad.axes[0] < -0.2) {
                                    conn.writeValue('left');
                                }
                                if (gamepad.buttons[7].pressed) {
                                    conn.writeValue('forward');
                                } else if (gamepad.buttons[6].pressed) {
                                    conn.writeValue('back');
                                }
                                if (gamepad.axes[1] > 0.2) {
                                    conn.writeValue('stop');
                                }
                                if (gamepad.buttons[4].pressed) {
                                    conn.writeValue('ultra-left');
                                } else if (gamepad.buttons[5].pressed) {
                                    conn.writeValue('ultra-right');
                                }
                                if (gamepad.buttons[1].pressed) {
                                    conn.writeValue('ultra-stop');
                                }
                                if (gamepad.buttons[3].pressed) {
                                    conn.writeValue('ultra-reset');
                                }
                            }
                        }
                    }
                }
            }
        }, 50)

        is_landscape = screen.orientation.type.includes('landscape');
        setInterval(() => {
            is_landscape = screen.orientation.type.includes('landscape');
        }, 100);

        if (navigator.getGamepads().length > 0) {
            setTimeout(() => alert('A gamepad was detected. If you want to use it, please press a random button on the gamepad.'), 100);
        }
    })
</script>

<!-- special svelte tags -->
<svelte:window on:keydown={keyDown} on:keyup={keyUp}/>

<!-- HTML -->
<div>
    {#if is_landscape || ignore_landscape_warning}
        <div style="border-bottom: 3px solid black; padding-bottom: 10px; padding-left: 5px">
            <h4>Currently connected: {conn_connected}</h4>
            <button type="button" class="btn btn-primary" on:click={connectBluetooth}>Connect to device</button>
            <button type="button" class="btn btn-primary" on:click={disconnectBluetooth}>Disconnect device</button>
        </div>
        <br/>
        {#if true}
            <div id="controls">
                <div class="base-ctrl-item">
                    <h3 style="text-align: center">Ultrasonic</h3>
                    <h3 style="text-align: center">Position: {state.ultrasonic.position}, Distance: {state.ultrasonic.distance}</h3>
                    <div id="ultrasonic-controls" class="base-ctrl-group">
                        <button type="button" id="ultrasonic-left-btn" class="base-ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-left"></i></button>
                        <button type="button" class="base-ctrl-btn btn btn-primary" on:click={() => conn.writeValue('ultra-reset')}><i class="fa-solid fa-ban"></i></button>
                        <button type="button" id="ultrasonic-right-btn" class="base-ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-right"></i></button>
                    </div>
                </div>
                <div class="base-ctrl-item">
                    <h3 style="text-align: center">Drive Controls</h3>
                    <div id="drive-controls" class="base-ctrl-group">
                        <p></p>
                        <button type="button" id="forward-btn" class="base-ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-up"></i></button>
                        <p></p>
                        <button type="button" id="left-btn" class="base-ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-left"></i></button>
                        <button type="button" class="base-ctrl-btn btn btn-primary" on:click={() => conn.writeValue('stop')}><i class="fa-solid fa-ban"></i></button>
                        <button type="button" id="right-btn" class="base-ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-right"></i></button>
                        <p></p>
                        <button type="button" id="back-btn" class="base-ctrl-btn btn btn-primary"><i class="fa-solid fa-arrow-down"></i></button>
                        <p></p>
                    </div>
                </div>
                <div class="base-ctrl-item">
                    <h3 style="text-align: center">Linetracking</h3>
                    <div id="linetracking-controls" class="base-ctrl-group">
                        <button type="button" class="base-ctrl-btn btn btn-primary" on:click={() => conn.writeValue('line-track-start')}><i class="fa-solid fa-car-side"></i>&nbsp;<i class="fa-solid fa-grip-lines"></i></button>
                        <button type="button" class="base-ctrl-btn btn btn-primary" on:click={() => conn.writeValue('line-track-stop')}><i class="fa-solid fa-ban"></i>&nbsp;<i class="fa-solid fa-grip-lines"></i></button>
                    </div>
                </div>
            </div>
        {/if}
    {:else}
        <h1 style="text-align: center">It's advised to only use this site in landscape mode</h1>
        <button type="button" id="ign-land-btn" class="btn btn-danger" on:click={() => ignore_landscape_warning = true}>Ignore warning</button>
    {/if}
</div>

<!-- CSS -->
<style lang="less">
  @media only screen and (max-width: 320px) {
    .btn {
      margin-bottom: 2%;
    }
  }

  #ign-land-btn {
    position: absolute;
    left: 50%;
    transform: translate(-50%, -50%);
    top: 6rem;
  }

  #controls {
    margin: auto;
    left: 50%;
    width: 90%;
  }

  .base-ctrl-item {
    margin: 1%;
  }

  .base-ctrl-group {
    display: grid;
    grid-template-columns: repeat(3, 1fr);
    max-width: 40vw;
    margin: auto;
    left: 50%;
  }

  @media only screen and (min-width: 290px) {
    .base-ctrl-btn {
      font-size: 6vw;
    }
  }

  .base-ctrl-btn {
    margin: 2%;
    aspect-ratio: 1/1;
  }

  #linetracking-controls {
    grid-template-columns: repeat(2, 1fr) !important;
  }
</style>