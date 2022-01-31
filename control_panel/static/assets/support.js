function createNoBrowserSupport() {
    const container = document.createElement('div');
    container.hidden = true;
    container.id = 'browsernosupport';
    container.classList.add('overlay');
    document.body.append(container);
    const title = document.createElement('h4');
    title.innerHTML = "This browser doesn't support bluetooth";
    container.appendChild(title);
    const fix1 = document.createElement('p');
    fix1.innerHTML = "Make sure you're using a chromium based browser.";
    container.appendChild(fix1);
    const fix2 = document.createElement('p');
    fix2.innerHTML = 'If you are using a chromium based browser make sure the enable-experimental-web-platform-features flag is enabled.';
    container.appendChild(fix2);
}

function createDeviceNoSupport() {
    const container = document.createElement('div');
    container.hidden = true;
    container.id = 'devicenosupport';
    container.classList.add('overlay');
    document.body.append(container);
    const title = document.createElement('h4');
    title.innerHTML = "This device doesn't support bluetooth";
    container.appendChild(title);
}

function checkSupport() {
    createNoBrowserSupport();
    createDeviceNoSupport();

    if (typeof navigator.bluetooth === 'undefined' || navigator.bluetooth === null || typeof navigator.bluetooth.getDevices !== "function") {
        console.log('Browser doesn\'t support bluetooth');
        document.querySelector('#browsernosupport').hidden = false;
    } else {
        navigator.bluetooth.getAvailability().then(available => {
            if (!available) {
                document.querySelector('#devicenosupport').hidden = false;
            }
        });
        navigator.bluetooth.addEventListener('availabilitychanged', event => {
            document.querySelector('#devicenosupport').hidden  = event.value;
        });
    }
}