function setupAudio(bufferSize, audioCallback) {
    if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
        console.log("getUserMedia supported.");
        navigator.mediaDevices
            .getUserMedia(
                // constraints - only audio needed for this app
                {
                    audio: true,
                },
            )

            // Success callback
            .then((stream) => {
                stream.getAudioTracks()[0].getSettings().noiseSuppression = false;
                stream.getAudioTracks()[0].getSettings().echoCancellation = false;

                console.log("got stream");
                const audioContext = new AudioContext();
                const source = audioContext.createMediaStreamSource(stream);
                const samplerate = audioContext.sampleRate;
                console.log(`capturing at SR: ${samplerate}`);
                const recorder = (audioContext.createScriptProcessor ||
                    audioContext.createJavaScriptNode).call(audioContext,
                        bufferSize,
                        1,
                        1);
                recorder.onaudioprocess = function (event) {
                    const samples = event.inputBuffer.getChannelData(0);

                    const samps = new Float32Array(samples);
                    audioCallback(samps);
                };
                source.connect(recorder);
                recorder.connect(audioContext.destination);

            })

            // Error callback
            .catch((err) => {
                console.error(`The following getUserMedia error occurred: ${err}`);
            });
    } else {
        console.log("getUserMedia not supported on your browser!");
    }

}

function rxAudio(samps) {
    const icanvas = document.getElementById("videoOut");
    const ctx = icanvas.getContext("2d");
    ctx.imageSmoothingEnabled = false;
    const data = ctx.createImageData(77, 58);
    for (let i = 0; i < data.data.length; i += 4) {
        data.data[i + 0] = samps[i / 4] * 255;
        data.data[i + 1] = samps[i / 4] * 255;
        data.data[i + 2] = samps[i / 4] * 255;
        data.data[i + 3] = 255;
    }
    ctx.putImageData(data, 0, 0);
}

document.addEventListener('DOMContentLoaded', (event) => {
    setupAudio(4096, rxAudio);
    const canvas = document.getElementById("videoOut");
    const ctx = canvas.getContext("2d");
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    ctx.imageSmoothingEnabled = false;
    const data = ctx.createImageData(10, 20);
    for (let i = 0; i < data.data.length; i += 4) {
        data.data[i] = 0;
        data.data[i] = 0;
        data.data[i] = 0;
        data.data[i + 3] = 255;
    }
    data.data[25] = 255;
    data.data[26] = 255;
    data.data[27] = 255;
    data.data[28] = 255;
    data.data[29] = 255;
    ctx.putImageData(data, 0, 0);
})
