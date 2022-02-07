function checkSupport() {
    if (typeof navigator.bluetooth === 'undefined' || navigator.bluetooth === null || typeof navigator.bluetooth.getDevices !== "function") {
        console.log('Browser doesn\'t support bluetooth');
        document.querySelector('#browsersupport').hidden = false;
    } else {
        navigator.bluetooth.getAvailability().then(available => {
            if (!available) {
                document.querySelector('#devicesupport').hidden = false;
            } else {
                document.querySelector('#svelte').hidden = false;
            }
        });
        navigator.bluetooth.addEventListener('availabilitychanged', event => {
            document.querySelector('#devicenosupport').hidden  = event.value;
        });
    }
}